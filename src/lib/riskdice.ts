import MainModuleFactory, { type MainModule } from "$lib/wasm/riskdice";

export type BattleConfig = {
	attackingTroops: number,
	defendingTroops: number,
	capitalDefending: boolean,
	zombiesDefending: boolean,
};

export type BattleDistrib = {
	attackerDamageProbs: number[],
	defenderDamageProbs: number[],
};

export interface BalanceConfig {
	winChanceCutoff: number,
	winChancePower: number,
	winOutcomeCutoff: number,
	winOutcomePower: number,
}

export function defaultBalanceConfig(): BalanceConfig {
	return {
		winChanceCutoff: 0.05,
		winChancePower: 1.3,
		winOutcomeCutoff: 0.1,
		winOutcomePower: 1.8,
	};
}

export class RiskDice {
	module: MainModule;

	constructor(module: MainModule) {
		this.module = module;
	}

	static async load(): Promise<RiskDice> {
		const emscriptenModule = await MainModuleFactory();
		console.log(emscriptenModule);
		return new RiskDice(emscriptenModule);
	}

	private mallocBattleConfig(config: BattleConfig): number {
		const ptr = this.module._malloc(12);
		let flags = 0;
		if (config.capitalDefending) flags |= 0x01;
		if (config.zombiesDefending) flags |= 0x02;
		this.module.setValue(ptr + 0, config.attackingTroops, "i32");
		this.module.setValue(ptr + 4, config.defendingTroops, "i32");
		this.module.setValue(ptr + 8, flags, "i32");
		return ptr;
	}

	private mallocBattleDistrib(): number {
		return this.module._malloc(16);
	}

	private mallocBalanceConfig(config: BalanceConfig): number {
		const ptr = this.module._malloc(32);
		this.module.setValue(ptr + 0, config.winChanceCutoff, "double");
		this.module.setValue(ptr + 8, config.winChancePower, "double");
		this.module.setValue(ptr + 16, config.winOutcomeCutoff, "double");
		this.module.setValue(ptr + 24, config.winOutcomePower, "double");
		return ptr;
	}

	calculateBattle(config: BattleConfig, balanceConfig: BalanceConfig | boolean = true): BattleDistrib | null {
		const distribPointer = this.mallocBattleDistrib();

		const configPointer = this.mallocBattleConfig(config);
		if (this.module._calc_battle_distrib(distribPointer, configPointer) < 0) {
			this.module._free(distribPointer);
			this.module._free(configPointer);
			return null;
		}
		this.module._free(configPointer);

		if (balanceConfig !== false) {
			if (balanceConfig === true) {
				balanceConfig = defaultBalanceConfig();
			}

			const balanceConfigPointer = this.mallocBalanceConfig(balanceConfig);
			if (this.module._apply_balance(distribPointer, balanceConfigPointer) < 0) {
				this.module._destroy_battle_distrib(distribPointer);
				this.module._free(distribPointer);
				this.module._free(balanceConfigPointer);
				return null;
			}
			this.module._free(balanceConfigPointer);
		}

		const distrib: BattleDistrib = {
			attackerDamageProbs: [],
			defenderDamageProbs: [],
		};

		const attackerDamageProbsPointer = this.module.getValue(distribPointer + 8, "double*");
		const defenderDamageProbsPointer = this.module.getValue(distribPointer + 12, "double*");

		for (let i = 0; i <= config.attackingTroops; ++i) {
			distrib.attackerDamageProbs.push(this.module.getValue(attackerDamageProbsPointer + 8 * i, "double"));
		}

		for (let i = 0; i <= config.defendingTroops; ++i) {
			distrib.defenderDamageProbs.push(this.module.getValue(defenderDamageProbsPointer + 8 * i, "double"));
		}

		this.module._destroy_battle_distrib(distribPointer);
		this.module._free(distribPointer);

		return distrib;
	}
}

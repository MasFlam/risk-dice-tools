"use strict";

function trimAlmostZeros(data) {
	const n = data.length;
	const maxprob = Math.max(...data);
	const cutoff = 0.0001 * maxprob;
	let i = 0;
	while (i < n - 1 && data[i] < cutoff) {
		++i;
	}
	data.splice(0, i);
	let j = 0;
	while (data.length > 1 && data[data.length - 1] < cutoff) {
		data.pop();
		++j;
	}
	return [i, j];
}

class RiskDice {
	constructor(mod) {
		this.mod = mod;
	}
	
	calcBattleDistrib(attacking, defending, capital, zombie, balanced) {
		// Remember that in C ABI:
		// - functions returning structs actually take a pointer to the output as the first argument
		// - structs are always passed by pointer, only then copied by the callee
		// Also remember about alignment and that our pointers are 32-bit.
		const mod = this.mod;
		
		//console.log(attacking, defending, capital ? 1 : 0, balanced);
		
		let battle_config = mod._malloc_battle_config();
		mod.setValue(battle_config + 0, attacking, "i32");
		mod.setValue(battle_config + 4, defending, "i32");
		mod.setValue(battle_config + 8, capital ? 1 : 0, "i32");
		mod.setValue(battle_config + 12, zombie ? 1 : 0, "i32");
		
		let battle_distrib = mod._malloc_battle_distrib();
		mod._calc_battle_distrib(battle_distrib, battle_config);
		
		let balance_config;
		
		if (balanced) {
			balance_config = mod._malloc_balance_config();
			mod.setValue(balance_config + 0, 0.05, "double");
			mod.setValue(balance_config + 8, 1.3, "double");
			mod.setValue(balance_config + 16, 0.1, "double");
			mod.setValue(balance_config + 24, 1.8, "double");
			
			mod._battle_distrib_apply_balance(battle_distrib, balance_config);
		}
		
		let result = {
			"attackingTroops": mod.getValue(battle_distrib + 0, "i32"),
			"defendingTroops": mod.getValue(battle_distrib + 4, "i32"),
			"attackerTroopLossProbs": [],
			"defenderTroopLossProbs": [],
			"attackerVictoryProb": mod.getValue(battle_distrib + 16, "double"),
			"defenderVictoryProb": mod.getValue(battle_distrib + 24, "double"),
		};
		
		let att_loss_probs = mod.getValue(battle_distrib + 8, "double*");
		let def_loss_probs = mod.getValue(battle_distrib + 12, "double*");
		
		for (let i = 0; i <= attacking; ++i) {
			let x = mod.getValue(att_loss_probs + i * 8, "double");
			result.attackerTroopLossProbs.push(x);
		}
		
		for (let i = 0; i <= defending; ++i) {
			let x = mod.getValue(def_loss_probs + i * 8, "double");
			result.defenderTroopLossProbs.push(x);
		}
		
		mod._free(battle_config);
		mod._destroy_battle_distrib(battle_distrib);
		mod._free(battle_distrib);
		
		if (balanced) {
			mod._free(balance_config);
		}
		
		return result;
	}
};

// Also returns the average troop loss (not counting the case when the side doesn't win),
// Or "N/A" when the side never wins.
function presentBattleChart(distrib, chart, isAttacking) {
	chart.options.scales = {
		y: {
			ticks: {
				callback: function (value, index, ticks) {
					return Chart.Ticks.formatters.numeric.apply(this, [100 * value, index, ticks]) + "%";
				},
			},
		},
	};
	
	const data = isAttacking ? distrib.attackerTroopLossProbs : distrib.defenderTroopLossProbs;
	const popped = data.pop(); // remove case when the side doesn't win
	
	let averageLoss = 0.0;
	let sumProbs = 0.0;
	for (let loss = 0; loss < data.length; ++loss) {
		averageLoss += loss * data[loss];
		sumProbs += data[loss];
	}
	averageLoss /= sumProbs;
	
	if (Math.abs(1.0 - popped) <= Number.EPSILON) {
		averageLoss = "N/A";
	}
	
	const dataStart = trimAlmostZeros(data)[0];
	const labels = new Array(data.length);
	for (let i = 0; i < labels.length; ++i) {
		labels[i] = `${dataStart + i}`;
	}
	
	chart.data.labels = labels;
	
	chart.data.datasets = [
		{
			label: `${isAttacking ? "Attacker" : "Defender"} troop loss distribution`,
			data: data,
		},
	];
	
	return averageLoss;
}

class TroopsNeededCalc {
	constructor(dice, idPrefix) {
		this.dice = dice;
		this.idPrefix = idPrefix;
		this.inpDefending = document.getElementById(`${idPrefix}-defending`);
		this.inpWanted = document.getElementById(`${idPrefix}-wanted`);
		this.inpCapital = document.getElementById(`${idPrefix}-capital`);
		this.inpZombie = document.getElementById(`${idPrefix}-zombie`);
		this.inpBalanced = document.getElementById(`${idPrefix}-balanced`);
		this.btnSubmit = document.getElementById(`${idPrefix}-submit`);
		this.spanResult = document.getElementById(`${idPrefix}-result`);
		this.spanAvgAtt = document.getElementById(`${idPrefix}-avg-att`);
		this.spanAvgDef = document.getElementById(`${idPrefix}-avg-def`);
		this.canvasChartAtt = document.getElementById(`${idPrefix}-chart-att`);
		this.canvasChartDef = document.getElementById(`${idPrefix}-chart-def`);
		
		this.btnSubmit.addEventListener("click", _ => {
			try {
				this.run();
			} catch (e) {
				console.error(e);
				this.spanResult.innerText = "error";
			}
		});
		
		this.chartAtt = new Chart(
			this.canvasChartAtt,
			{
				type: "bar",
				options: {
					borderColor: "#4f8af0",
					backgroundColor: "#4f8af0",
					pointStyle: false,
				},
				data: {
					datasets: [],
				},
			}
		);
		
		this.chartDef = new Chart(
			this.canvasChartDef,
			{
				type: "bar",
				options: {
					borderColor: "#ff427a",
					backgroundColor: "#ff427a",
					pointStyle: false,
				},
				data: {
					datasets: [],
				},
			}
		);
	}
	
	run() {
		const defending = this.inpDefending.valueAsNumber;
		const wanted = this.inpWanted.valueAsNumber;
		const capital = this.inpCapital.checked;
		const zombie = this.inpZombie.checked;
		const balanced = this.inpBalanced.checked;
		
		// Binary search for the amount.
		// It's a bit tricky with the floating point inaccuracy.
		// So idk this might be off by 1 sometimes.
		
		let p = 1;
		let q = 3 * defending;
		
		for (let i = 0; i < Math.log2(q) + 2; ++i) {
			const attacking = Math.floor((p + q) / 2);
			const result = this.dice.calcBattleDistrib(attacking, defending, capital, zombie, balanced);
			//console.log(p, q, Math.abs(result.attackerVictoryProb - wanted), result);
			if (Math.abs(result.attackerVictoryProb - wanted) <= Number.EPSILON) {
				q = attacking;
			} else if (result.attackerVictoryProb < wanted) {
				p = attacking + 1;
			} else if (result.attackerVictoryProb > wanted) {
				q = attacking;
			}
		}
		
		// +1 for the troop left behind
		this.spanResult.innerText = q + 1;
		
		const result = this.dice.calcBattleDistrib(q, defending, capital, zombie, balanced);
		
		this.spanAvgAtt.innerText = presentBattleChart(result, this.chartAtt, true);
		this.spanAvgDef.innerText = presentBattleChart(result, this.chartDef, false);
		
		this.chartAtt.update();
		this.chartDef.update();
	}
};

class BattleCalc {
	constructor(dice, idPrefix) {
		this.dice = dice;
		this.idPrefix = idPrefix;
		this.inpAttacking = document.getElementById(`${idPrefix}-attacking`);
		this.inpDefending = document.getElementById(`${idPrefix}-defending`);
		this.inpCapital = document.getElementById(`${idPrefix}-capital`);
		this.inpZombie = document.getElementById(`${idPrefix}-zombie`);
		this.inpBalanced = document.getElementById(`${idPrefix}-balanced`);
		this.btnSubmit = document.getElementById(`${idPrefix}-submit`);
		this.spanProb = document.getElementById(`${idPrefix}-prob`);
		this.spanAvgAtt = document.getElementById(`${idPrefix}-avg-att`);
		this.spanAvgDef = document.getElementById(`${idPrefix}-avg-def`);
		this.canvasChartAtt = document.getElementById(`${idPrefix}-chart-att`);
		this.canvasChartDef = document.getElementById(`${idPrefix}-chart-def`);
		
		this.btnSubmit.addEventListener("click", _ => {
			try {
				this.run();
			} catch (e) {
				console.error(e);
				this.spanProb.innerText = "error";
			}
		});
		
		this.chartAtt = new Chart(
			this.canvasChartAtt,
			{
				type: "bar",
				options: {
					borderColor: "#4f8af0",
					backgroundColor: "#4f8af0",
					pointStyle: false,
				},
				data: {
					datasets: [],
				},
			}
		);
		
		this.chartDef = new Chart(
			this.canvasChartDef,
			{
				type: "bar",
				options: {
					borderColor: "#ff427a",
					backgroundColor: "#ff427a",
					pointStyle: false,
				},
				data: {
					datasets: [],
				},
			}
		);
	}
	
	run() {
		const attacking = this.inpAttacking.valueAsNumber - 1;
		const defending = this.inpDefending.valueAsNumber;
		const capital = this.inpCapital.checked;
		const zombie = this.inpZombie.checked;
		const balanced = this.inpBalanced.checked;
		
		const result = this.dice.calcBattleDistrib(attacking, defending, capital, zombie, balanced);
		
		this.spanProb.innerText = result.attackerVictoryProb;
		
		this.spanAvgAtt.innerText = presentBattleChart(result, this.chartAtt, true);
		this.spanAvgDef.innerText = presentBattleChart(result, this.chartDef, false);
		
		this.chartAtt.update();
		this.chartDef.update();
	}
};

window.onload = async () => {
	const mod = await Module();
	console.log(mod);
	
	const riskdice = new RiskDice(mod);
	
	const troopsNeededCalc = new TroopsNeededCalc(riskdice, "tool1");
	
	const battleCalc = new BattleCalc(riskdice, "tool2");
};

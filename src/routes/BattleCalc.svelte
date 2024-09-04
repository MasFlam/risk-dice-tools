<script lang="ts">
	import { RiskDice, type BattleDistrib } from "$lib/riskdice";
	import { SlideToggle } from "@skeletonlabs/skeleton";
	import BattleChart from "./BattleChart.svelte";

	export let riskdice: RiskDice;

	$: attackerVictoryProb = null as number | null;
	$: attackerDamageAvg = null as number | null;
	$: defenderDamageAvg = null as number | null;

	let attackingStack: number = 2;
	let defendingStack: number = 1;
	let defendingCapital: boolean = false;
	let defendingZombies: boolean = false;
	let balancedBlitz: boolean = true;

	$: distrib = null as BattleDistrib | null;

	function recalculate() {
		const natt = attackingStack - 1;
		const ndef = defendingStack;

		const distribTemp = riskdice.calculateBattle(
			{
				attackingTroops: natt,
				defendingTroops: ndef,
				capitalDefending: defendingCapital,
				zombiesDefending: defendingZombies,
			},
			balancedBlitz
		);

		if (!distribTemp) {
			distrib = null;
			attackerVictoryProb = null;
			attackerDamageAvg = null;
			defenderDamageAvg = null;
		} else {
			// Probabilities of victory for each side:
			const victoryProbAttacker = distribTemp.defenderDamageProbs[ndef];
			const victoryProbDefender = distribTemp.attackerDamageProbs[natt];

			// The average troop loss on victory for each side:
			let avgLossAttacker = 0.0 as number | null;
			let avgLossDefender = 0.0 as number | null;

			for (let i = 0; i < ndef; ++i) {
				avgLossDefender! += distribTemp.defenderDamageProbs[i] * i;
			}

			if (victoryProbDefender > 0) {
				avgLossDefender! /= victoryProbDefender;
			} else {
				avgLossDefender = null;
			}

			for (let i = 0; i < natt; ++i) {
				avgLossAttacker! += distribTemp.attackerDamageProbs[i] * i;
			}

			if (victoryProbAttacker > 0) {
				avgLossAttacker! /= victoryProbAttacker;
			} else {
				avgLossAttacker = null;
			}

			// Update the UI except the chart:
			attackerVictoryProb = distribTemp.defenderDamageProbs[ndef];
			attackerDamageAvg = avgLossAttacker;
			defenderDamageAvg = avgLossDefender;

			// Update the chart: (done here because it alters the distrib arrays)
			distrib = distribTemp;
		}
	}
</script>

<div
	class="max-w-xl p-6 pt-4 bg-surface-100-800-token border border-surface-300-600-token rounded-lg space-y-4"
>
	<header class="text-2xl text-center font-bold">Battle Calculator</header>
	<hr />
	<form
		class="grid grid-cols-2 gap-4"
		on:submit={(e) => {
			e.preventDefault();
			recalculate();
		}}
	>
		<div>
			<label class="label">
				<div class="px-2 text-center">Attacking stack:</div>
				<input
					type="number"
					class="input text-2xl px-4"
					step="1"
					min="2"
					bind:value={attackingStack}
				/>
			</label>
		</div>
		<div>
			<label class="label">
				<div class="px-2 text-center">Defending stack:</div>
				<input
					type="number"
					class="input text-2xl px-4"
					step="1"
					min="1"
					bind:value={defendingStack}
				/>
			</label>
		</div>
		<SlideToggle name="toggle-capital" bind:checked={defendingCapital}>
			<div class="cursor-pointer">Capital defending?</div>
		</SlideToggle>
		<SlideToggle name="toggle-zombies" bind:checked={defendingZombies}>
			<div class="cursor-pointer">Zombies defending?</div>
		</SlideToggle>
		<SlideToggle name="toggle-balance" bind:checked={balancedBlitz}>
			<div class="cursor-pointer">Balanced Blitz?</div>
		</SlideToggle>
		<button class="btn variant-filled-primary" type="submit">Calculate</button>
	</form>
	<hr />
	<div>
		<div class="text-center">Probability of attacker victory:</div>
		<div class="text-2xl text-center font-bold">
			{#if attackerVictoryProb !== null}
				{(attackerVictoryProb * 100).toFixed(0)}%
			{:else}
				&mdash;
			{/if}
		</div>
	</div>
	<div>
		<div class="grid grid-cols-2 gap-4">
			<div>
				<div class="text-center">Average attacker<br />damage on victory:</div>
				<div class="text-xl text-center font-bold">
					{#if attackerDamageAvg !== null}
						{attackerDamageAvg?.toFixed(2)} troops
					{:else}
						&mdash;
					{/if}
				</div>
			</div>
			<div>
				<div class="text-center">Average defender<br />damage on victory:</div>
				<div class="text-xl text-center font-bold">
					{#if defenderDamageAvg !== null}
						{defenderDamageAvg?.toFixed(2)} troops
					{:else}
						&mdash;
					{/if}
				</div>
			</div>
		</div>
	</div>
	<BattleChart {distrib}></BattleChart>
</div>

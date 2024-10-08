<script lang="ts">
	import type { BattleDistrib, RiskDice } from "$lib/riskdice";
	import { SlideToggle } from "@skeletonlabs/skeleton";
	import BattleChart from "./BattleChart.svelte";

	export let riskdice: RiskDice;

	let attackingSide: boolean = true;
	let opponentStack: number = 1;
	let wantedProbPercent: number = 100;
	let defendingCapital: boolean = false;
	let defendingZombies: boolean = false;
	let balancedBlitz: boolean = true;
	
	let stackNeeded: number | null = null;
	let distrib: BattleDistrib | null = null;

	function roundProb(prob: number) {
		// Round to sixteen bits of precision (`prob` is normalized to [0, 1])
		const roundingFactor = 65536;
		return Math.round(prob * roundingFactor) / roundingFactor;
	}

	function recalculate() {
		// Binary search the minimum defending/attacking stack needed.
		// The caveat is of course that we are working with floating point numbers,
		// so what we do is we always round the probability and assume that's enough
		// to maintain monotonicity. We are only searching for multiples of 1%,
		// so as long as we round to a less significant place, we *should* be fine.

		const wantedProb = wantedProbPercent * 0.01;

		let p = 1;
		let q = 5 * opponentStack;
		let distribTemp: BattleDistrib | null = null;

		while (true) {
			const ourStack = Math.floor((p + q) / 2);
			distribTemp = riskdice.calculateBattle(
				{
					attackingTroops: (attackingSide ? ourStack : opponentStack) - 1,
					defendingTroops: attackingSide ? opponentStack : ourStack,
					capitalDefending: defendingCapital,
					zombiesDefending: defendingZombies,
				},
				balancedBlitz
			);
			
			if (p >= q) break;

			if (!distribTemp) {
				stackNeeded = null;
				distrib = null;
				return;
			} else {
				const natt = distribTemp.attackerDamageProbs.length - 1;
				const ndef = distribTemp.defenderDamageProbs.length - 1;

				const victoryProbAtt = distribTemp.defenderDamageProbs[ndef];
				const victoryProbDef = distribTemp.attackerDamageProbs[natt];

				const victoryProb = roundProb(attackingSide ? victoryProbAtt : victoryProbDef);
				
				console.log("====================");
				console.log(`p=${p}, q=${q}`);
				console.log(`our=${ourStack}, opponent=${opponentStack}`);
				console.log(distribTemp);
				console.log(`P(att vic) = ${victoryProbAtt}`);
				console.log(`P(def vic) = ${victoryProbDef}`);
				console.log(`rounded victory prob = ${victoryProb}`);
				
				if (victoryProb < wantedProb) {
					p = ourStack + 1;
				} else {
					q = ourStack;
				}
			}
		}

		stackNeeded = p;
		distrib = distribTemp;
	}
	
	function onSideChange() {
		if (!attackingSide) {
			opponentStack = Math.max(opponentStack, 2);
		}
	}
	
	function onBalancedBlitzChange() {
		if (!balancedBlitz) {
			wantedProbPercent = Math.max(1, Math.min(99, wantedProbPercent));
		}
	}
</script>

<div
	class="max-w-xl p-6 pt-4 bg-surface-100-800-token border border-surface-300-600-token rounded-lg space-y-4"
>
	<header class="text-2xl text-center font-bold">Troops Needed Calculator</header>
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
				<div class="px-2 text-center">
					<!--Opponent stack:-->
					{#if attackingSide}
						Defending stack:
					{:else}
						Attacking stack:
					{/if}
				</div>
				<input
					type="number"
					class="input text-2xl px-4"
					step="1"
					min={attackingSide ? 1 : 2}
					bind:value={opponentStack}
				/>
			</label>
		</div>
		<div>
			<label class="label">
				<div class="px-2 text-center">Probability of victory:</div>
				<div class="input-group input-group-divider grid-cols-[1fr_auto]">
					<input
						type="number"
						class="text-2xl px-4"
						step="1"
						min={balancedBlitz ? 0 : 1}
						max={balancedBlitz ? 100 : 99}
						bind:value={wantedProbPercent}
					/>
					<div class="input-group-shim text-2xl">%</div>
				</div>
			</label>
		</div>
		<SlideToggle name="toggle-capital" bind:checked={defendingCapital}>
			<div class="cursor-pointer">Capital defending?</div>
		</SlideToggle>
		<SlideToggle name="toggle-zombies" bind:checked={defendingZombies}>
			<div class="cursor-pointer">Zombies defending?</div>
		</SlideToggle>
		<SlideToggle name="toggle-balance" bind:checked={balancedBlitz} on:change={onBalancedBlitzChange}>
			<div class="cursor-pointer">Balanced Blitz?</div>
		</SlideToggle>
		<SlideToggle name="toggle-balance" background="bg-cyan-600" active="bg-red-600" bind:checked={attackingSide} on:change={onSideChange}>
			<div class="cursor-pointer">
				I am
				<b>
					{#if attackingSide}
						attacking
					{:else}
						defending
					{/if}
				</b>
			</div>
		</SlideToggle>
		<button class="btn variant-filled-primary col-span-2" type="submit">Calculate</button>
	</form>
	<hr />
	<div>
		<div class="text-center">
			{#if attackingSide}
				Attacking stack needed:
			{:else}
				Defending stack needed:
			{/if}
		</div>
		<div class="text-2xl text-center font-bold">
			{#if stackNeeded !== null}
				{stackNeeded}
			{:else}
				&mdash;
			{/if}
		</div>
	</div>
	<BattleChart {distrib}></BattleChart>
</div>

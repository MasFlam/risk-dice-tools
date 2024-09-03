<script lang="ts">
	import { RiskDice } from "$lib/riskdice";
	import BattleCalc from "./BattleCalc.svelte";
	import TroopsNeededCalc from "./TroopsNeededCalc.svelte";

	let riskdicePromise = RiskDice.load();
</script>

<div class="p-4 flex flex-wrap justify-center gap-8">
	{#await riskdicePromise}
		<span>Loading...</span>
	{:then riskdice}
		<BattleCalc {riskdice} />
		<TroopsNeededCalc {riskdice} />
	{/await}
</div>

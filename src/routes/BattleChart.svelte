<script lang="ts">
	import colors from "$lib/colors";
	import { formatPercentage } from "$lib/formatting";
	import { type BattleDistrib } from "$lib/riskdice";
	import { modeCurrent } from "@skeletonlabs/skeleton";
	import type { ChartData, ChartOptions, TooltipItem } from "chart.js";
	import {
		BarElement,
		CategoryScale,
		Chart as ChartJS,
		Legend,
		LinearScale,
		LineElement,
		PointElement,
		Title,
		Tooltip,
	} from "chart.js";
	import annotationPlugin from "chartjs-plugin-annotation";
	import { Line } from "svelte-chartjs";

	ChartJS.register(
		Title,
		Tooltip,
		Legend,
		BarElement,
		PointElement,
		LineElement,
		CategoryScale,
		LinearScale
	);
	ChartJS.register(annotationPlugin);

	export let distrib: BattleDistrib | null;

	$: chartOptions = {
		backgroundColor: colors.attacker,
		scales: {
			y: {
				title: {
					display: false,
					text: "Probability",
					color: $modeCurrent ? "black" : "white",
				},
				ticks: {
					color: $modeCurrent ? "black" : "white",
					callback: function (value, index, ticks) {
						return formatPercentage(Math.abs(value as number));
					},
				},
				grid: {
					color: $modeCurrent ? "rgba(0, 0, 0, 0.3)" : "rgba(255, 255, 255, 0.3)",
				},
			},
			x: {
				title: {
					display: true,
					text: "Troops delta / Troop loss",
					color: $modeCurrent ? "black" : "white",
				},
				ticks: {
					color: $modeCurrent ? "black" : "white",
				},
				grid: {
					color: $modeCurrent ? "rgba(0, 0, 0, 0.3)" : "rgba(255, 255, 255, 0.3)",
				},
			},
		},
		plugins: {
			legend: {
				labels: {
					color: $modeCurrent ? "black" : "white",
				},
			},
			tooltip: {
				callbacks: {
					label: function (ctx: TooltipItem<"line">) {
						return formatPercentage(ctx.parsed.y);
					},
				},
			},
		},
	} as ChartOptions<"line">;

	let chartData: ChartData<"line", number[]> = { datasets: [] };

	$: {
		if (!distrib) {
			chartData = { datasets: [] };
		} else {
			const natt = distrib.attackerDamageProbs.length - 1;
			const ndef = distrib.defenderDamageProbs.length - 1;

			// The outcome probabilities will be made negative when the defender wins,
			// so that they appear below the X axe on the chart.
			const dataProbs: number[] = new Array(natt + ndef);

			// This is the troop delta, which is just `defender_troop_loss - attacker_troop_loss`.
			const dataDelta: number[] = new Array(natt + ndef);

			// These are the troop losses of the winning side in any given outcome.
			const dataDamage: number[] = new Array(natt + ndef);

			// In order to later hide negligible probability outcomes from the chart,
			// we calculate the maximum amount any outcome probability differs from 0.
			let maxAbsProb = 0.0;

			for (let i = 0; i < ndef; ++i) {
				const prob = distrib.defenderDamageProbs[i];
				dataProbs[i] = -prob;
				dataDamage[i] = i;
				dataDelta[i] = -natt + i;
				maxAbsProb = Math.max(maxAbsProb, prob);
			}

			for (let i = 0; i < natt; ++i) {
				const idx = natt + ndef - 1 - i;
				const prob = distrib.attackerDamageProbs[i];
				dataProbs[idx] = prob;
				dataDamage[idx] = i;
				dataDelta[idx] = +ndef - i;
				maxAbsProb = Math.max(maxAbsProb, prob);
			}

			// Now we will hide the outcomes with negligible probabilities from the chart by cutting
			// them off both ends of our data arrays. This makes the chart more readable.

			// Threshold for the probability below which outcomes are to be hidden.
			const threshold = Math.min(0.0001, maxAbsProb * 0.01);

			// Numbers of data points we hide from each end:
			let cutoffLeft = 0;
			let cutoffRight = 0;

			// Cut from the left:
			while (cutoffLeft < natt + ndef - 1 && Math.abs(dataProbs[cutoffLeft]) < threshold) {
				++cutoffLeft;
			}

			// Cut from the right:
			while (
				cutoffLeft + cutoffRight < natt + ndef - 1 &&
				Math.abs(dataProbs[natt + ndef - 1 - cutoffRight]) < threshold
			) {
				++cutoffRight;
			}

			// If we cut off only one data point, it's probably the last one reduced by balanced blitz.
			// Might as well show it then.
			if (cutoffLeft <= 1) cutoffLeft = 0;
			if (cutoffRight <= 1) cutoffRight = 0;

			// Finally cut the arrays:
			const dataProbsCut = dataProbs.slice(cutoffLeft, natt + ndef - cutoffRight);
			const dataDeltaCut = dataDelta.slice(cutoffLeft, natt + ndef - cutoffRight);
			const dataDamageCut = dataDamage.slice(cutoffLeft, natt + ndef - cutoffRight);

			// Update the chart data object:
			chartData = {
				labels: dataDeltaCut.map((delta, idx) => {
					const side = idx < ndef - cutoffLeft ? "D" : "A";
					const damage = dataDamageCut[idx];
					return [`Δ ${delta > 0 ? "+" : ""}${delta}`, `${side} -${damage}`];
				}),
				datasets: [
					{
						label: "Probability",
						data: dataProbsCut,
						pointStyle: false,
						borderColor: colors.defender,
						backgroundColor: colors.defender,
						tension: 0.1,
					},
				],
			};

			// Show a line marking the point at which victory "changes sides":
			const victoryX = ndef - cutoffLeft - 0.5;
			chartOptions.plugins!.annotation = {
				annotations: {
					victoryLine: {
						type: "line",
						scaleID: "x",
						value: victoryX,
						endValue: victoryX,
						borderColor: colors.attacker,
					},
				},
			};
		}
	}
</script>

<Line options={chartOptions} data={chartData} />

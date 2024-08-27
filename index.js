"use strict";

function trimAlmostZeros(data) {
	const n = data.length;
	let i = 0;
	while (i < n - 1 && data[i] < 0.0001) {
		++i;
	}
	data.splice(0, i);
	let j = 0;
	while (data.length > 1 && data[data.length - 1] < 0.0001) {
		data.pop();
		++j;
	}
	return [i, j];
}

class RiskDice {
	constructor(mod) {
		this.mod = mod;
	}
	
	calcBattleDistrib(attacking, defending, capital, balanced) {
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

class TroopsNeededCalc {
	constructor(dice, idPrefix) {
		this.dice = dice;
		this.idPrefix = idPrefix;
		this.inpDefending = document.getElementById(`${idPrefix}-defending`);
		this.inpWanted = document.getElementById(`${idPrefix}-wanted`);
		this.inpCapital = document.getElementById(`${idPrefix}-capital`);
		this.inpBalanced = document.getElementById(`${idPrefix}-balanced`);
		this.btnSubmit = document.getElementById(`${idPrefix}-submit`);
		this.spanResult = document.getElementById(`${idPrefix}-result`);
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
		const balanced = this.inpBalanced.checked;
		
		// Binary search for the amount:
		
		let p = 1;
		let q = 3 * defending;
		
		for (let i = 0; i < Math.log2(q) + 2; ++i) {
			const attacking = Math.floor((p + q) / 2);
			const result = this.dice.calcBattleDistrib(attacking, defending, capital, balanced);
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
		
		const result = this.dice.calcBattleDistrib(q, defending, capital, balanced);
		
		this.chartAtt.options.scales = {
			y: {
				ticks: {
					callback: function (value, index, ticks) {
						return Chart.Ticks.formatters.numeric.apply(this, [100 * value, index, ticks]) + "%";
						//return `${100 * value}%`;
					},
				},
			},
		};
		
		const dataAtt = result.attackerTroopLossProbs;
		dataAtt.pop();
		const dataAttStart = trimAlmostZeros(dataAtt)[0];
		
		const labelsAtt = new Array(dataAtt.length);
		for (let i = 0; i < labelsAtt.length; ++i) {
			labelsAtt[i] = `${dataAttStart + i}`;
		}
		console.log(dataAtt)
		console.log(labelsAtt)
		
		this.chartAtt.data.labels = labelsAtt;
		
		this.chartAtt.data.datasets = [
			{
				label: "Attacker troop loss distribution",
				data: dataAtt,
			},
		];
		
		this.chartDef.options.scales = {
			y: {
				ticks: {
					callback: function (value, index, ticks) {
						return Chart.Ticks.formatters.numeric.apply(this, [100 * value, index, ticks]) + "%";
						//return `${Math.round(100 * value)}%`;
					},
				},
			},
		};
		
		const dataDef = result.defenderTroopLossProbs;
		dataDef.pop();
		const dataDefStart = trimAlmostZeros(dataDef)[0];
		
		const labelsDef = new Array(dataDef.length);
		for (let i = 0; i < labelsDef.length; ++i) {
			labelsDef[i] = `${dataDefStart + i}`;
		}
		
		this.chartDef.data.labels = labelsDef;
		
		this.chartDef.data.datasets = [
			{
				label: "Defender troop loss distribution",
				data: dataDef,
			},
		];
		
		this.chartAtt.update();
		this.chartDef.update();
	}
};

class BattleCalc {
	
};

window.onload = async () => {
	const mod = await Module();
	console.log(mod);
	
	const riskdice = new RiskDice(mod);
	
	const troopsNeededCalc = new TroopsNeededCalc(riskdice, "tool1");
	/*
	let att_range = document.getElementById("att_troops_range");
	let def_range = document.getElementById("def_troops_range");
	let cap_checkbox = document.getElementById("capital_checkbox");
	let canvas = document.getElementById("distribs_canvas");
	let inp_att_troops = document.getElementById("att_troops");
	let inp_def_troops = document.getElementById("def_troops");
	let span_att_win = document.getElementById("att_win_prob");
	let span_def_win = document.getElementById("def_win_prob");
	
	let chart = new Chart(
		canvas,
		{
			type: "line",
			options: {
				reactive: true,
				animation: false,
				pointStyle: false,
			},
			data: {
				datasets: [],
			}	
		}
	);
	
	function calc_battle_distrib(attacking, defending, capital) {
		// Remember that in C ABI:
		// - functions returning structs actually take a pointer to the output as the first argument
		// - structs are always passed by pointer, only then copied by the callee
		// Also remember about alignment and that our pointers are 32-bit.
		
		let battle_config = mod._malloc_battle_config();
		mod.setValue(battle_config + 0, attacking, "i32");
		mod.setValue(battle_config + 4, defending, "i32");
		mod.setValue(battle_config + 8, capital ? 1 : 0, "i32");
		
		let battle_distrib = mod._malloc_battle_distrib();
		mod._calc_battle_distrib(battle_distrib, battle_config);
		
		let balance_config = mod._malloc_balance_config();
		mod.setValue(balance_config + 0, 0.05, "double");
		mod.setValue(balance_config + 8, 1.3, "double");
		mod.setValue(balance_config + 16, 0.1, "double");
		mod.setValue(balance_config + 24, 1.8, "double");
		
		mod._battle_distrib_apply_balance(battle_distrib, balance_config);
		
		let result = {
			"att_troops": mod.getValue(battle_distrib + 0, "i32"),
			"def_troops": mod.getValue(battle_distrib + 4, "i32"),
			"att_loss_probs": [],
			"def_loss_probs": [],
			"att_victory_prob": mod.getValue(battle_distrib + 16, "double"),
			"def_victory_prob": mod.getValue(battle_distrib + 24, "double"),
		};
		
		let att_loss_probs = mod.getValue(battle_distrib + 8, "double*");
		let def_loss_probs = mod.getValue(battle_distrib + 12, "double*");
		
		for (let i = 0; i <= attacking; ++i) {
			let x = mod.getValue(att_loss_probs + i * 8, "double");
			result.att_loss_probs.push(x);
		}
		
		for (let i = 0; i <= defending; ++i) {
			let x = mod.getValue(def_loss_probs + i * 8, "double");
			result.def_loss_probs.push(x);
		}
		
		mod._free(battle_config);
		mod._free(balance_config);
		mod._destroy_battle_distrib(battle_distrib);
		mod._free(battle_distrib);
		
		return result;
	}
	
	function update_charts() {
		let result = calc_battle_distrib(att_range.value, def_range.value, cap_checkbox.checked);
		console.log(result);
		
		inp_att_troops.value = result.att_troops;
		inp_def_troops.value = result.def_troops;
		span_att_win.innerText = result.att_victory_prob;
		span_def_win.innerText = result.def_victory_prob;
		
		let labels = new Array(Math.max(result.att_troops, result.def_troops) + 1);
		for (let i = 0; i < labels.length; ++i) {
			labels[i] = `${i + 1}`;
		}
		
		chart.data.labels = labels;
		chart.data.datasets = [
			{
				label: "P(Attacker ends up losing X troops)",
				data: result.att_loss_probs,
			},
			{
				label: "P(Defender ends up losing X troops)",
				data: result.def_loss_probs,
			},
		];
		
		chart.update();
		
		console.log("Charts updated");
	}
	
	att_range.addEventListener("input", evt => update_charts());
	def_range.addEventListener("input", evt => update_charts());
	cap_checkbox.addEventListener("input", evt => update_charts());
	
	inp_att_troops.addEventListener("input", evt => {
		att_range.value = inp_att_troops.value;
		update_charts();
	});
	
	inp_def_troops.addEventListener("input", evt => {
		def_range.value = inp_def_troops.value;
		update_charts();
	});
	
	update_charts();
	*/
};

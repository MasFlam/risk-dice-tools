#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "riskdice.h"

// Sorts ints in ascending order
static inline void sortints(int *arr, int n) {
	// insertion sort -- n is like really small here
	for (int i = 1; i < n; ++i) {
		int val = arr[i];
		int j = i - 1;
		while (j >= 0 && arr[j] > val) {
			arr[j + 1] = arr[j];
			--j;
		}
		arr[j + 1] = val;
	}
}

RollOutcome calc_roll_outcome(int att_dice_count, int def_dice_count, int *att_dice, int *def_dice) {
	RollOutcome outcome = {0, 0};
	
	int att[att_dice_count];
	int def[def_dice_count];
	memcpy(att, att_dice, att_dice_count * sizeof(int));
	memcpy(def, def_dice, def_dice_count * sizeof(int));
	
	sortints(att, att_dice_count);
	sortints(def, def_dice_count);
	
	// n = min(att count, def count)
	int n = att_dice_count > def_dice_count ? def_dice_count : att_dice_count;
	
	for (int i = 0; i < n; ++i) {
		if (att[att_dice_count - 1 - i] > def[def_dice_count - 1 - i]) {
			++outcome.def_loss;
		} else {
			++outcome.att_loss;
		}
	}
	
	return outcome;
}

RollDistrib *malloc_roll_distrib() {
	return (RollDistrib *) malloc(sizeof(RollDistrib));
}

RollDistrib calc_roll_distrib(int att_dice_count, int def_dice_count) {
	const int n = att_dice_count;
	const int m = def_dice_count;
	
	// len = min(n, m) + 1
	const int len = (n > m ? m : n) + 1;
	
	RollDistrib distrib = {
		.length = len,
		.counts_sum = 0,
		.outcomes = malloc(len * sizeof(RollOutcome)),
		.counts = calloc(len, sizeof(int)),
	};
	
	int att[n];
	for (int i = 0; i < n; ++i) att[i] = 1;
	
	while (1) {
		int def[m];
		for (int i = 0; i < m; ++i) def[i] = 1;
		
		while (1) {
			RollOutcome outcome = calc_roll_outcome(n, m, att, def);
			distrib.outcomes[outcome.def_loss] = outcome;
			++distrib.counts[outcome.def_loss];
			++distrib.counts_sum;
			
			int done = 1;
			for (int i = 0; i < m; ++i) {
				if (def[i] == 6) {
					def[i] = 1;
				} else {
					++def[i];
					done = 0;
					break;
				}
			}
			if (done) break;
		}
		
		int done = 1;
		for (int i = 0; i < n; ++i) {
			if (att[i] == 6) {
				att[i] = 1;
			} else {
				++att[i];
				done = 0;
				break;
			}
		}
		if (done) break;
	}
	
	return distrib;
}

void destroy_roll_distrib(RollDistrib *distrib) {
	free(distrib->outcomes);
	free(distrib->counts);
}

BattleDistrib *malloc_battle_distrib() {
	return (BattleDistrib *) malloc(sizeof(BattleDistrib));
}

BattleConfig *malloc_battle_config() {
	return (BattleConfig *) malloc(sizeof(BattleConfig));
}

BattleDistrib calc_battle_distrib(const BattleConfig *config) {
	const int n = config->att_troops;
	const int m = config->def_troops;
	const int capital = config->is_capital != 0;
	
	BattleDistrib distrib = {
		.att_troops = n,
		.def_troops = m,
		.att_loss_probs = calloc(n + 1, sizeof(double)),
		.def_loss_probs = calloc(m + 1, sizeof(double)),
		.att_victory_prob = 0.0,
		.def_victory_prob = 0.0,
	};
	
	// dp[i][j] = P(at some point in time the attacker had `i` troops and the defender had `j`)
	#define DP(i, j) dp[(i) * (m + 1) + (j)]
	double *dp = calloc((n + 1) * (m + 1), sizeof(double));
	
	DP(n, m) = 1.0;
	
	for (int i = n; i >= 3; --i) {
		for (int j = m; j >= 2 + capital; --j) {
			if (capital) {
				// 3v3 dice roll
				DP(i - 0, j - 3) += ( 6420.0 / 46656.0) * DP(i, j);
				DP(i - 1, j - 2) += (10017.0 / 46656.0) * DP(i, j);
				DP(i - 2, j - 1) += (12348.0 / 46656.0) * DP(i, j);
				DP(i - 3, j - 0) += (17871.0 / 46656.0) * DP(i, j);
			} else {
				// 3v2 dice roll
				DP(i - 0, j - 2) += (2890.0 / 7776.0) * DP(i, j);
				DP(i - 1, j - 1) += (2611.0 / 7776.0) * DP(i, j);
				DP(i - 2, j - 0) += (2275.0 / 7776.0) * DP(i, j);
			}
		}
		if (capital) {
			// 3v2 dice roll
			int j = 2;
			DP(i - 0, j - 2) += (2890.0 / 7776.0) * DP(i, j);
			DP(i - 1, j - 1) += (2611.0 / 7776.0) * DP(i, j);
			DP(i - 2, j - 0) += (2275.0 / 7776.0) * DP(i, j);
		}
		{
			// 3v1 dice roll
			int j = 1;
			DP(i - 0, j - 1) += (855.0 / 1296.0) * DP(i, j);
			DP(i - 1, j - 0) += (441.0 / 1296.0) * DP(i, j);
		}
	}
	{
		int i = 2;
		for (int j = m; j >= 2 + capital; --j) {
			if (capital) {
				// 2v3 dice roll
				DP(i - 0, j - 2) += (2275.0 / 7776.0) * DP(i, j);
				DP(i - 1, j - 1) += (2611.0 / 7776.0) * DP(i, j);
				DP(i - 2, j - 0) += (2890.0 / 7776.0) * DP(i, j);
			} else {
				// 2v2 dice roll
				DP(i - 0, j - 2) += (295.0 / 1296.0) * DP(i, j);
				DP(i - 1, j - 1) += (420.0 / 1296.0) * DP(i, j);
				DP(i - 2, j - 0) += (581.0 / 1296.0) * DP(i, j);
			}
		}
		if (capital) {
			// 2v2 dice roll
			int j = 2;
			DP(i - 0, j - 2) += (295.0 / 1296.0) * DP(i, j);
			DP(i - 1, j - 1) += (420.0 / 1296.0) * DP(i, j);
			DP(i - 2, j - 0) += (581.0 / 1296.0) * DP(i, j);
		}
		{
			// 2v1 dice roll
			int j = 1;
			DP(i - 0, j - 1) += (125.0 / 216.0) * DP(i, j);
			DP(i - 1, j - 0) += ( 91.0 / 216.0) * DP(i, j);
		}
	}
	{
		int i = 1;
		for (int j = m; j >= 2 + capital; --j) {
			if (capital) {
				// 1v3 dice roll
				DP(i - 0, j - 1) += (441.0 / 1296.0) * DP(i, j);
				DP(i - 1, j - 0) += (855.0 / 1296.0) * DP(i, j);
			} else {
				// 1v2 dice roll
				DP(i - 0, j - 1) += ( 91.0 / 216.0) * DP(i, j);
				DP(i - 1, j - 0) += (125.0 / 216.0) * DP(i, j);
			}
		}
		if (capital) {
			// 1v2 dice roll
			int j = 2;
			DP(i - 0, j - 1) += ( 91.0 / 216.0) * DP(i, j);
			DP(i - 1, j - 0) += (125.0 / 216.0) * DP(i, j);
		}
		{
			// 1v1 dice roll
			int j = 1;
			DP(i - 0, j - 1) += (15.0 / 36.0) * DP(i, j);
			DP(i - 1, j - 0) += (21.0 / 36.0) * DP(i, j);
		}
	}
	
	for (int i = 0; i <= n; ++i) {
		distrib.att_loss_probs[i] = DP(n - i, 0);
		distrib.att_victory_prob += DP(n - i, 0);
	}
	
	for (int i = 0; i <= m; ++i) {
		distrib.def_loss_probs[i] = DP(0, m - i);
		distrib.def_victory_prob += DP(0, m - i);
	}
	
	distrib.att_loss_probs[n] = distrib.def_victory_prob;
	distrib.def_loss_probs[m] = distrib.att_victory_prob;
	
	free(dp);
	#undef DP
	
	return distrib;
}

void destroy_battle_distrib(BattleDistrib *distrib) {
	free(distrib->att_loss_probs);
	free(distrib->def_loss_probs);
}

BalanceConfig *malloc_balance_config() {
	return (BalanceConfig *) malloc(sizeof(BalanceConfig));
}

static inline void normalize_sum(double *arr, double wanted_sum, int len) {
	if (wanted_sum <= 0.0) {
		for (int i = 0; i < len; ++i) {
			arr[i] *= 0.0;
		}
	}
	
	double sum = 0.0;
	for (int i = 0; i < len; ++i) {
		sum += arr[i];
	}
	
	if (sum <= 0) {
		for (int i = 0; i < len; ++i) {
			arr[i] = wanted_sum / len;
		}
	} else {
		double ratio = wanted_sum / sum;
		for (int i = 0; i < len; ++i) {
			arr[i] *= ratio;
		}
	}
}

static inline void apply_win_chance_cutoff(BattleDistrib *distrib, double cutoff) {
	const int n = distrib->att_troops;
	const int m = distrib->def_troops;
	
	if (distrib->att_victory_prob <= cutoff) {
		for (int i = 0; i < n; ++i) {
			distrib->att_loss_probs[i] = 0.0;
		}
		distrib->att_loss_probs[n] = 1.0;
		distrib->att_victory_prob = 0.0;
		distrib->def_victory_prob = 1.0;
		normalize_sum(distrib->def_loss_probs, 1.0, m);
	} else if (distrib->def_victory_prob <= cutoff) {
		for (int i = 0; i < m; ++i) {
			distrib->def_loss_probs[i] = 0.0;
		}
		distrib->def_loss_probs[m] = 1.0;
		distrib->def_victory_prob = 0.0;
		distrib->att_victory_prob = 1.0;
		normalize_sum(distrib->att_loss_probs, 1.0, n);
	}
}

static inline void apply_win_chance_power(BattleDistrib *distrib, double power) {
	const int n = distrib->att_troops;
	const int m = distrib->def_troops;
	
	distrib->att_victory_prob = pow(distrib->att_victory_prob, power);
	distrib->def_victory_prob = pow(distrib->def_victory_prob, power);
	
	double ratio = 1.0 / (distrib->att_victory_prob + distrib->def_victory_prob);
	distrib->att_victory_prob *= ratio;
	distrib->def_victory_prob *= ratio;
	
	// NB: normalization excludes the last prob, as that prob is the case the side *doesn't win*
	normalize_sum(distrib->att_loss_probs, distrib->att_victory_prob, n);
	normalize_sum(distrib->def_loss_probs, distrib->def_victory_prob, m);
}

static inline void apply_win_outcome_cutoff(BattleDistrib *distrib, double cutoff) {
	const int n = distrib->att_troops;
	const int m = distrib->def_troops;
	
	double *dist = malloc((n + m) * sizeof(double));
	
	for (int i = 0; i < n; ++i) {
		dist[i] = distrib->att_loss_probs[i];
	}
	for (int i = 0; i < m; ++i) {
		dist[n + m - 1 - i] = distrib->def_loss_probs[i];
	}
	
	{
		double total_cut = 0.0;
		for (int i = 0; i < n + m; ++i) {
			total_cut += dist[i];
			
			if (total_cut > cutoff) {
				dist[i] = total_cut - cutoff;
				break;
			} else {
				dist[i] = 0.0;
			}
		}
	}
	
	{
		double total_cut = 0.0;
		for (int i = n + m - 1; i >= 0; --i) {
			total_cut += dist[i];
			
			if (total_cut > cutoff) {
				dist[i] = total_cut - cutoff;
				break;
			} else {
				dist[i] = 0.0;
			}
		}
	}
	
	double att_victory_prob = 0.0;
	for (int i = 0; i < n; ++i) {
		distrib->att_loss_probs[i] = dist[i];
		att_victory_prob += dist[i];
	}
	
	double def_victory_prob = 0.0;
	for (int i = 0; i < m; ++i) {
		distrib->def_loss_probs[i] = dist[n + m - 1 - i];
		def_victory_prob += dist[n + m - 1 - i];
	}
	
	double ratio = 1.0 / (att_victory_prob + def_victory_prob);
	att_victory_prob *= ratio;
	def_victory_prob *= ratio;
	
	normalize_sum(distrib->att_loss_probs, att_victory_prob, n);
	normalize_sum(distrib->def_loss_probs, def_victory_prob, m);
	
	distrib->att_loss_probs[n] = distrib->def_victory_prob = def_victory_prob;
	distrib->def_loss_probs[m] = distrib->att_victory_prob = att_victory_prob;
	
	free(dist);
}

static inline void apply_win_outcome_power(BattleDistrib *distrib, double power) {
	const int n = distrib->att_troops;
	const int m = distrib->def_troops;
	
	for (int i = 0; i < n; ++i) {
		distrib->att_loss_probs[i] = pow(distrib->att_loss_probs[i], power);
	}
	
	for (int i = 0; i < m; ++i) {
		distrib->def_loss_probs[i] = pow(distrib->def_loss_probs[i], power);
	}
	
	normalize_sum(distrib->att_loss_probs, distrib->att_victory_prob, n);
	normalize_sum(distrib->def_loss_probs, distrib->def_victory_prob, m);
}

void battle_distrib_apply_balance(BattleDistrib *distrib, const BalanceConfig *config) {
	apply_win_chance_cutoff(distrib, config->win_chance_cutoff);
	apply_win_chance_power(distrib, config->win_chance_power);
	apply_win_outcome_cutoff(distrib, config->win_outcome_cutoff);
	apply_win_outcome_power(distrib, config->win_outcome_power);
}

/*
int main() {
	printf("10 vs 10 battle:\n");
	
	BattleDistrib distrib = calc_battle_distrib(&(BattleConfig) {
		.att_troops = 10,
		.def_troops = 10,
		.is_capital = 0,
	});
	
	printf("Attacker remaining:\n");
	for (int i = 0; i < distrib.att_troops; ++i) {
		printf("  P(%-3d left) = %.6f\n", i + 1, distrib.att_remaining_probs[i]);
	}
	printf("P(attacker wins) = %.6f\n", distrib.att_total_prob);
	
	printf("Defender remaining:\n");
	for (int i = 0; i < distrib.def_troops; ++i) {
		printf("  P(%-3d left) = %.6f\n", i + 1, distrib.def_remaining_probs[i]);
	}
	printf("P(defender wins) = %.6f\n", distrib.def_total_prob);
	
	destroy_battle_distrib(&distrib);
}*/

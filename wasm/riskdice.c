#include "riskdice.h"
#include "common.h"

static inline int true_random_exact(BattleDistrib *distrib, const BattleConfig *config);
static inline void normalize_sum(double *nums, double wanted_sum, int len);
static inline void apply_win_chance_cutoff(BattleDistrib *distrib, double cutoff);
static inline void apply_win_chance_power(BattleDistrib *distrib, double power);
static inline int apply_win_outcome_cutoff(BattleDistrib *distrib, double cutoff);
static inline void apply_win_outcome_power(BattleDistrib *distrib, double power);

int calc_battle_distrib(BattleDistrib *distrib, const BattleConfig *config) {
	return true_random_exact(distrib, config);
}

void destroy_battle_distrib(BattleDistrib *distrib) {
	free(distrib->attacker_damage_probs);
	free(distrib->defender_damage_probs);
}

int apply_balance(BattleDistrib *distrib, const BalanceConfig *config) {
	int r;
	
	apply_win_chance_cutoff(distrib, config->win_chance_cutoff);
	
	apply_win_chance_power(distrib, config->win_chance_power);
	
	// This one allocates memory, so it can fail
	r = apply_win_outcome_cutoff(distrib, config->win_outcome_cutoff);
	if (r < 0) {
		return r;
	}
	
	apply_win_outcome_power(distrib, config->win_outcome_power);
	
	return 0;
}

int true_random_exact(BattleDistrib *distrib, const BattleConfig *config) {
	const int natt = config->attacking_troops;
	const int ndef = config->defending_troops;
	const int capital = !!(config->flags & BC_CAPITAL_DEFENDING);
	const int zombies = !!(config->flags & BC_ZOMBIES_DEFENDING);
	
	double *raw_dp = calloc((natt + 1) * (ndef + 1), sizeof(double));
	#define DP(att_rem, def_rem) raw_dp[(att_rem) * (ndef + 1) + (def_rem)]
	
	if (!raw_dp) {
		return -1;
	}
	
	DP(natt, ndef) = 1.0;
	
	#define ROLL_3v3() do { \
		if (zombies) { \
			/* 3v3 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 3) += (17871.0 / 46656.0) * DP(att, def); \
			DP(att - 1, def - 2) += (12348.0 / 46656.0) * DP(att, def); \
			DP(att - 2, def - 1) += (10017.0 / 46656.0) * DP(att, def); \
			DP(att - 3, def - 0) += ( 6420.0 / 46656.0) * DP(att, def); \
		} else { \
			/* 3v3 dice roll, defender wins ties: */ \
			DP(att - 0, def - 3) += ( 6420.0 / 46656.0) * DP(att, def); \
			DP(att - 1, def - 2) += (10017.0 / 46656.0) * DP(att, def); \
			DP(att - 2, def - 1) += (12348.0 / 46656.0) * DP(att, def); \
			DP(att - 3, def - 0) += (17871.0 / 46656.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_3v2() do { \
		if (zombies) { \
			/* 3v2 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 2) += (4816.0 / 7776.0) * DP(att, def); \
			DP(att - 1, def - 1) += (1981.0 / 7776.0) * DP(att, def); \
			DP(att - 2, def - 0) += ( 979.0 / 7776.0) * DP(att, def); \
		} else { \
			/* 3v2 dice roll, defender wins ties: */ \
			DP(att - 0, def - 2) += (2890.0 / 7776.0) * DP(att, def); \
			DP(att - 1, def - 1) += (2611.0 / 7776.0) * DP(att, def); \
			DP(att - 2, def - 0) += (2275.0 / 7776.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_3v1() do { \
		if (zombies) { \
			/* 3v1 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 1) += (1071.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 0) += ( 225.0 / 1296.0) * DP(att, def); \
		} else { \
			/* 3v1 dice roll, defender wins ties: */ \
			DP(att - 0, def - 1) += (855.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 0) += (441.0 / 1296.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_2v3() do { \
		if (zombies) { \
			/* 2v3 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 2) += (2275.0 / 7776.0) * DP(att, def); \
			DP(att - 1, def - 1) += (2611.0 / 7776.0) * DP(att, def); \
			DP(att - 2, def - 0) += (2890.0 / 7776.0) * DP(att, def); \
		} else { \
			/* 2v3 dice roll, defender wins ties: */ \
			DP(att - 0, def - 2) += ( 979.0 / 7776.0) * DP(att, def); \
			DP(att - 1, def - 1) += (1981.0 / 7776.0) * DP(att, def); \
			DP(att - 2, def - 0) += (4816.0 / 7776.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_2v2() do { \
		if (zombies) { \
			/* 2v2 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 2) += (581.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 1) += (420.0 / 1296.0) * DP(att, def); \
			DP(att - 2, def - 0) += (295.0 / 1296.0) * DP(att, def); \
		} else { \
			/* 2v2 dice roll, defender wins ties: */ \
			DP(att - 0, def - 2) += (295.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 1) += (420.0 / 1296.0) * DP(att, def); \
			DP(att - 2, def - 0) += (581.0 / 1296.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_2v1() do { \
		if (zombies) { \
			/* 2v1 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 1) += (161.0 / 216.0) * DP(att, def); \
			DP(att - 1, def - 0) += ( 55.0 / 216.0) * DP(att, def); \
		} else { \
			/* 2v1 dice roll, defender wins ties: */ \
			DP(att - 0, def - 1) += (125.0 / 216.0) * DP(att, def); \
			DP(att - 1, def - 0) += ( 91.0 / 216.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_1v3() do { \
		if (zombies) { \
			/* 1v3 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 1) += (441.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 0) += (855.0 / 1296.0) * DP(att, def); \
		} else { \
			/* 1v3 dice roll, defender wins ties: */ \
			DP(att - 0, def - 1) += ( 225.0 / 1296.0) * DP(att, def); \
			DP(att - 1, def - 0) += (1071.0 / 1296.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_1v2() do { \
		if (zombies) { \
			/* 1v2 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 1) += ( 91.0 / 216.0) * DP(att, def); \
			DP(att - 1, def - 0) += (125.0 / 216.0) * DP(att, def); \
		} else { \
			/* 1v2 dice roll, defender wins ties: */ \
			DP(att - 0, def - 1) += ( 55.0 / 216.0) * DP(att, def); \
			DP(att - 1, def - 0) += (161.0 / 216.0) * DP(att, def); \
		} \
	} while (0)
	
	#define ROLL_1v1() do { \
		if (zombies) { \
			/* 1v1 dice roll, attacker wins ties: */ \
			DP(att - 0, def - 1) += (21.0 / 36.0) * DP(att, def); \
			DP(att - 1, def - 0) += (15.0 / 36.0) * DP(att, def); \
		} else { \
			/* 1v1 dice roll, defender wins ties: */ \
			DP(att - 0, def - 1) += (15.0 / 36.0) * DP(att, def); \
			DP(att - 1, def - 0) += (21.0 / 36.0) * DP(att, def); \
		} \
	} while (0)
	
	for (int att = natt; att >= 3; --att) {
		for (int def = ndef; def >= 2 + capital; --def) {
			if (capital) {
				ROLL_3v3();
			} else {
				ROLL_3v2();
			}
		}
		if (capital && ndef >= 2) {
			int def = 2;
			ROLL_3v2();
		}
		if (ndef >= 1) {
			int def = 1;
			ROLL_3v1();
		}
	}
	
	if (natt >= 2) {
		int att = 2;
		for (int def = ndef; def >= 2 + capital; --def) {
			if (capital) {
				ROLL_2v3();
			} else {
				ROLL_2v2();
			}
		}
		if (capital && ndef >= 2) {
			int def = 2;
			ROLL_2v2();
		}
		if (ndef >= 1) {
			int def = 1;
			ROLL_2v1();
		}
	}
	
	if (natt >= 1) {
		int att = 1;
		for (int def = ndef; def >= 2 + capital; --def) {
			if (capital) {
				ROLL_1v3();
			} else {
				ROLL_1v2();
			}
		}
		if (capital && ndef >= 2) {
			int def = 2;
			ROLL_1v2();
		}
		if (ndef >= 1) {
			int def = 1;
			ROLL_1v1();
		}
	}
	
	distrib->attacker_starting_troops = natt;
	distrib->defender_starting_troops = ndef;
	
	distrib->attacker_damage_probs = malloc((natt + 1) * sizeof(double));
	if (!distrib->attacker_damage_probs) {
		free(raw_dp);
		return -1;
	}
	
	distrib->defender_damage_probs = malloc((ndef + 1) * sizeof(double));
	if (!distrib->defender_damage_probs) {
		free(raw_dp);
		free(distrib->attacker_damage_probs);
		return -1;
	}
	
	distrib->defender_damage_probs[ndef] = 0.0;
	for (int att_damage = 0; att_damage < natt; ++att_damage) {
		double prob = DP(natt - att_damage, 0);
		distrib->attacker_damage_probs[att_damage] = prob;
		distrib->defender_damage_probs[ndef] += prob;
	}
	
	distrib->attacker_damage_probs[natt] = 0.0;
	for (int def_damage = 0; def_damage < ndef; ++def_damage) {
		double prob = DP(0, ndef - def_damage);
		distrib->defender_damage_probs[def_damage] = prob;
		distrib->attacker_damage_probs[natt] += prob;
	}
	
	#undef DP
	#undef ROLL_3v3
	#undef ROLL_3v2
	#undef ROLL_3v1
	#undef ROLL_2v3
	#undef ROLL_2v2
	#undef ROLL_2v1
	#undef ROLL_1v3
	#undef ROLL_1v2
	#undef ROLL_1v1
	free(raw_dp);
	
	return 0;
}

void normalize_sum(double *nums, double wanted_sum, int len) {
	if (wanted_sum <= 0.0) {
		for (int i = 0; i < len; ++i) {
			nums[i] *= 0.0;
		}
	}
	
	double sum = 0.0;
	for (int i = 0; i < len; ++i) {
		sum += nums[i];
	}
	
	if (sum <= 0.0) {
		for (int i = 0; i < len; ++i) {
			nums[i] = wanted_sum / len;
		}
	} else {
		double ratio = wanted_sum / sum;
		for (int i = 0; i < len; ++i) {
			nums[i] *= ratio;
		}
	}
}

void apply_win_chance_cutoff(BattleDistrib *distrib, double cutoff) {
	const int natt = distrib->attacker_starting_troops;
	const int ndef = distrib->defender_starting_troops;
	
	// If the defender victory probability is <= cutoff, make it 0:
	if (distrib->attacker_damage_probs[natt] <= cutoff) {
		distrib->attacker_damage_probs[natt] = 0.0;
		distrib->defender_damage_probs[ndef] = 1.0;
		for (int i = 0; i < ndef; ++i) {
			distrib->defender_damage_probs[i] = 0.0;
		}
		// Renormalize the probabilities to sum to 1 again
		normalize_sum(distrib->attacker_damage_probs, 1.0, natt);
	}
	
	// If the attacker victory probability is <= cutoff, make it 0:
	if (distrib->defender_damage_probs[ndef] <= cutoff) {
		distrib->defender_damage_probs[ndef] = 0.0;
		distrib->attacker_damage_probs[natt] = 1.0;
		for (int i = 0; i < natt; ++i) {
			distrib->attacker_damage_probs[i] = 0.0;
		}
		// Renormalize the probabilities to sum to 1 again
		normalize_sum(distrib->defender_damage_probs, 1.0, ndef);
	}
}

void apply_win_chance_power(BattleDistrib *distrib, double power) {
	const int natt = distrib->attacker_starting_troops;
	const int ndef = distrib->defender_starting_troops;
	
	// Raise both sides' victory probabilties to the power of `power`:
	distrib->attacker_damage_probs[natt] = pow(distrib->attacker_damage_probs[natt], power);
	distrib->defender_damage_probs[ndef] = pow(distrib->defender_damage_probs[ndef], power);
	
	// Renormalize both sides' victory probabilities for them to sum to 1:
	double ratio = 1.0 / (distrib->attacker_damage_probs[natt] + distrib->defender_damage_probs[ndef]);
	distrib->attacker_damage_probs[natt] *= ratio;
	distrib->defender_damage_probs[ndef] *= ratio;
	
	// And renormalize each side's damage probabilities for them to sum to the side's probability of victory:
	normalize_sum(distrib->attacker_damage_probs, distrib->defender_damage_probs[ndef], natt);
	normalize_sum(distrib->defender_damage_probs, distrib->attacker_damage_probs[natt], ndef);
}

int apply_win_outcome_cutoff(BattleDistrib *distrib, double cutoff) {
	const int natt = distrib->attacker_starting_troops;
	const int ndef = distrib->defender_starting_troops;
	
	// We concatenate both side's damage distributions into one:
	
	double *dist = malloc((natt + ndef) * sizeof(double));
	if (!dist) {
		return -1;
	}
	
	for (int i = 0; i < natt; ++i) {
		dist[i] = distrib->attacker_damage_probs[i];
	}
	
	for (int i = 0; i < ndef; ++i) {
		dist[natt + ndef - 1 - i] = distrib->defender_damage_probs[i];
	}
	
	// Then cut off the least likely outcomes from both ends of the distribution:
	
	{
		// Cut off `cutoff` total probability off the low end:
		double total_cut = 0.0;
		for (int i = 0; i < natt + ndef; ++i) {
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
		// Cut off `cutoff` total probability off the high end:
		double total_cut = 0.0;
		for (int i = natt + ndef - 1; i >= 0; --i) {
			total_cut += dist[i];
			
			if (total_cut > cutoff) {
				dist[i] = total_cut - cutoff;
				break;
			} else {
				dist[i] = 0.0;
			}
		}
	}
	
	// Put the changed probabilities back into `distrib`, recalculating victory probabiltiies as well:
	
	distrib->defender_damage_probs[ndef] = 0.0;
	for (int i = 0; i < natt; ++i) {
		double prob = dist[i];
		distrib->attacker_damage_probs[i] = prob;
		distrib->defender_damage_probs[ndef] += prob;
	}
	
	distrib->attacker_damage_probs[natt] = 0.0;
	for (int i = 0; i < ndef; ++i) {
		double prob = dist[natt + ndef - 1 - i];
		distrib->defender_damage_probs[i] = prob;
		distrib->attacker_damage_probs[natt] += prob;
	}
	
	// Renormalize both sides' victory probabilities for them to sum to 1:
	double ratio = 1.0 / (distrib->attacker_damage_probs[natt] + distrib->defender_damage_probs[ndef]);
	distrib->attacker_damage_probs[natt] *= ratio;
	distrib->defender_damage_probs[ndef] *= ratio;
	
	// And renormalize each side's damage probabilities for them to sum to the side's probability of victory:
	normalize_sum(distrib->attacker_damage_probs, distrib->defender_damage_probs[ndef], natt);
	normalize_sum(distrib->defender_damage_probs, distrib->attacker_damage_probs[natt], ndef);
	
	free(dist);
	
	return 0;
}

void apply_win_outcome_power(BattleDistrib *distrib, double power) {
	const int natt = distrib->attacker_starting_troops;
	const int ndef = distrib->defender_starting_troops;
	
	// Raise both sides' damage probabilties to the power of `power`:
	
	for (int i = 0; i < natt; ++i) {
		distrib->attacker_damage_probs[i] = pow(distrib->attacker_damage_probs[i], power);
	}
	
	for (int i = 0; i < ndef; ++i) {
		distrib->defender_damage_probs[i] = pow(distrib->defender_damage_probs[i], power);
	}
	
	// And renormalize each side's damage probabilities for them to sum to the side's probability of victory:
	normalize_sum(distrib->attacker_damage_probs, distrib->defender_damage_probs[ndef], natt);
	normalize_sum(distrib->defender_damage_probs, distrib->attacker_damage_probs[natt], ndef);
}

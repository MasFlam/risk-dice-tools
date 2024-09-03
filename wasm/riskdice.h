#pragma once

#include "common.h"

#define BC_CAPITAL_DEFENDING 0x01
#define BC_ZOMBIES_DEFENDING 0x02

typedef struct battle_config BattleConfig;
typedef struct battle_distrib BattleDistrib;
typedef struct balance_config BalanceConfig;

struct battle_config {
	int attacking_troops;
	int defending_troops;
	unsigned flags; // `BC_*` bitmask
};

struct battle_distrib {
	int attacker_starting_troops;
	int defender_starting_troops;
	double *attacker_damage_probs;
	double *defender_damage_probs;
};

struct balance_config {
	double win_chance_cutoff;
	double win_chance_power;
	double win_outcome_cutoff;
	double win_outcome_power;
};

// Returns 0 on success, -1 on failure. `distrib` should be destroyed only on success.
EXPORTED
int calc_battle_distrib(BattleDistrib *distrib, const BattleConfig *config);

// Deallocates the damage probability arrays in `distrib`.
EXPORTED
void destroy_battle_distrib(BattleDistrib *distrib);

// Returns 0 on success, -1 on failure. `distrib` will be altered even on failure.
EXPORTED
int apply_balance(BattleDistrib *distrib, const BalanceConfig *config);

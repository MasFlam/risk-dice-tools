#pragma once

#include <stdlib.h>

#define EXPORTED __attribute__((used))

typedef struct RollOutcome {
	int att_loss;
	int def_loss;
} RollOutcome;

typedef struct RollDistrib {
	int length;
	int counts_sum;
	RollOutcome *outcomes;
	int *counts;
} RollDistrib;

typedef struct BattleConfig {
	int att_troops;
	int def_troops;
	int is_capital;
	int is_zombie;
} BattleConfig;

typedef struct BattleDistrib {
	int att_troops;
	int def_troops;
	double *att_loss_probs; // att_loss_probs[0 <= i <= n] = P(attacker ends up losing `i` troops)
	double *def_loss_probs; // att_loss_probs[0 <= i <= m] = P(defender ends up losing `i` troops)
	double att_victory_prob;
	double def_victory_prob;
} BattleDistrib;

typedef struct BalanceConfig {
	double win_chance_cutoff;
	double win_chance_power;
	double win_outcome_cutoff;
	double win_outcome_power;
} BalanceConfig;

EXPORTED
RollOutcome calc_roll_outcome(int att_dice_count, int def_dice_count, int *att_dice, int *def_dice, int is_zombie);

[[nodiscard]]
EXPORTED
RollDistrib *malloc_roll_distrib();

[[nodiscard]]
EXPORTED
RollDistrib calc_roll_distrib(int att_dice_count, int def_dice_count, int is_zombie);

EXPORTED
void destroy_roll_distrib(RollDistrib *distrib);

[[nodiscard]]
EXPORTED
BattleDistrib *malloc_battle_distrib();

[[nodiscard]]
EXPORTED
BattleConfig *malloc_battle_config();

[[nodiscard]]
EXPORTED
BattleDistrib calc_battle_distrib(const BattleConfig *config);

EXPORTED
void destroy_battle_distrib(BattleDistrib *distrib);

[[nodiscard]]
EXPORTED
BalanceConfig *malloc_balance_config();

EXPORTED
void battle_distrib_apply_balance(BattleDistrib *distrib, const BalanceConfig *config);

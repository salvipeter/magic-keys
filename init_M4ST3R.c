#include <stdlib.h>

#include "magic-keys.h"

static int min(int a, int b) {
  return a < b ? a : b;
}

static int max(int a, int b) {
  return a > b ? a : b;
}

static void deinit(void *extra) {
  free((int *)extra);
}

/*
   A bit smarter than W8PRQR:
   - all dice are asleep -> use magic pond (when available)
   - if there are sleeping dice:
     - regenerate when possible
     - if we have no fake keys or in the first half of the board -> take furthest key
   - if we can reach the golden key -> take it
   - otherwise -> move as far as we can and roll again
*/
static Action move(const GameState *state, const int *dice, void *extra) {
  int *id = (int *)extra;
  if (dice[0] == 0 && dice[1] == 0 && dice[2] == 0) {
    if (state->position == 0)
      return TELEPORT | ROLL_AGAIN;
    if (state->fake_keys[*id] > 0)
      return MAGIC_POND | ROLL_AGAIN;
    return END_TURN;
  }
  int sleep = 0;
  for (int i = 0; i < 3; ++i)
    if (dice[i] == 0)
      sleep++;
  if (sleep > 0) {
    // Try to regenerate
    for (int i = 0; i < 3; ++i)
      if (dice[i] > 0 && state->position + dice[i] == 11)
        return dice[i] | ROLL_AGAIN;
    if (state->fake_keys[*id] == 0 || state->position < 11) // Try to take a key
      for (int i = 2; i >= 0; --i)
        if (dice[i] > 0 && state->keys[min(state->position + dice[i], 22)] == UNKNOWN)
          return dice[i] | TRY_KEY;
  }
  // Move forward except if at the end
  int best = max(dice[0], max(dice[1], dice[2]));
  if (state->position + best >= 22)
      return best | TRY_KEY;
  return best | ROLL_AGAIN;
}

void init_M4ST3R(Player *player, int player_index, int n_players) {
  player->extra = malloc(sizeof(int));                 /* only save own ID */
  *(int *)player->extra = player_index;
  player->deinit = deinit;
  player->move = move;
}

#include <stdlib.h>

#include "magic-keys.h"

/*
  Example implementation

  To avoid name clashes:
  - functions other than `init_W8PRQR` should be static
  - structs / enums / etc. should use the same Neptun ID suffix,
    e.g. SomeImportantData_W8PRQR
*/

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
  Always takes the largest step, and takes the first available key.
  Also uses fake keys when needed.
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
  int best = max(dice[0], max(dice[1], dice[2]));
  int pos = min(state->position + best, 22);
  if (pos == 22 || state->keys[pos] == UNKNOWN)
    return best | TRY_KEY;
  return best | ROLL_AGAIN;
}

void init_W8PRQR(Player *player, int player_index, int n_players) {
  player->extra = malloc(sizeof(int));                 /* only save own ID */
  *(int *)player->extra = player_index;
  player->deinit = deinit;
  player->move = move;
}

#include <stdlib.h>

#include "magic-keys.h"

/*
  Example implementation

  To avoid name clashes:
  - functions other than `init_XPL428` should be static
  - structs / enums / etc. should use the same Neptun ID suffix,
    e.g. SomeImportantData_XPL428
*/

static void deinit(void *extra) {
}

/* Always takes the smallest step, and tries to get the golden key. */
static Action move(const GameState *state, const int *dice, void *extra) {
  if (dice[0] == 0 && dice[1] == 0 && dice[2] == 0) {
    if (state->position == 0)
      return TELEPORT | ROLL_AGAIN;
    return END_TURN;
  }
  int minimal = 5;
  for (int i = 0; i < 3; ++i)
    if (dice[i] > 0 && dice[i] < minimal)
      minimal = dice[i];
  if (state->position + minimal >= 22)
    return minimal | TRY_KEY;
  return minimal | ROLL_AGAIN;
}

void init_XPL428(Player *player, int player_index, int n_players) {
  player->deinit = deinit;
  player->move = move;
}

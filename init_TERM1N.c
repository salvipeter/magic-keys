#include <stdio.h>

#include "magic-keys.h"

/* Interactive player implementation (better to use it with logging enabled) */

static void deinit(void *extra) {
}

/* Always takes the smallest step, and tries to get the golden key. */
static Action move(const GameState *state, const int *dice, void *extra) {
  printf("[8;1H");           /* position cursor at row 8, column 1 */
  for (int i = 0; i < 23; ++i) {
    printf(i == 1 || i == 6 || i == 11 || i == 12 || i == 17 || i == 22
           ? "|" : " ");
    switch (state->keys[i]) {
    case UNKNOWN:  printf("?"); break;
    case TRUE_KEY: printf("T"); break;
    case FAKE_KEY: printf("F"); break;
    default:       printf("-");
    }
  }
  printf("\n[2K");              /* clear a new row */
  for (int i = 0; i < state->position; ++i)
    printf("  ");
  printf(" ^\n\n");
  for (int i = 0; i < state->n_players; ++i)
    printf("[Player #%d] Gems: %2d, Keys: %2d\n",
           i + 1, state->gems[i], state->fake_keys[i]);
  printf("\n[2K");              /* clear a new row */
  printf("[%d,%d,%d]? ", dice[0], dice[1], dice[2]);
  char line[80];
  scanf("%s", line);
  printf("------------------------------------------------------------\n");
  printf("[0J");              /* clear everything below */
  if (line[0] == 'e')
    return END_TURN;
  if (line[0] == 'p')
    return MAGIC_POND | ROLL_AGAIN;
  if (line[0] == 't')
    return TELEPORT | ROLL_AGAIN;
  int movement = line[0] - '0';
  int decision = line[1] == 'k' ? TRY_KEY : ROLL_AGAIN;
  return movement | decision | (line[2] == 'p' ? MAGIC_POND : 0);
}

void init_TERM1N(Player *player, int player_index, int n_players) {
  printf("c");                /* clear screen */
  printf("Possible actions:\n");
  printf("- e: end turn\n");
  printf("- p: use magic pond and roll again\n");
  printf("- t: teleport and roll again\n");
  printf("- [1234][kr]p?: move, use key or roll, and possibly use pond\n");
  printf("============================================================\n");
  printf("[15;1H");           /* position cursor at row 16, column 1 */
  printf("------------------------------------------------------------\n");
  player->deinit = deinit;
  player->move = move;
}

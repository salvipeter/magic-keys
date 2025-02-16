#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "magic-keys.h"

/* Set this to `true` to see all movements in the game */
#define DEBUG_LOG false

char error_msg[255];
int winning_gems[5] = { 0, 0, 15, 13, 11 };
int MOVEMENT_MASK = 0x00F;
int DECISION_MASK = 0x0F0;
int OTHER_MASK    = 0xF00;

#define ERROR(str) {                                            \
    strcpy(error_msg, str);                                     \
    winner = -(i + 1);                                          \
    break;                                                      \
  }

#define LOG(...) if (DEBUG_LOG) printf(__VA_ARGS__)

void generate_keys(bool *keys, int from) {
  int i = from;
  keys[i++] = true; keys[i++] = true; keys[i++] = true;
  keys[i++] = false; keys[i] = false;
  /* Shuffle */
  for (i = 0; i < 4; ++i) {
    int j = i + rand() % (5 - i);
    bool tmp = keys[from+i];
    keys[from+i] = keys[from+j];
    keys[from+j] = tmp;
  }
}

bool check_combination(Action action) {
  if ((action & OTHER_MASK) == TELEPORT)
    return action == (TELEPORT | ROLL_AGAIN);
  if ((action & OTHER_MASK) == END_TURN)
    return action == END_TURN;
  if ((action & OTHER_MASK) == MAGIC_POND) {
    if ((action & MOVEMENT_MASK) == 0)
      return action == (MAGIC_POND | ROLL_AGAIN);
    action -= MAGIC_POND;
  }
  int move = action & MOVEMENT_MASK;
  int decision = action & DECISION_MASK;
  return action == (move | decision) &&
    (move == MOVE_1 || move == MOVE_2 || move == MOVE_3 || move == MOVE_4) &&
    (decision == TRY_KEY || decision == ROLL_AGAIN);
}

bool check_dice(const int *dice, int steps) {
  return dice[0] == steps || dice[1] == steps || dice[2] == steps;
}

bool all_asleep(const int *dice) {
  return dice[0] == 0 && dice[1] == 0 && dice[2] == 0;
}

bool some_asleep(const int *dice) {
  return dice[0] * dice[1] * dice[2] == 0;
}

/* Throw the non-sleeping dice */
void throw(int *dice) {
  for (int i = 0; i < 3; ++i)
    if (dice[i])
      dice[i] = rand() % 6 % 5;
}

void state_init(GameState *state, int n_players) {
  state->n_players = n_players;
  state->gems = (int *)malloc(n_players * sizeof(int));
  state->fake_keys = (int *)malloc(n_players * sizeof(int));
  for (int i = 0; i < n_players; ++i) {
    state->gems[i] = 0;
    state->fake_keys[i] = 0;
  }
  state->position = 0;
  for (int i = 0; i < 22; ++i)
    state->keys[i] = UNKNOWN;
  state->keys[0] = state->keys[11] = NOT_KEY;
  state->keys[22] = TRUE_KEY;
}

GameState *state_clone(const GameState *state) {
  GameState *clone = (GameState *)malloc(sizeof(GameState));
  clone->n_players = state->n_players;
  clone->gems = (int *)malloc(clone->n_players * sizeof(int));
  clone->fake_keys = (int *)malloc(clone->n_players * sizeof(int));
  for (int i = 0; i < clone->n_players; ++i) {
    clone->gems[i] = state->gems[i];
    clone->fake_keys[i] = state->fake_keys[i];
  }
  clone->position = state->position;
  for (int i = 0; i < 23; ++i)
    clone->keys[i] = state->keys[i];
  return clone;
}

void state_cleanup(GameState *state) {
  free(state->fake_keys);
  free(state->gems);
  free(state);
}

/*
  Return value:
    n > 0 -> winner is Player n
    n < 0 -> Player (-n) has made a mistake
  (Players are indexed starting from 1.)
*/
int play(int n_players, Player *players, int starting_player) {
  /* Initialize the game state */
  GameState *state = (GameState *)malloc(sizeof(GameState));
  state_init(state, n_players);
  bool *key_type = (bool *)malloc(23 * sizeof(bool));
  generate_keys(key_type, 1);
  generate_keys(key_type, 6);
  generate_keys(key_type, 12);
  generate_keys(key_type, 17);
  key_type[22] = true;

  /* Game loop */
  int winner = 0;
  while (!winner) {
    for (int i = starting_player; !winner && i < n_players; ++i) {
      starting_player = 0;
      int dice[3] = { 1, 1, 1 }; /* anything but 0 */
      bool end_turn = false;
      while (!end_turn) {
        throw(dice);
        LOG("Player #%d throws [%d,%d,%d]\n", i + 1, dice[0], dice[1], dice[2]);

        /* Clone game state & dice in case of malicious agents */
        GameState *clone = state_clone(state);
        int dice_clone[3] = { dice[0], dice[1], dice[2] };
        int action = players[i].move(clone, dice_clone, players[i].extra);
        state_cleanup(clone);

        if (!check_combination(action))
          ERROR("invalid combination of actions");

        if ((action & OTHER_MASK) == END_TURN) {
          if (!all_asleep(dice))
            ERROR("tried to end turn with active dice");
          if (state->position == 0)
            ERROR("tried to end turn on starting position");
          LOG("Player #%d ends the turn\n", i + 1);
          end_turn = true;
          continue;
        }
        if ((action & OTHER_MASK) == TELEPORT) {
          if (state->position != 0)
            ERROR("tried to teleport when not on starting position");
          if (!all_asleep(dice))
            ERROR("tried to teleport with active dice");
          LOG("Player #%d teleports\n", i + 1);
          state->position = 11;
          continue;
        }

        /* From here on we know that there should be normal movement */
        int steps = action & MOVEMENT_MASK;
        if (steps > 0) {
          if (!check_dice(dice, steps))
            ERROR("no dice with that number of steps");
          state->position += steps;
          if (state->position > 22)
            state->position = 22;
          LOG("Player #%d moves to position %d\n", i + 1, state->position);
          if (state->position == 11 && some_asleep(dice)) {
            dice[0] = dice[1] = dice[2] = 1; /* wake all dice */
            LOG("Player #%d's dice are all awake now\n", i + 1);
          }
        }

        if ((action & OTHER_MASK) == MAGIC_POND) {
          if (state->fake_keys[i] == 0)
            ERROR("tried to use pond w/o fake keys");
          if (!some_asleep(dice))
            ERROR("tried to use pond w/o sleeping dice");
          state->fake_keys[i]--;
          LOG("Player #%d throws a fake key into the magic pond (now has %d)\n",
              i + 1, state->fake_keys[i]);
          dice[0] = dice[1] = dice[2] = 1; /* wake all dice */
          continue;
        }

        if ((action & DECISION_MASK) == ROLL_AGAIN) {
          if (state->position == 22)
            ERROR("tried to go beyond the golden key");
          continue;
        }

        /* Try the key */
        if (state->keys[state->position] == NOT_KEY)
          ERROR("tried to use key at a non-key position");
        if (state->position < 22 &&
            (state->keys[state->position] == TRUE_KEY ||
             state->keys[state->position] == FAKE_KEY))
          ERROR("tried to use already used key");
        if (key_type[state->position]) {
          /* True key */
          state->keys[state->position] = TRUE_KEY;
          int new_gems = 0;
          if (state->position < 11)
            new_gems = 2;
          else if (state->position < 17)
            new_gems = 3;
          else if (state->position < 22)
            new_gems = 4;
          else
            new_gems = 5;
          state->gems[i] += new_gems;
          LOG("Player #%d tries a true key, gets %d gems (now has %d)\n",
              i + 1, new_gems, state->gems[i]);
          if (state->gems[i] >= winning_gems[n_players])
            winner = i + 1;
        } else {
          /* Fake key */
          state->keys[state->position] = FAKE_KEY;
          state->fake_keys[i]++;
          LOG("Player #%d tries a fake key (now has %d)\n",
              i + 1, state->fake_keys[i]);
        }
        state->position = 0;
        end_turn = true;
      }
    }
  }

  /* Deinitialize the game state & the players */
  for (int i = 0; i < state->n_players; ++i)
    players[i].deinit(players[i].extra);
  state_cleanup(state);
  free(key_type);

  if (winner < 0) {
    LOG("Player #%d makes a mistake\n", -winner);
  } else {
    LOG("Player #%d wins the game\n", winner);
  }

  return winner;
}

void init_NEPTUN1(Player *, int, int);
void init_NEPTUN2(Player *, int, int);

int main(int argc, char **argv) {
  int rounds = 100;
  if (argc > 1)
    rounds = atoi(argv[1]);
  int wins[3] = { 0, 0, 0 };
  for (int i = 0; i < rounds; ++i) {
    Player players[2];
    init_NEPTUN1(&players[0], 0, 2);
    init_NEPTUN2(&players[1], 1, 2);
    int winner = play(2, players, i % 2); /* alternate starting player */
    if (winner < 0) {
      fprintf(stderr, "Player #%d error: %s\n", -winner, error_msg);
      return winner;
    }
    wins[winner]++;
  }
  printf("Player 1: %d wins - Player 2: %d wins\n", wins[1], wins[2]);
}

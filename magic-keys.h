/*
  API for Magic Keys Bot Competition
  https://boardgamegeek.com/boardgame/368899/magic-keys

  The submitted program should export a single function of the following type:

    void init_W8PRQR(Player *player, int player_index, int n_players);

  Its name should be of the format init_ followed by the Neptun ID.
  The filename should also be the same (e.g. "init_W8PRQR.c").
*/

#pragma once

/*
  The board is represented by positions, numbered from 0 to 22:
   0           is the starting position,
   1 - 5       are the purple squares (worth 2 gems)
   6 -10       are the blue   squares (worth 2 gems)
   11          is the recovery space
   12-16       are the green  squares (worth 3 gems)
   17-21       are the yellow squares (worth 4 gems)
   22          is the  golden square  (worth 5 gems)
*/

typedef enum {
  UNKNOWN,                      /* the key has not been used yet */
  TRUE_KEY,                     /* golden or already tried true key */
  FAKE_KEY,                     /* already tried fake key */
  NOT_KEY                       /* not a normal key position */
} KeyState;

typedef struct {
  int n_players;                /* # of players */
  int *gems;                    /* # of gems (for each player) */
  int *fake_keys;               /* # of _unused_ fake keys (for each player) */
  int position;                 /* current position of the pawn */
  KeyState keys[23];            /* positions 0 and 11 are NOT_KEY */
} GameState;

/*
  Some actions can be combined, e.g. MOVE_2 | TRY_KEY.
  The following combinations are possible (arrows showing order of execution):
  - MOVE_X -> [MAGIC_POND] -> TRY_KEY / ROLL_AGAIN
  - MAGIC_POND / TELEPORT -> ROLL_AGAIN
  - END_TURN
*/
typedef enum {
  /* Movements */
  MOVE_1     = 0x1,             /* move forward 1 space */
  MOVE_2     = 0x2,             /* move forward 2 spaces */
  MOVE_3     = 0x3,             /* move forward 3 spaces */
  MOVE_4     = 0x4,             /* move forward 4 spaces */
  /* Decision */
  TRY_KEY    = 0x10,            /* try the key at the current spot */
  ROLL_AGAIN = 0x20,            /* roll again the remaining dice */
  /* Other */
  TELEPORT   = 0x100,           /* teleport to square 11 */
  MAGIC_POND = 0x200,           /* throw a fake key in the magic pond */
  END_TURN   = 0x400            /* end turn now, leaving the pawn there */
} Action;

typedef struct {
  void *extra;                  /* for holding agent-specific information */

  void (*deinit)(void *extra);  /* releases extra resources */

  /* The dice are represented by an array of 3 integers (0 = sleeping) */
  Action (*move)(const GameState *state, int *dice, void *extra);
} Player;

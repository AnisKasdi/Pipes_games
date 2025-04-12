#ifndef GAME_MOVES_H
#define GAME_MOVES_H

#include "game.h"

typedef struct {
  uint row;
  uint col;
  direction old_dir;
  int nb_turns;
} move_t;

#endif  // GAME_MOVES_H
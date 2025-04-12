/**
 * @file game_private.c
 * @copyright University of Bordeaux. All rights reserved, 2024.
 **/

#include "game_private.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_ext.h"
#include "game_struct.h"
#include "queue.h"

/* ************************************************************************** */
/*                             STACK ROUTINES                                 */
/* ************************************************************************** */

void _stack_push_move(queue* q, move m) {
  assert(q);
  move* pm = malloc(sizeof(move));
  assert(pm);
  *pm = m;
  queue_push_head(q, pm);
}

/* ************************************************************************** */

move _stack_pop_move(queue* q) {
  assert(q);
  move* pm = queue_pop_head(q);
  assert(pm);
  move m = *pm;
  free(pm);
  return m;
}

/* ************************************************************************** */

bool _stack_is_empty(queue* q) {
  assert(q);
  return queue_is_empty(q);
}

/* ************************************************************************** */

void _stack_clear(queue* q) {
  assert(q);
  queue_clear_full(q, free);
  assert(queue_is_empty(q));
}

/* ************************************************************************** */
/*                                  MISC                                      */
/* ************************************************************************** */

char* square2str[NB_SHAPES][NB_DIRS] = {
    {" ", " ", " ", " "},  // empty
    {"^", ">", "v", "<"},  // endpoint
    {"|", "-", "|", "-"},  // segment
    {"└", "┌", "┐", "┘"},  // corner
    {"┴", "├", "┬", "┤"},  // tee
    {"+", "+", "+", "+"},  // cross
};

char* _square2str(shape s, direction d) {
  assert(s < NB_SHAPES);
  assert(d < NB_DIRS);
  return square2str[s][d];
}

/* ************************************************************************** */
/**
 * @file game_private.h
 * @brief Private Game Functions.
 * @copyright University of Bordeaux. All rights reserved, 2024.
 **/

#ifndef __GAME_PRIVATE_H__
#define __GAME_PRIVATE_H__

#include <stdbool.h>

#include "game.h"
#include "game_struct.h"
#include "queue.h"

/* ************************************************************************** */
/*                             DATA TYPES                                     */
/* ************************************************************************** */

/**
 * @brief Move structure.
 * @details This structure is used to save the game history.
 */
struct move_s {
  uint i, j;      // piece position
  direction old;  // old piece orientation
  direction new;  // new piece orientation
};

typedef struct move_s move;

/* ************************************************************************** */
/*                                MACRO                                       */
/* ************************************************************************** */

#define MAX(x, y) ((x > (y)) ? (x) : (y))

/* ************************************************************************** */
/*                             STACK ROUTINES                                 */
/* ************************************************************************** */

/** push a move in the stack */
void _stack_push_move(queue* q, move m);

/** pop a move from the stack */
move _stack_pop_move(queue* q);

/** test if the stack is empty */
bool _stack_is_empty(queue* q);

/** clear all the stack */
void _stack_clear(queue* q);

/* ************************************************************************** */
/*                                MISC                                        */
/* ************************************************************************** */

/** convert a square into its string representation
 * @details a single utf8 wide char represented by a string
 */
char* _square2str(shape s, direction d);

#endif  // __GAME_PRIVATE_H__

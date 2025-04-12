#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "game_aux.h"
#include "game_private.h"
#include "game_tools.h"

int main(int argc, char *argv[]) {
  game g;
  if (argc > 1) {
    g = game_load(argv[1]);
  } else {  // Fixed logic: use else instead of separate if
    g = game_default();
  }

  while (!game_won(g)) {
    game_print(g);
    char c;
    printf("? [h for help]\n");

    // Check character input
    if (scanf(" %c", &c) != 1) {
      fprintf(stderr, "Error: Failed to read character.\n");
      game_delete(g);
      return EXIT_FAILURE;
    }

    if (c == 'h') {
      printf("action: help\n");
      printf("press 'c <i> <j>' to rotate piece clockwise in square (i,j)\n");
      printf(
          "press 'a <i> <j>' to rotate piece anti-clockwise in square (i,j)\n");
      printf("press 'r' to shuffle game\n");
      printf("press 'q' to quit\n");
    }
    if (c == 'r') {
      printf("action: shuffle\n");
      game_shuffle_orientation(g);
    }
    if (c == 'q') {
      printf("action: restart\n");
      game_delete(g);
      printf("SHAME\n");
      return EXIT_SUCCESS;
    }
    if (c == 'c' || c == 'a') {
      int x, y;

      // Check coordinate input
      if (scanf(" %d %d", &x, &y) != 2) {
        fprintf(stderr, "Error: Failed to read coordinates.\n");
        game_delete(g);
        return EXIT_FAILURE;
      }

      if (c == 'c') {
        game_play_move(g, x, y, 1);
        printf("action: play move 'c' into square (%d,%d)\n", x, y);
      } else {
        game_play_move(g, x, y, -1);
        printf("action: play move 'a' into square (%d,%d)\n", x, y);
      }
    }
    if (c == 's') {
      char filename[256];
      if (scanf(" %255s", filename) == 1) {
        game_save(g, filename);
        printf("Game saved to %s\n", filename);
      } else {
        printf("Error: No filename provided.\n");
      }
    }
  }

  game_print(g);
  printf("CONGRATULATIONS\n");
  game_delete(g);
  return EXIT_SUCCESS;
}
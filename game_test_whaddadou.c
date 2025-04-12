#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_moves.h"
#include "game_struct.h"

int test_dummy() { return EXIT_SUCCESS; }

bool test_game_print() {
  game g = game_default();
  game_print(g);
  game_delete(g);
  return true;
}

bool test_game_shuffle_orientation() {
  game g1 = game_default();
  game g2 = game_copy(g1);

  if (g1 == NULL || g2 == NULL) {
    fprintf(stderr,
            "error, either game_default() or game_copy() failed to "
            "create a game ! \n");
    return false;
  }
  game_shuffle_orientation(g1);

  if (game_equal(g1, g2, false)) {
    fprintf(stderr, "game_orientation not set !!\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }
  printf("the function is correct !! \n");
  game_delete(g1);
  game_delete(g2);
  return true;
}

bool test_game_won() {
  game g = game_new_empty_ext(5, 5, true);
  if (!game_won(g)) {
    game_delete(g);
    return false;
  }

  game def = game_default();
  game sol = game_default_solution();
  if (game_won(def) || !game_won(sol)) {
    game_delete(def);
    game_delete(sol);
    game_delete(g);
    return false;
  }
  game_delete(def);
  game_delete(sol);

  game_set_piece_shape(g, 2, 2, CORNER);
  if (game_won(g)) {
    game_delete(g);
    return false;
  }

  game_set_piece_shape(g, 2, 3, CORNER);
  game_set_piece_orientation(g, 2, 3, WEST);
  game_set_piece_shape(g, 1, 2, CORNER);
  game_set_piece_orientation(g, 1, 2, EAST);
  game_set_piece_shape(g, 1, 3, CORNER);
  game_set_piece_orientation(g, 1, 3, SOUTH);

  if (!game_won(g)) {
    game_delete(g);
    return false;
  }

  game_set_piece_shape(g, 1, 3, TEE);
  if (game_won(g)) {
    game_delete(g);
    return false;
  }

  game_set_piece_shape(g, 1, 3, CORNER);
  game_set_piece_shape(g, 3, 0, SEGMENT);
  game_set_piece_shape(g, 2, 0, ENDPOINT);

  if (game_won(g)) {
    game_delete(g);
    return false;
  }

  game_set_piece_shape(g, 4, 0, ENDPOINT);
  game_set_piece_orientation(g, 2, 0, SOUTH);

  if (game_won(g)) {
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_game_reset_orientation() {
  game g = game_default();
  if (g == NULL) {
    fprintf(stderr, "function game_default does not set a game default!!");
    return false;
  }
  game_reset_orientation(g);
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      if (game_get_piece_orientation(g, i, j) != NORTH) {
        fprintf(stderr,
                "function does not reset all orientations to the north");
        game_delete(g);
        return false;
      }
    }
  }
  game g_solution = game_default_solution();
  game_reset_orientation(g_solution);
  if (game_won(g_solution)) {
    fprintf(
        stderr,
        "game g should not be a winning game if we reset his orientation !!\n");
    game_delete(g);
    game_delete(g_solution);
    return false;
  }
  game_delete(g);
  game_delete(g_solution);
  return true;
}

bool test_game_get_piece_orientation() {
  game g = game_default();

  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_orientation(g, i, j, NORTH);
      if (game_get_piece_orientation(g, i, j) != NORTH) {
        fprintf(stderr,
                "this piece orientation is supposed to be oriented to "
                "the NORTH \n");
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_orientation(g, i, j, EAST);
      if (game_get_piece_orientation(g, i, j) != EAST) {
        fprintf(
            stderr,
            "this piece orientation is supposed to be oriented to the EAST \n");
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_orientation(g, i, j, SOUTH);
      if (game_get_piece_orientation(g, i, j) != SOUTH) {
        fprintf(stderr,
                "this piece orientation is supposed to be oriented to "
                "the SOUTH \n");
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_orientation(g, i, j, WEST);
      if (game_get_piece_orientation(g, i, j) != WEST) {
        fprintf(
            stderr,
            "this piece orientation is supposed to be oriented to the WEST \n");
        game_delete(g);
        return false;
      }
    }
  }
  game_delete(g);
  return true;
}

bool test_game_get_piece_shape() {
  game g = game_default();

  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, TEE);
      if (game_get_piece_shape(g, i, j) != TEE) {
        fprintf(stderr, "the piece (%d,%d) is supposed to  BE A TEE  \n", i, j);
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, CORNER);
      if (game_get_piece_shape(g, i, j) != CORNER) {
        fprintf(stderr, "the piece (%d,%d) is supposed to  BE A CORNER  \n", i,
                j);
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, SEGMENT);
      if (game_get_piece_shape(g, i, j) != SEGMENT) {
        fprintf(stderr, "the piece (%d,%d) is supposed to  BE A SEGMENT  \n", i,
                j);
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, ENDPOINT);
      if (game_get_piece_shape(g, i, j) != ENDPOINT) {
        fprintf(stderr, "the piece (%d,%d) is supposed to  BE A ENDPOINT  \n",
                i, j);
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, CROSS);
      if (game_get_piece_shape(g, i, j) != CROSS) {
        fprintf(stderr, "the piece (%d,%d) is supposed to  BE A CROSS  \n", i,
                j);
        game_delete(g);
        return false;
      }
    }
  }
  for (int i = 0; i < game_nb_rows(g); i++) {
    for (int j = 0; j < game_nb_cols(g); j++) {
      game_set_piece_shape(g, i, j, EMPTY);
      if (game_get_piece_shape(g, i, j) != EMPTY) {
        fprintf(stderr,
                "the piece (%d,%d) is supposed to  BE AN EMPTY SHAPE   \n", i,
                j);
        game_delete(g);
        return false;
      }
    }
  }
  game_delete(g);
  return true;
}

bool test_game_play_move() {
  shape shapes[25] = {EMPTY};
  direction orientations[25] = {NORTH};
  game g = game_new_empty_ext(5, 5, true);
  if (!g) {
    fprintf(stderr, "Erreur: Échec de création du jeu\n");
    return false;
  }

  // Test rotation simple
  game_set_piece_shape(g, 1, 1, SEGMENT);
  direction initial_dir = game_get_piece_orientation(g, 1, 1);
  fprintf(stderr, "Direction initiale: %d\n", initial_dir);

  // Jouer un coup
  game_play_move(g, 1, 1, 1);
  direction new_dir = game_get_piece_orientation(g, 1, 1);
  fprintf(stderr, "Nouvelle direction: %d\n", new_dir);

  // Vérifier que la direction a changé
  if (initial_dir == new_dir) {
    fprintf(stderr, "Erreur: La direction n'a pas changé après le mouvement\n");
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

int main(int argc, char *argv[]) {
  // start test
  fprintf(stderr, "=> Start test \"%s\"\n", argv[1]);
  bool ok = false;
  if (strcmp("dummy", argv[1]) == 0) {
    return test_dummy();
  } else if (strcmp("game_print", argv[1]) == 0) {
    ok = test_game_print();
  } else if (strcmp("game_shuffle_orientation", argv[1]) == 0) {
    ok = test_game_shuffle_orientation();
  } else if (strcmp("game_won", argv[1]) == 0) {
    ok = test_game_won();
  } else if (strcmp("game_reset_orientation", argv[1]) == 0) {
    ok = test_game_reset_orientation();
  } else if (strcmp("game_get_piece_orientation", argv[1]) == 0) {
    ok = test_game_get_piece_orientation();
  } else if (strcmp("game_get_piece_shape", argv[1]) == 0) {
    ok = test_game_get_piece_shape();
  } else if (strcmp("game_play_move", argv[1]) == 0) {
    ok = test_game_play_move();
  } else {
    fprintf(stderr, "Error: test \"%s\" not found!\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  // print test result
  if (ok) {
    fprintf(stderr, "Test \"%s\" finished: SUCCESS\n", argv[1]);
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Test \"%s\" finished: FAILURE\n", argv[1]);
    return EXIT_FAILURE;
  }
}

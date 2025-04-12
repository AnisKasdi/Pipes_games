#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_moves.h"
#include "game_struct.h"

bool check_game_ext(game g, uint rows, uint cols, shape *shapes,
                    bool wrapping) {
  if (g == NULL) return false;
  if (game_nb_rows(g) != rows || game_nb_cols(g) != cols) return false;
  if (game_is_wrapping(g) != wrapping) return false;
  if (shapes != NULL) {
    for (uint i = 0; i < rows; i++) {
      for (uint j = 0; j < cols; j++) {
        if (game_get_piece_shape(g, i, j) != shapes[i * cols + j]) return false;
      }
    }
  }
  return true;
}

int test_equal_ext(void) {
  shape shapes[30] = {CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS};
  direction orientations[30] = {
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH};

  game g1 = game_new_ext(3, 10, shapes, orientations, false);
  game g2 = game_new_ext(3, 10, shapes, orientations, false);
  game g3 = game_new_ext(3, 10, shapes, orientations, true);

  // same game
  bool test1 = (game_equal(g1, g2, false) == true);

  // set a single different piece
  game_set_piece_shape(g2, 2, 9, SEGMENT);
  bool test2 = (game_equal(g1, g2, false) == false);

  // different options
  bool test3 = (game_equal(g1, g3, false) == false);

  game_delete(g1);
  game_delete(g2);
  game_delete(g3);

  if (test1 && test2 && test3) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

int test_copy_ext(void) {
  // game empty 3x5w
  game g1 = game_new_empty_ext(3, 5, true);
  game g2 = game_copy(g1);
  bool test0 = check_game_ext(g2, 3, 5, NULL, true);
  game_delete(g1);
  game_delete(g2);

  // game 3x10
  shape shapes[30] = {CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS, CROSS,
                      CROSS, CROSS, CROSS, CROSS, CROSS, CROSS};
  direction orientations[30] = {
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH,
      NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH, NORTH};

  game g3 = game_new_ext(3, 10, shapes, orientations, false);
  game g4 = game_copy(g3);
  bool test1 = check_game_ext(g4, 3, 10, shapes, false);
  game_delete(g3);
  game_delete(g4);

  if (test0 && test1) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

int test_undo_one(void) {
  game g = game_new_empty_ext(3, 3, false);
  game_set_piece_shape(g, 0, 0, CROSS);
  game_set_piece_shape(g, 1, 1, CROSS);
  game_set_piece_shape(g, 2, 2, CROSS);

  game_play_move(g, 0, 0, 1);
  game_play_move(g, 1, 1, 1);
  game_play_move(g, 2, 2, 1);

  bool test0 = (game_get_piece_orientation(g, 2, 2) == 1);
  game_undo(g);
  bool test1 = (game_get_piece_orientation(g, 2, 2) == 0);
  game_redo(g);
  bool test2 = (game_get_piece_orientation(g, 2, 2) == 1);

  game_delete(g);

  if (test0 && test1 && test2) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

int test_undo_redo_all(void) {
  // undo & redo all moves
  shape shapes[9] = {CROSS, CROSS, CROSS, CROSS, CROSS,
                     CROSS, CROSS, CROSS, CROSS};
  direction orientations[9] = {NORTH, NORTH, NORTH, NORTH, NORTH,
                               NORTH, NORTH, NORTH, NORTH};
  game g1 = game_new_ext(3, 3, shapes, orientations, false);

  game_play_move(g1, 0, 0, 1);
  game_play_move(g1, 1, 1, 1);
  game_play_move(g1, 2, 2, 1);
  game_play_move(g1, 0, 2, 1);
  game_play_move(g1, 1, 2, 1);
  game_play_move(g1, 2, 1, 1);
  game_play_move(g1, 2, 0, 1);
  game_play_move(g1, 1, 0, 1);
  game_play_move(g1, 0, 1, 1);

  bool test1 = check_game_ext(g1, 3, 3, shapes, false);

  for (int k = 0; k < 9; k++) game_undo(g1);
  bool test2 = check_game_ext(g1, 3, 3, shapes, false);

  for (int k = 0; k < 9; k++) game_redo(g1);
  bool test3 = check_game_ext(g1, 3, 3, shapes, false);

  game_delete(g1);

  if (test1 && test2 && test3) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

int test_undo_redo_some(void) {
  // undo & redo some moves
  shape shapes[9] = {CROSS, CROSS, CROSS, CROSS, CROSS,
                     CROSS, CROSS, CROSS, CROSS};
  direction orientations[9] = {NORTH, NORTH, NORTH, NORTH, NORTH,
                               NORTH, NORTH, NORTH, NORTH};
  game g0 = game_new_ext(3, 3, shapes, orientations, false);

  game_play_move(g0, 0, 0, 1);
  game_play_move(g0, 1, 1, 1);
  game_play_move(g0, 2, 2, 1);
  game_play_move(g0, 0, 2, 1);
  game_play_move(g0, 1, 2, 1);
  game_play_move(g0, 2, 1, 1);
  game_play_move(g0, 2, 0, 1);
  game_play_move(g0, 1, 0, 1);
  game_play_move(g0, 0, 1, 1);  // this is a bad move!
  game_undo(g0);                // undo (0,1) => cancel bad move
  game_undo(g0);                // undo (1,0) => cancel good move
  game_redo(g0);                // redo (1,0) => redo good move
  game_play_move(g0, 0, 1, 1);
  game_play_move(g0, 2, 1, 1);

  bool test0 = check_game_ext(g0, 3, 3, shapes, false);
  game_delete(g0);

  if (test0) return EXIT_SUCCESS;
  return EXIT_FAILURE;
}

bool test_game_new_ext(void) {
  // Créer un jeu vide
  game gm = game_new_empty();
  uint nb_rows = game_nb_rows(gm);  // Nombre de lignes du jeu
  uint nb_cols = game_nb_cols(gm);  // Nombre de colonnes du jeu

  bool wrapping = true;  // Tester avec le mode torique activé

  game g = game_new_ext(nb_rows, nb_cols, NULL, NULL, wrapping);
  if (g == NULL) return false;

  bool all_empty = true;  // Flag pour vérifier si toutes les cases sont vides

  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) != EMPTY) {
        all_empty = false;  // La pièce n'est pas vide
        break;
      }

      if (game_get_piece_orientation(g, i, j) != NORTH) {
        all_empty = false;  // L'orientation n'est pas NORTE
        break;
      }
    }
    if (!all_empty) break;  // Si une case n'est pas vide, on sort de la boucle
  }

  // Si le mode torique n'est pas activé, retourner une erreur
  if (!wrapping) {
    fprintf(stderr, "Erreur dans le test: mode torique incorrect\n");
    game_delete(g);
    return false;
  }

  game_delete(g);
  game_delete(gm);
  return all_empty;
}

bool test_game_new_empty_ext() {
  uint nb_rows = 5;  // Nombre de lignes du jeu
  uint nb_cols = 5;  // Nombre de colonnes du jeu
  bool wrapping = true;

  // Créer un jeu vide avec les dimensions spécifiées et le mode torique activé
  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);
  if (g == NULL) {
    return false;  // Si le jeu n'a pas pu être créé, retourner false
  }

  bool vide = true;  // Flag pour vérifier si toutes les cases sont vides

  for (int i = 0; i < nb_rows; i++) {
    for (int j = 0; j < nb_cols; j++) {
      shape s = game_get_piece_shape(g, i, j);  // Obtenir la forme de la pièce
      direction d = game_get_piece_orientation(
          g, i, j);  // Obtenir l'orientation de la pièce

      // Vérifier si la forme est vide et l'orientation est vers le nord
      if (s != EMPTY || d != NORTH) {
        vide = false;  // Si l'une des conditions n'est pas remplie, marquer
                       // comme non vide
        break;
      }
      if (!vide) break;
    }
  }

  // Vérifier que l'option de wrapping est correcte
  if (game_is_wrapping(g) != wrapping) {
    vide = false;
  }

  game_delete(g);

  return vide;
}

bool test_game_nb_rows() {
  uint rows = 4, cols = 5;
  game g = game_new_empty_ext(rows, cols, false);
  if (g == NULL) {
    fprintf(stderr, "Erreur : game_new_empty_ext a retourné NULL.\n");
    return false;
  }

  if (game_nb_rows(g) != rows) {
    fprintf(stderr, "Erreur : Nombre de lignes incorrect (%u).\n",
            game_nb_rows(g));
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_game_nb_cols() {
  uint rows = 4, cols = 5;
  game g = game_new_empty_ext(rows, cols, false);
  if (g == NULL) {
    fprintf(stderr, "Erreur : game_new_empty_ext a retourné NULL.\n");
    return false;
  }

  if (game_nb_cols(g) != cols) {
    fprintf(stderr, "Erreur : Nombre de colonnes incorrect (%u).\n",
            game_nb_cols(g));
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_game_is_wrapping() {
  game g1 = game_new_empty_ext(3, 3, true);
  if (g1 == NULL || !game_is_wrapping(g1)) {
    fprintf(stderr, "Erreur : Wrapping devrait être activé.\n");
    if (g1) game_delete(g1);
    return false;
  }

  game g2 = game_new_empty_ext(3, 3, false);
  if (g2 == NULL || game_is_wrapping(g2)) {
    fprintf(stderr, "Erreur : Wrapping devrait être désactivé.\n");
    if (g2) game_delete(g2);
    return false;
  }

  game_delete(g1);
  game_delete(g2);
  return true;
}

bool test_wrapping() {
  game g = game_new_empty_ext(3, 3, true);
  uint next_i, next_j;

  game_get_ajacent_square(g, 0, 2, EAST, &next_i, &next_j);
  if (next_j != 0) {
    fprintf(stderr, "Erreur : Wrapping horizontal incorrect.\n");
    game_delete(g);
    return false;
  }

  game_get_ajacent_square(g, 2, 0, SOUTH, &next_i, &next_j);
  if (next_i != 0) {
    fprintf(stderr, "Erreur : Wrapping vertical incorrect.\n");
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_cross_piece() {
  game g = game_new_empty_ext(3, 3, false);
  game_set_piece_shape(g, 1, 1, CROSS);

  if (!game_has_half_edge(g, 1, 1, NORTH) ||
      !game_has_half_edge(g, 1, 1, SOUTH) ||
      !game_has_half_edge(g, 1, 1, EAST) ||
      !game_has_half_edge(g, 1, 1, WEST)) {
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

bool test_undo() {
  game g = game_default();
  game gg = game_copy(g);
  game_play_move(g, 0, 0, 1);
  game_play_move(g, 0, 0, 1);
  game_undo(g);
  game_undo(g);
  bool result = game_equal(g, gg, false);
  game_delete(g);
  game_delete(gg);

  return result;
}
bool test_redo() {
  game g = game_default();
  game gg = game_copy(g);
  game_play_move(g, 0, 0, 1);
  game_play_move(gg, 0, 0, 1);
  game_play_move(g, 0, 0, 1);
  game_play_move(gg, 0, 0, 1);
  game_undo(g);
  game_redo(g);
  bool result = game_equal(g, gg, false);
  game_delete(g);
  game_delete(gg);

  return result;
}

bool test_rectangular_grid() {
  uint rows = 3;
  uint cols = 4;
  game g = game_new_empty_ext(rows, cols, false);

  if (game_nb_rows(g) != rows || game_nb_cols(g) != cols) {
    game_delete(g);
    return false;
  }

  for (uint i = 0; i < rows; i++) {
    for (uint j = 0; j < cols; j++) {
      game_set_piece_shape(g, i, j, CROSS);
      if (game_get_piece_shape(g, i, j) != CROSS) {
        game_delete(g);
        return false;
      }
    }
  }

  game_delete(g);
  return true;
}

bool test_history_clear() {
  game g = game_new_empty_ext(3, 3, false);

  game_play_move(g, 0, 0, 1);
  game_play_move(g, 1, 1, 1);

  game_undo(g);

  game_play_move(g, 2, 2, 1);

  direction before = game_get_piece_orientation(g, 1, 1);
  game_redo(g);
  direction after = game_get_piece_orientation(g, 1, 1);

  if (before != after) {
    game_delete(g);
    return false;
  }

  game_delete(g);
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Erreur : Aucun test spécifié.\n");
    return EXIT_FAILURE;
  }

  fprintf(stderr, "=> Start test \"%s\"\n", argv[1]);
  bool ok = false;

  if (strcmp("game_new_ext", argv[1]) == 0) {
    ok = test_game_new_ext();
  } else if (strcmp("game_new_empty_ext", argv[1]) == 0) {
    ok = test_game_new_empty_ext();
  } else if (strcmp("game_nb_rows", argv[1]) == 0) {
    ok = test_game_nb_rows();
  } else if (strcmp("game_nb_cols", argv[1]) == 0) {
    ok = test_game_nb_cols();
  } else if (strcmp("game_is_wrapping", argv[1]) == 0) {
    ok = test_game_is_wrapping();
  } else if (strcmp("wrapping", argv[1]) == 0) {
    ok = test_wrapping();
  } else if (strcmp("cross_piece", argv[1]) == 0) {
    ok = test_cross_piece();
  } else if (strcmp("game_undo", argv[1]) == 0) {
    ok = test_undo();
  } else if (strcmp("game_redo", argv[1]) == 0) {
    ok = test_redo();
  } else if (strcmp("rectangular_grid", argv[1]) == 0) {
    ok = test_rectangular_grid();
  } else if (strcmp("history_clear", argv[1]) == 0) {
    ok = test_history_clear();
  } else {
    fprintf(stderr, "Erreur : Test \"%s\" introuvable.\n", argv[1]);
    return EXIT_FAILURE;
  }

  if (ok) {
    fprintf(stderr, "Test \"%s\" terminé avec SUCCÈS.\n", argv[1]);
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Test \"%s\" a ÉCHOUÉ.\n", argv[1]);
    return EXIT_FAILURE;
  }
}

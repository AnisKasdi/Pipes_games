#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_moves.h"
#include "game_struct.h"

bool default_check(game g) {
  return game_get_piece_shape(g, 0, 0) == 3 &&
         game_get_piece_orientation(g, 0, 0) == WEST &&
         game_get_piece_shape(g, 0, 1) == 1 &&
         game_get_piece_orientation(g, 0, 1) == NORTH &&
         game_get_piece_shape(g, 0, 2) == 1 &&
         game_get_piece_orientation(g, 0, 2) == WEST &&
         game_get_piece_shape(g, 0, 3) == 3 &&
         game_get_piece_orientation(g, 0, 3) == NORTH &&
         game_get_piece_shape(g, 0, 4) == 1 &&
         game_get_piece_orientation(g, 0, 4) == SOUTH &&
         game_get_piece_shape(g, 1, 0) == 4 &&
         game_get_piece_orientation(g, 1, 0) == SOUTH &&
         game_get_piece_shape(g, 1, 1) == 4 &&
         game_get_piece_orientation(g, 1, 1) == WEST &&
         game_get_piece_shape(g, 1, 2) == 4 &&
         game_get_piece_orientation(g, 1, 2) == NORTH &&
         game_get_piece_shape(g, 1, 3) == 4 &&
         game_get_piece_orientation(g, 1, 3) == EAST &&
         game_get_piece_shape(g, 1, 4) == 4 &&
         game_get_piece_orientation(g, 1, 4) == EAST &&
         game_get_piece_shape(g, 2, 0) == 1 &&
         game_get_piece_orientation(g, 2, 0) == EAST &&
         game_get_piece_shape(g, 2, 1) == 1 &&
         game_get_piece_orientation(g, 2, 1) == NORTH &&
         game_get_piece_shape(g, 2, 2) == 4 &&
         game_get_piece_orientation(g, 2, 2) == WEST &&
         game_get_piece_shape(g, 2, 3) == 1 &&
         game_get_piece_orientation(g, 2, 3) == WEST &&
         game_get_piece_shape(g, 2, 4) == 2 &&
         game_get_piece_orientation(g, 2, 4) == EAST &&
         game_get_piece_shape(g, 3, 0) == 1 &&
         game_get_piece_orientation(g, 3, 0) == SOUTH &&
         game_get_piece_shape(g, 3, 1) == 4 &&
         game_get_piece_orientation(g, 3, 1) == SOUTH &&
         game_get_piece_shape(g, 3, 2) == 4 &&
         game_get_piece_orientation(g, 3, 2) == NORTH &&
         game_get_piece_shape(g, 3, 3) == 3 &&
         game_get_piece_orientation(g, 3, 3) == WEST &&
         game_get_piece_shape(g, 3, 4) == 2 &&
         game_get_piece_orientation(g, 3, 4) == NORTH &&
         game_get_piece_shape(g, 4, 0) == 1 &&
         game_get_piece_orientation(g, 4, 0) == EAST &&
         game_get_piece_shape(g, 4, 1) == 4 &&
         game_get_piece_orientation(g, 4, 1) == WEST &&
         game_get_piece_shape(g, 4, 2) == 1 &&
         game_get_piece_orientation(g, 4, 2) == SOUTH &&
         game_get_piece_shape(g, 4, 3) == 1 &&
         game_get_piece_orientation(g, 4, 3) == EAST &&
         game_get_piece_shape(g, 4, 4) == 1 &&
         game_get_piece_orientation(g, 4, 4) == SOUTH;
}

bool game_test_out_of_range(cgame g, int i, int j) {
  if (i < 0 || i >= g->nb_rows || j < 0 || j >= g->nb_cols) {
    return true;
  }
  return false;
}

// Fonction de test pour game_is_connected
int test_game_is_connected() {
  int result = EXIT_SUCCESS;
  uint nb_rows = 5;
  uint nb_cols = 5;

  // Initialisation du jeu par défaut
  game g = game_default();
  if (!g) {
    fprintf(stderr, "Erreur : Le jeu par défaut n'a pas pu être créé.\n");
    return EXIT_FAILURE;
  }

  // Test 1 : Vérifie que le jeu par défaut n'est pas connecté
  if (game_is_connected(g)) {
    fprintf(stderr,
            "Erreur : Le jeu par défaut ne devrait pas être connecté.\n");
    result = EXIT_FAILURE;
  }
  game_delete(g);

  // Initialisation de la solution par défaut
  g = game_default_solution();
  if (!g) {
    fprintf(stderr, "Erreur : Le jeu solution n'a pas pu être créé.\n");
    return EXIT_FAILURE;
  }

  // Test 2 : Vérifie que le jeu solution est bien connecté
  if (!game_is_connected(g)) {
    fprintf(stderr, "Erreur : Le jeu solution devrait être connecté.\n");
    result = EXIT_FAILURE;
  }

  // Modification de certaines pièces pour provoquer une déconnexion
  game_set_piece_shape(g, 3, 3, 1);
  game_set_piece_orientation(g, 3, 3, WEST);
  game_set_piece_shape(g, 3, 4, 1);
  game_set_piece_orientation(g, 3, 4, NORTH);
  game_set_piece_orientation(g, 4, 3, EAST);
  game_set_piece_orientation(g, 4, 4, WEST);

  // Test 3 : Vérifie que le jeu modifié n'est plus connecté
  if (game_is_connected(g)) {
    fprintf(
        stderr,
        "Erreur : Le jeu ne devrait plus être connecté après modification.\n");
    result = EXIT_FAILURE;
  }
  game_delete(g);

  if (result == EXIT_SUCCESS) {
    printf(
        "La fonction game_is_connected a passé tous les tests avec succès !\n");
  }

  return result;
}

// GAME DEFAULT
int test_game_has_half_edge() {
  bool test;
  int res = EXIT_SUCCESS;
  game g = game_new_empty();
  if (g == NULL) {
    fprintf(stderr, "Le jeu n'a pas été crée");
    res = EXIT_FAILURE;
  }
  game_set_piece_shape(g, 0, 0, SEGMENT);
  game_set_piece_orientation(g, 0, 0, NORTH);

  bool test_SOUTH = game_has_half_edge(g, 0, 0, SOUTH);
  bool test_NORTH = game_has_half_edge(g, 0, 0, NORTH);
  bool test_EAST = game_has_half_edge(g, 0, 0, EAST);
  bool test_WEST = game_has_half_edge(g, 0, 0, WEST);
  game_set_piece_shape(g, 0, 0, CROSS);
  game_set_piece_orientation(g, 0, 0, NORTH);
  bool test1 = game_has_half_edge(g, 0, 0, SOUTH);
  bool test2 = game_has_half_edge(g, 0, 0, NORTH);
  bool test3 = game_has_half_edge(g, 0, 0, EAST);
  bool test4 = game_has_half_edge(g, 0, 0, WEST);
  test = test_SOUTH && test_NORTH && !test_WEST && !test_EAST;
  test2 = test1 && test2 && test3 && test4;
  if (!test) {
    fprintf(stderr,
            "La fonction 'game_has_half_edge' Indique des information eronnée");
    res = EXIT_FAILURE;
  }
  if (!test2) {
    fprintf(stderr,
            "La fonction 'game_has_half_edge' Indique des information eronnée");
    res = EXIT_FAILURE;
  }
  return res;
}

int test_game_default() {
  int result = EXIT_SUCCESS;

  game g = game_default();
  if (!default_check(g)) {
    result = EXIT_FAILURE;
  }
  if (g == NULL) {
    fprintf(stderr, "Erreur : La fonction retourne une variable game vide.\n");
    result = EXIT_FAILURE;
  }

  if (g != NULL && game_won(g)) {
    fprintf(stderr, "Erreur : La fonction crée un jeu déjà résolu\n");
    result = EXIT_FAILURE;
  }
  // Vérification des pièces hors des limites
  for (int i = 0; i < g->nb_rows; i++) {
    for (int j = 0; j < g->nb_cols; j++) {
      if (game_test_out_of_range(g, i, j)) {
        fprintf(stderr,
                "Erreur : Le jeu créé par game_default() a une pièce hors des "
                "limites aux indices (%d, %d).\n",
                i, j);
        result = EXIT_FAILURE;
      }
    }
  }
  game_delete(g);
  if (result == EXIT_SUCCESS) {
    printf("La fonction game_default() a passé tous les tests avec succès !\n");
  }
  return result;
}

// Test dummy simple
int test_dummy() { return EXIT_SUCCESS; }

int test_game_default_solution() {
  int resultat = EXIT_SUCCESS;
  game test = game_default_solution();
  if (test == NULL) {
    fprintf(stderr,
            "La fonction 'Game_default_solution' retourne une variable "
            "Game de valeur NULL\n");
    resultat = EXIT_FAILURE;
  }
  if (!game_won(test)) {
    fprintf(stderr,
            "La fonction 'Game_default_solution' indique que le jeu n'est "
            "toujours pas gagné alors que le jeu devrait l'être\n ");
    resultat = EXIT_FAILURE;
  }
  // Vérification des pièces hors des limites
  for (int i = 0; i < test->nb_rows; i++) {
    for (int j = 0; j < test->nb_cols; j++) {
      if (game_test_out_of_range(test, i, j)) {
        fprintf(stderr,
                "Erreur : Le jeu créé par game_default() a une pièce hors des "
                "limites aux indices (%d, %d).\n",
                i, j);
        resultat = EXIT_FAILURE;
      }
    }
  }
  game_delete(test);
  game var = game_default();
  game solution = game_default_solution();
  if (var == solution) {
    fprintf(stderr,
            "La fonction game_default_solution a la même valeur que "
            "game_default\n");
    resultat = EXIT_FAILURE;
  }
  game_delete(var);
  game_delete(solution);
  if (resultat == EXIT_SUCCESS) {
    printf(
        "La fonction 'Game_default_solution' a passé tous les tests avec "
        "succès !\n");
  }
  return resultat;
}

// Test game_well_paired
bool test_game_is_well_paired() {
  game g = game_default_solution();
  game g1 = game_new_empty();
  game g2 = game_default();

  // verifier que la fonction renvoie vrai pour la solution

  if (!game_is_well_paired(g)) {
    fprintf(stderr, "test échoué pour default solution");
    return false;
  }

  // verifier que la fonction renvoie vrai pour un jeu vide

  if (!game_is_well_paired(g1)) {
    fprintf(stderr, "test échoué pour empty");
    return false;
  }

  // verifier que la fonction renvoie faux pour game_default

  if (game_is_well_paired(g2)) {
    fprintf(stderr, "test échoué pour default");
    return false;
  }

  for (int i = 0; i < g->nb_rows; i++) {
    for (int j = 0; j < g->nb_cols; j++) {
      shape actual_shape = game_get_piece_shape(g, i, j);
      game_set_piece_shape(g, i, j, EMPTY);
      if (game_is_well_paired(g)) {
        fprintf(stderr, "ERREUR avec EMPTY");
        game_delete(g);
        game_delete(g1);
        game_delete(g2);

        return false;
      }
      if (actual_shape == SEGMENT) {
        game_set_piece_shape(g, i, j, TEE);
        if (game_is_well_paired(g)) {
          fprintf(stderr, "ERREUR avec TEE");
          game_delete(g);
          game_delete(g1);
          game_delete(g2);
          return false;
        }
      }
      if (actual_shape == TEE) {
        game_set_piece_shape(g, i, j, SEGMENT);
        if (game_is_well_paired(g)) {
          fprintf(stderr, "ERREUR avec SEGMENT");
          game_delete(g);
          game_delete(g1);
          game_delete(g2);
          return false;
        }
      }
      if (actual_shape == ENDPOINT) {
        game_set_piece_shape(g, i, j, SEGMENT);
        if (!game_is_connected(g)) {
          fprintf(stderr, "ERREUR avec SEGMENT pour ENDPOINT");
          game_delete(g);
          game_delete(g1);
          game_delete(g2);
          return false;
        }
      }
      if (actual_shape == CORNER) {
        game_set_piece_shape(g, i, j, SEGMENT);
        if (game_is_well_paired(g)) {
          fprintf(stderr, "ERREUR avec SEGMENT pour CORNER");
          game_delete(g);
          game_delete(g1);
          game_delete(g2);
          return false;
        }
      }
      if (actual_shape == CROSS) {
        game_set_piece_shape(g, i, j, SEGMENT);
        if (game_is_well_paired(g)) {
          fprintf(stderr, "ERREUR avec SEGMENT pour CROSS");
          game_delete(g);
          game_delete(g1);
          game_delete(g2);
          return false;
        }
      }
      game_set_piece_shape(g, i, j, actual_shape);
    }
  }

  game_delete(g);
  game_delete(g1);
  game_delete(g2);
  return true;
}

int test_game_check_edge() {
  game g = game_default_solution();
  edge_status x = game_check_edge(g, 0, 0, EAST);
  edge_status y = game_check_edge(g, 0, 0, SOUTH);
  bool test1 = (x != MISMATCH) && (y != MISMATCH);
  game_play_move(g, 0, 0, 1);
  x = game_check_edge(g, 0, 0, EAST);
  y = game_check_edge(g, 0, 0, SOUTH);
  bool test2 = (x == MISMATCH) || (y == MISMATCH);
  game_delete(g);
  return (test1 && test2) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int test_get_adjacent_square() {
  game g = game_new_empty_ext(5, 5, false);
  uint pi_next, pj_next;
  bool test1 = game_get_ajacent_square(g, 0, 0, EAST, &pi_next, &pj_next);
  bool test2 = game_get_ajacent_square(g, 0, 0, SOUTH, &pi_next, &pj_next);
  bool test3 = !game_get_ajacent_square(g, 0, 0, NORTH, &pi_next, &pj_next);
  bool test4 = !game_get_ajacent_square(g, 0, 0, WEST, &pi_next, &pj_next);
  game_delete(g);
  game f = game_new_empty_ext(5, 5, true);
  bool test5 = game_get_ajacent_square(g, 0, 0, NORTH, &pi_next, &pj_next);
  bool test6 = game_get_ajacent_square(g, 0, 0, WEST, &pi_next, &pj_next);
  game_delete(f);
  return (test1 && test2 && test3 && test4 && test5 && test6) ? EXIT_SUCCESS
                                                              : EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
  // Vérifie que l'utilisateur a bien passé un argument
  if (argc != 2) {
    fprintf(
        stderr,
        "Erreur : il faut passer exactement un argument (le nom du test).\n");
    return EXIT_FAILURE;
  }

  // Début du test
  fprintf(stderr, "=> Start test \"%s\"\n", argv[1]);
  bool ok = false;

  // Exécution du test correspondant à l'argument
  if (strcmp("dummy", argv[1]) == 0) {
    return test_dummy();  // Test dummy avec retour direct
  } else if (strcmp("game_is_connected", argv[1]) == 0) {
    ok = test_game_is_connected() == EXIT_SUCCESS;
  } else if (strcmp("game_is_well_paired", argv[1]) == 0) {
    ok = test_game_is_well_paired();
  } else if (strcmp("game_check_edge", argv[1]) == 0) {
    ok = test_game_check_edge() == EXIT_SUCCESS;
  } else if (strcmp("get_adjacent_square", argv[1]) == 0) {
    ok = test_get_adjacent_square() == EXIT_SUCCESS;
  } else if (strcmp("game_default", argv[1]) == 0) {
    ok = test_game_default() == EXIT_SUCCESS;
  } else if (strcmp("game_default_solution", argv[1]) == 0) {
    ok = test_game_default_solution() == EXIT_SUCCESS;
  } else if (strcmp("game_has_half_edge", argv[1]) == 0) {
    ok = test_game_has_half_edge() == EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Erreur : test \"%s\" non trouvé !\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  // Affiche le résultat du test
  if (ok) {
    fprintf(stderr, "Test \"%s\" finished: SUCCESS\n", argv[1]);
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Test \"%s\" finished: FAILURE\n", argv[1]);
    return EXIT_FAILURE;
  }
}
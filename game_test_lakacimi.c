#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_moves.h"
#include "game_struct.h"

bool test_dummy() { return true; }

bool test_game_set_piece_orientation() {
  game g = game_new_empty();
  game_set_piece_orientation(g, 1, 1, EAST);
  direction result_orientation = game_get_piece_orientation(g, 1, 1);
  printf("Expected: %d, Found: %d\n", EAST, result_orientation);
  bool result = (result_orientation == EAST);
  game_delete(g);
  return result;
}

bool test_game_set_piece_shape() {
  game g = game_default();
  assert(g != NULL);

  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      shape initial_shape = game_get_piece_shape(g, i, j);
      shape new_shape = (initial_shape + 1) % NB_SHAPES;

      game_set_piece_shape(g, i, j, new_shape);
      shape result_shape = game_get_piece_shape(g, i, j);

      if (result_shape != new_shape) {
        game_delete(g);
        return false;
      }
    }
  }

  game_delete(g);
  return true;
}

bool test_game_delete() {
  game g = game_default();
  assert(g != NULL);
  // Effectuez quelques opérations sur le jeu pour s'assurer qu'il est valide
  game_set_piece_shape(g, 0, 0, CROSS);
  assert(game_get_piece_shape(g, 0, 0) == CROSS);
  game_delete(g);
  // Après la suppression, g est un pointeur invalide.
  g = NULL;
  // Créons un nouveau jeu pour s'assurer que nous pouvons encore allouer de la
  // mémoire
  game new_g = game_default();
  assert(new_g != NULL);
  game_delete(new_g);

  return true;
}

shape *get_random_shape_tab(void) {
  game g = game_default();
  shape *tab = malloc(g->nb_rows * g->nb_cols * sizeof(shape));
  if (tab == NULL) {
    return NULL;
  }

  for (int i = 0; i < g->nb_rows * g->nb_cols; i++) {
    tab[i] = rand() % NB_SHAPES;
  }

  game_delete(g);
  return tab;
}

direction *get_random_dir_tab(void) {
  game g = game_default();
  direction *tab = malloc(g->nb_rows * g->nb_cols * sizeof(direction));
  if (tab == NULL) {
    return NULL;
  }

  for (int i = 0; i < g->nb_rows * g->nb_cols; i++) {
    tab[i] = rand() % NB_DIRS;
  }

  game_delete(g);
  return tab;
}

bool test_game_equal() {
  game g1 = game_new_empty();
  game g2 = game_new_empty();
  if (!g1 || !g2) {
    fprintf(stderr, "Erreur: Échec de création des jeux\n");
    return false;
  }

  // Les jeux devraient être égaux initialement
  if (!game_equal(g1, g2, false)) {
    fprintf(stderr, "Erreur: Les jeux vides ne sont pas égaux\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Modifier g2 et vérifier qu'ils ne sont plus égaux
  game_set_piece_shape(g2, 0, 0, CROSS);

  // Déboguer l'état des deux jeux
  fprintf(stderr, "g1(0,0) shape: %d, orientation: %d\n",
          game_get_piece_shape(g1, 0, 0), game_get_piece_orientation(g1, 0, 0));
  fprintf(stderr, "g2(0,0) shape: %d, orientation: %d\n",
          game_get_piece_shape(g2, 0, 0), game_get_piece_orientation(g2, 0, 0));

  if (game_equal(g1, g2, false)) {
    fprintf(stderr, "Erreur: Les jeux sont égaux après modification\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  // Test avec ignore_orientation
  game_set_piece_shape(g1, 0, 0, CROSS);
  game_set_piece_orientation(g1, 0, 0, NORTH);
  game_set_piece_orientation(g2, 0, 0, EAST);

  if (!game_equal(g1, g2, true)) {
    fprintf(
        stderr,
        "Erreur: Les jeux devraient être égaux en ignorant l'orientation\n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }

  game_delete(g1);
  game_delete(g2);
  return true;
}

bool test_game_copy() {
  game g = game_default();
  if (g == NULL) {
    fprintf(stderr, "Erreur: game_default() a retourné NULL\n");
    return false;
  }
  printf("Jeu original créé avec succès\n");

  game g1 = game_copy(g);
  if (g1 == NULL) {
    fprintf(stderr, "Erreur: game_copy() a retourné NULL\n");
    game_delete(g);
    return false;
  }
  printf("Copie du jeu créée avec succès\n");

  bool equal = game_equal(g, g1, false);
  if (!equal) {
    fprintf(stderr, "Erreur: Les jeux ne sont pas égaux après la copie\n");
  } else {
    printf("Les jeux sont égaux après la copie\n");
  }

  game_delete(g);
  game_delete(g1);
  return equal;
}

bool test_game_new() {
  // Créer un jeu vide
  game g = game_new(NULL, NULL);
  if (g == NULL) {
    fprintf(stderr, "game is null!\n");
    return false;
  }

  // Vérifier que toutes les cases sont vides
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) != EMPTY) {
        fprintf(stderr,
                "La fonction game_get_piece_shape a retourné un square "
                "alors qu'elle était censée être vide !\n");
        game_delete(g);
        return false;
      }
    }
  }
  game gg = game_default();
  if (gg == NULL) {
    fprintf(stderr, "game_default a échoué!\n");
    game_delete(g);
    return false;
  }
  if (game_equal(g, gg, true)) {
    fprintf(stderr, "Les deux jeux ne devraient pas être égaux\n");
    game_delete(g);
    game_delete(gg);
    return false;
  }
  game_delete(g);
  game_delete(gg);
  return true;
}

bool test_game_new_empty(void) {
  // Création du jeu vide
  game g = game_new_empty();
  if (g == NULL) {
    fprintf(stderr, "ERROR: game_new_empty() returned NULL\n");
    return false;
  }

  // Test 1: Vérification que toutes les cases sont vides
  bool test1 = true;
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) != EMPTY) {
        test1 = false;
        break;
      }
    }
  }

  // Test 2: Vérification que toutes les orientations sont NORTH
  bool test2 = true;
  for (uint i = 0; i < g->nb_rows; i++) {
    for (uint j = 0; j < g->nb_cols; j++) {
      if (game_get_piece_orientation(g, i, j) != NORTH) {
        test2 = false;
        break;
      }
    }
  }

  // Nettoyage
  game_delete(g);

  // Retourne le résultat combiné des tests
  return test1 && test2;
}

void usage(int argc, char *argv[]) {
  fprintf(stderr, "Usage : %s <testname>\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc == 1) usage(argc, argv);  // Passez argc et argv ici

  bool res = false;
  fprintf(stderr, "=> Start test \"%s\"\n", argv[1]);

  if (strcmp("dummy", argv[1]) == 0) {
    res = test_dummy();
  } else if (strcmp("test_game_set_piece_orientation", argv[1]) == 0) {
    res = test_game_set_piece_orientation();
  } else if (strcmp("test_game_set_piece_shape", argv[1]) == 0) {
    res = test_game_set_piece_shape();
    res = test_game_set_piece_shape();
  } else if (strcmp("test_game_new", argv[1]) == 0) {
    res = test_game_new();
  } else if (strcmp("test_game_new_empty", argv[1]) == 0) {
    res = test_game_new_empty();
  } else if (strcmp("test_game_delete", argv[1]) == 0) {
    res = test_game_delete();
  } else if (strcmp("test_game_equal", argv[1]) == 0) {
    res = test_game_equal();
  } else if (strcmp("test_game_copy", argv[1]) == 0) {
    res = test_game_copy();
  } else {
    fprintf(stderr, "Erreur: test \"%s\" non trouvé!\n", argv[1]);
    return EXIT_FAILURE;
  }

  if (res) {
    printf("Test \"%s\" terminé : SUCCES\n", argv[1]);
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Test \"%s\" terminé : ECHEC\n", argv[1]);
    return EXIT_FAILURE;
  }
}

#include "game_tools.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_private.h"
#include "game_struct.h"
#include "queue.h"

bool test_game_load() {
  // Nom du fichier de test
  char* test_filename = "default.txt";

  // Créer un fichier de test avec des données prédéfinies
  FILE* file = fopen(test_filename, "w");
  if (file == NULL) {
    printf("Erreur : Impossible de créer le fichier %s\n", test_filename);
    return false;
  }
  fprintf(file, "5 5 0\n");           // nb_rows, nb_cols, wrapping
  fprintf(file, "CW NN NW CN NS\n");  // Row 1
  fprintf(file, "TS TW TN TE TE\n");  // Row 2
  fprintf(file, "NE NN TW NW SE\n");  // Row 3
  fprintf(file, "NS TS TN CW SN\n");  // Row 4
  fprintf(file, "NE TW NS NE NS\n");  // Row 5
  fclose(file);

  // Charger le jeu à partir du fichier
  game g = game_load(test_filename);

  // Vérifier les dimensions et le wrapping
  assert(game_nb_rows(g) == 5);
  assert(game_nb_cols(g) == 5);
  assert(game_is_wrapping(g) == false);

  // Vérifier les formes et les directions pour chaque position
  assert(game_get_piece_shape(g, 0, 0) == CORNER);
  assert(game_get_piece_orientation(g, 0, 0) == WEST);

  assert(game_get_piece_shape(g, 0, 1) == ENDPOINT);
  assert(game_get_piece_orientation(g, 0, 1) == NORTH);

  assert(game_get_piece_shape(g, 0, 2) == ENDPOINT);
  assert(game_get_piece_orientation(g, 0, 2) == WEST);

  assert(game_get_piece_shape(g, 0, 3) == CORNER);
  assert(game_get_piece_orientation(g, 0, 3) == NORTH);

  assert(game_get_piece_shape(g, 0, 4) == ENDPOINT);
  assert(game_get_piece_orientation(g, 0, 4) == SOUTH);

  // Continuer avec les vérifications pour chaque ligne et colonne...

  // Libérer les ressources du jeu
  game_delete(g);

  // Supprimer le fichier de test
  remove(test_filename);

  return true;
}

// Fonction de test pour game_save
bool test_game_save() {
  game g1 = game_default();

  game_save(g1, "test_game_load.txt");
  game g2 = game_load("test_game_load.txt");
  // game_print(g1);
  // game_print(g2);
  if (g2 == NULL) {
    printf("Erreur : Impossible de lire le fichier \n");
    game_delete(g1);
    game_delete(g2);
    return false;
  }
  bool test = game_equal(g1, g2, true);

  // printf("c'est reussit");
  game_delete(g1);
  game_delete(g2);
  return test;
}

bool test_game_random() {
  uint nb_rows = 5;
  uint nb_cols = 5;
  bool wrapping = false;
  uint nb_empty = 5;
  uint nb_extra = 2;

  // Générer un jeu aléatoire
  game g = game_random(nb_rows, nb_cols, wrapping, nb_empty, nb_extra);

  // Vérifier que le jeu n'est pas NULL
  if (g != NULL) {
    printf("Erreur ");
  }
  // Vérifier les dimensions et le wrapping
  assert(game_nb_rows(g) == nb_rows);
  assert(game_nb_cols(g) == nb_cols);
  assert(game_is_wrapping(g) == wrapping);

  // Vérifier le nombre de cases vides
  uint empty_count = 0;
  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) == EMPTY) {
        empty_count++;
      }
    }
  }
  assert(empty_count == nb_empty);

  // Vérifier qu'il y a au moins deux ENDPOINT
  uint endpoint_count = 0;
  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      if (game_get_piece_shape(g, i, j) == ENDPOINT) {
        endpoint_count++;
      }
    }
  }
  assert(endpoint_count >= 2);

  // Vous pouvez ajouter des vérifications supplémentaires ici selon les besoins

  // Libérer les ressources du jeu
  game_delete(g);

  return true;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    return EXIT_FAILURE;
  }

  bool ok = false;
  if (strcmp("game_load", argv[1]) == 0)
    ok = test_game_load();
  else if (strcmp("game_save", argv[1]) == 0)
    ok = test_game_save();
  else if (strcmp("game_random", argv[1]) == 0)
    ok = test_game_random();
  else {
    fprintf(stderr, "Error: test \"%s\" not found!\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  if (ok) {
    fprintf(stderr, "Test \"%s\" finished: SUCCESS\n", argv[1]);
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, "Test \"%s\" finished: FAILURE\n", argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_FAILURE;
}

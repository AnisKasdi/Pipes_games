#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_ext.h"
#include "game_private.h"
#include "game_struct.h"
#include "game_tools.h"
#include "queue.h"
#include "string.h"

int main(int argc, char *argv[]) {
  // Vérifier les arguments
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "Usage: %s <option> <input> [<output>]\n", argv[0]);
    fprintf(stderr, "Options: -s (solve), -c (count solutions)\n");
    return EXIT_FAILURE;
  }

  // Charger le jeu depuis le fichier d'entrée
  game g = game_load(argv[2]);
  if (!g) {
    fprintf(stderr, "Erreur : impossible de charger le fichier %s\n", argv[2]);
    return EXIT_FAILURE;
  }

  // Traiter l'option
  if (strcmp(argv[1], "-s") == 0) {
    // Option -s : trouver une solution
    if (!game_solve(g)) {
      game_delete(g);
      return EXIT_FAILURE;  // Pas de solution
    }

    // Sauvegarder ou afficher le résultat
    if (argc == 4) {
      game_save(g, argv[3]);
    } else {
      game_print(g);
    }

  } else if (strcmp(argv[1], "-c") == 0) {
    // Option -c : compter les solutions
    uint nb_solutions = game_nb_solutions(g);

    // Sauvegarder ou afficher le résultat
    if (argc == 4) {
      FILE *f = fopen(argv[3], "w");
      if (!f) {
        fprintf(stderr, "Erreur : impossible de créer %s\n", argv[3]);
        game_delete(g);
        return EXIT_FAILURE;
      }
      fprintf(f, "%u\n", nb_solutions);
      fclose(f);
    } else {
      printf("%u\n", nb_solutions);
    }

  } else {
    fprintf(stderr, "Option inconnue : %s\n", argv[1]);
    game_delete(g);
    return EXIT_FAILURE;
  }

  game_delete(g);
  return EXIT_SUCCESS;
}
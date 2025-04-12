#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "game_aux.h"
#include "game_private.h"
#include "game_tools.h"

int main(int argc, char *argv[]) {
  if (argc < 7) {
    fprintf(stderr,
            "Usage: %s <nb_rows> <nb_cols> <wrapping> <nb_empty> <nb_extra> "
            "<shuffle> [<filename>]\n",
            argv[0]);
    return EXIT_FAILURE;
  }

  uint nb_rows = atoi(argv[1]);
  uint nb_cols = atoi(argv[2]);
  bool wrapping = (atoi(argv[3]) != 0);
  uint nb_empty = atoi(argv[4]);
  uint nb_extra = atoi(argv[5]);
  bool shuffle = (atoi(argv[6]) != 0);
  char *filename = (argc >= 8) ? argv[7] : NULL;

  srand(time(NULL));

  game g = game_random(nb_rows, nb_cols, wrapping, nb_empty, nb_extra);
  if (!g) {
    fprintf(stderr, "Erreur : Impossible de générer le jeu.\n");
    return EXIT_FAILURE;
  }

  if (shuffle) {
    game_shuffle_orientation(g);
  }

  game_print(g);

  if (filename) {
    game_save(g, filename);  // Sauvegarde sans vérification de retour
    printf("Jeu sauvegardé dans %s\n", filename);
  }

  game_delete(g);
  return EXIT_SUCCESS;
}

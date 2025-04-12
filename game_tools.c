#include "game_tools.h"

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
#include "queue.h"

// Convertit une forme en caractère
char shape_to_char(shape s) {
  switch (s) {
    case SEGMENT:
      return 'S';
    case CROSS:
      return 'X';
    case TEE:
      return 'T';
    case ENDPOINT:
      return 'N';
    case EMPTY:
      return 'E';
    case CORNER:
      return 'C';
    default:
      fprintf(stderr, "Erreur : Forme inconnue (%d)\n", s);
      exit(EXIT_FAILURE);
  }
}

// Convertit une direction en caractère
char direction_to_char(direction d) {
  switch (d) {
    case SOUTH:
      return 'S';
    case WEST:
      return 'W';
    case NORTH:
      return 'N';
    case EAST:
      return 'E';
    default:
      fprintf(stderr, "Erreur : Orientation inconnue (%d)\n", d);
      exit(EXIT_FAILURE);
  }
}

// Convertit un caractère en forme
shape translate_char_to_shape(char a) {
  switch (a) {
    case 'C':
      return CORNER;
    case 'T':
      return TEE;
    case 'E':
      return EMPTY;
    case 'N':
      return ENDPOINT;
    case 'S':
      return SEGMENT;
    case 'X':
      return CROSS;
    default:
      fprintf(stderr, "Erreur : Forme inconnue '%c'\n", a);
      exit(EXIT_FAILURE);
  }
}

// Convertit un caractère en direction
direction translate_char_to_direction(char a) {
  switch (a) {
    case 'N':
      return NORTH;
    case 'S':
      return SOUTH;
    case 'E':
      return EAST;
    case 'W':
      return WEST;
    default:
      fprintf(stderr, "Erreur : Orientation inconnue '%c'\n", a);
      exit(EXIT_FAILURE);
  }
}

// Charge un jeu depuis un fichier
game game_load(char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", filename);
    exit(EXIT_FAILURE);
  }

  uint nb_rows, nb_cols, wrapping;
  int ret = fscanf(file, "%u %u %u", &nb_rows, &nb_cols, &wrapping);
  assert(ret == 3);
  bool res_wrapping = (wrapping != 0);

  shape* shapes = malloc(nb_rows * nb_cols * sizeof(shape));
  direction* dirs = malloc(nb_rows * nb_cols * sizeof(direction));

  for (uint i = 0; i < nb_rows; i++) {
    for (uint j = 0; j < nb_cols; j++) {
      char s, d;
      ret = fscanf(file, " %c%c", &s, &d);
      assert(ret == 2);
      shapes[i * nb_cols + j] = translate_char_to_shape(s);
      dirs[i * nb_cols + j] = translate_char_to_direction(d);
    }
  }

  fclose(file);
  game g = game_new_ext(nb_rows, nb_cols, shapes, dirs, res_wrapping);
  free(shapes);
  free(dirs);
  return g;
}

// Sauvegarde un jeu dans un fichier
void game_save(cgame g, char* filename) {
  FILE* file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "Erreur : Impossible de créer le fichier %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fprintf(file, "%u %u %u\n", game_nb_rows(g), game_nb_cols(g),
          game_is_wrapping(g) ? 1 : 0);

  for (uint i = 0; i < game_nb_rows(g); i++) {
    for (uint j = 0; j < game_nb_cols(g); j++) {
      shape s = game_get_piece_shape(g, i, j);
      direction d = game_get_piece_orientation(g, i, j);
      fprintf(file, "%c%c", shape_to_char(s), direction_to_char(d));
      if (j < game_nb_cols(g) - 1) fprintf(file, " ");
    }
    fprintf(file, "\n");
  }

  fclose(file);
}

// Génère un jeu aléatoire
game game_random(uint nb_rows, uint nb_cols, bool wrapping, uint nb_empty,
                 uint nb_extra) {
  if (nb_rows * nb_cols < 2 || nb_empty > (nb_rows * nb_cols - 2)) return NULL;

  game g = game_new_empty_ext(nb_rows, nb_cols, wrapping);
  if (!g) return NULL;

  uint i, j, ni, nj;
  direction d;
  bool placed = false;
  for (int attempts = 0; attempts < 100; attempts++) {
    i = rand() % nb_rows;
    j = rand() % nb_cols;
    d = rand() % NB_DIRS;
    if (game_get_ajacent_square(g, i, j, d, &ni, &nj)) {
      game_set_piece_shape(g, i, j, ENDPOINT);
      game_set_piece_orientation(g, i, j, d);
      game_set_piece_shape(g, ni, nj, ENDPOINT);
      game_set_piece_orientation(g, ni, nj, OPPOSITE_DIR(d));
      placed = true;
      break;
    }
  }
  if (!placed) {
    game_delete(g);
    return NULL;
  }

  int desired = nb_rows * nb_cols - nb_empty;
  int current = 2;
  while (current < desired) {
    typedef struct {
      uint i, j;
      direction d;
    } Candidate;
    Candidate* candidates = NULL;
    size_t count = 0;

    for (uint x = 0; x < nb_rows; x++) {
      for (uint y = 0; y < nb_cols; y++) {
        if (game_get_piece_shape(g, x, y) != EMPTY)
          for (direction dir = 0; dir < NB_DIRS; dir++)
            if (!game_has_half_edge(g, x, y, dir)) {
              uint adj_i, adj_j;
              if (game_get_ajacent_square(g, x, y, dir, &adj_i, &adj_j) &&
                  game_get_piece_shape(g, adj_i, adj_j) == EMPTY) {
                candidates =
                    realloc(candidates, (count + 1) * sizeof(Candidate));
                candidates[count++] = (Candidate){x, y, dir};
              }
            }
      }
    }

    if (count == 0) {
      game_delete(g);
      free(candidates);
      return NULL;
    }
    size_t idx = rand() % count;
    Candidate sel = candidates[idx];
    if (_add_edge(g, sel.i, sel.j, sel.d))
      current++;
    else {
      game_delete(g);
      free(candidates);
      return NULL;
    }
    free(candidates);
  }

  for (uint e = 0; e < nb_extra; e++) {
    typedef struct {
      uint i, j;
      direction d;
    } ExtraCandidate;
    ExtraCandidate* extras = NULL;
    size_t count = 0;

    for (uint x = 0; x < nb_rows; x++) {
      for (uint y = 0; y < nb_cols; y++) {
        if (game_get_piece_shape(g, x, y) != EMPTY)
          for (direction dir = 0; dir < NB_DIRS; dir++)
            if (!game_has_half_edge(g, x, y, dir)) {
              uint adj_i, adj_j;
              if (game_get_ajacent_square(g, x, y, dir, &adj_i, &adj_j) &&
                  game_get_piece_shape(g, adj_i, adj_j) != EMPTY &&
                  !game_has_half_edge(g, adj_i, adj_j, OPPOSITE_DIR(dir))) {
                extras = realloc(extras, (count + 1) * sizeof(ExtraCandidate));
                extras[count++] = (ExtraCandidate){x, y, dir};
              }
            }
      }
    }

    if (count == 0) break;
    size_t idx = rand() % count;
    ExtraCandidate sel = extras[idx];
    _add_edge(g, sel.i, sel.j, sel.d);
    free(extras);
  }

  return g;
}

// Retourne les orientations possibles pour une pièce
static void get_possible_directions(shape s, direction* dirs, uint* nb_dirs) {
  *nb_dirs = 0;
  switch (s) {
    case SEGMENT:
      dirs[(*nb_dirs)++] = NORTH;  // symétrie : NORTH/SOUTH
      dirs[(*nb_dirs)++] = EAST;   // symétrie : EAST/WEST
      break;
    case CROSS:
      dirs[(*nb_dirs)++] = NORTH;  // une seule orientation suffit
      break;
    default:  // CORNER, TEE, ENDPOINT, EMPTY
      for (direction d = NORTH; d <= WEST; d++) {
        dirs[(*nb_dirs)++] = d;
      }
      break;
  }
}

// Vérification complète locale (vérifie les voisins gauches et supérieurs,
// ainsi que les contraintes wrap-around si applicable)
static bool check_local_edges(cgame g, uint i, uint j) {
  if (j > 0 && game_check_edge(g, i, j - 1, EAST) == MISMATCH) return false;
  if (i > 0 && game_check_edge(g, i - 1, j, SOUTH) == MISMATCH) return false;
  if (game_is_wrapping(g)) {
    if (j == 0 && game_check_edge(g, i, game_nb_cols(g) - 1, EAST) == MISMATCH)
      return false;
    if (i == 0 && game_check_edge(g, game_nb_rows(g) - 1, j, SOUTH) == MISMATCH)
      return false;
  }
  return true;
}

// Vérification partielle : ne vérifie que les voisins qui sont assurément
// assignés
static bool check_partial_edges(cgame g, uint i, uint j) {
  // On vérifie uniquement le voisin à gauche et au-dessus,
  // car ils sont toujours assignés lors du parcours en ligne.
  if (j > 0 && game_check_edge(g, i, j - 1, EAST) == MISMATCH) return false;
  if (i > 0 && game_check_edge(g, i - 1, j, SOUTH) == MISMATCH) return false;
  return true;
}

// ------------------
// Résolution récursive pour trouver UNE solution
// ------------------
bool solve_recursive(game g, int index, uint* count, bool stop_early) {
  // Si le jeu est déjà résolu, incrémenter le compteur et retourner true
  if (game_won(g)) {
    if (count) (*count)++;
    return true;
  }

  // Si nous avons parcouru toutes les cases, il n'y a pas de solution
  if (index >= game_nb_rows(g) * game_nb_cols(g)) {
    return false;
  }

  // Calculer les coordonnées (i, j) correspondant à l'index
  uint i = index / game_nb_cols(g);
  uint j = index % game_nb_cols(g);

  // Récupérer la forme et l'orientation actuelles de la pièce
  shape s = game_get_piece_shape(g, i, j);
  direction original_orientation = game_get_piece_orientation(g, i, j);

  // Ignorer les cases vides
  if (s == EMPTY) {
    return solve_recursive(g, index + 1, count, stop_early);
  }

  // Déterminer le nombre d'orientations possibles pour la pièce
  int nb_orientations = (s == SEGMENT || s == CROSS) ? 2 : 4;

  for (int orientation = 0; orientation < nb_orientations; orientation++) {
    // Appliquer l'orientation courante
    game_set_piece_orientation(g, i, j, orientation);

    // Vérifier si cette configuration résout le jeu
    if (game_won(g)) {
      if (count) (*count)++;
      return true;
    }

    // Vérifier les bords adjacents pour éviter les incohérences
    bool valid_edges = true;

    if (i > 0 && game_check_edge(g, i, j, NORTH) == MISMATCH) {
      valid_edges = false;
    }
    if (j > 0 && game_check_edge(g, i, j, WEST) == MISMATCH) {
      valid_edges = false;
    }

    if (valid_edges) {
      // Continuer la résolution récursive
      if (solve_recursive(g, index + 1, count, stop_early)) {
        if (stop_early) {
          return true;  // Arrêter dès qu'une solution est trouvée
        }
      }
    }
  }

  // Restaurer l'orientation initiale avant de retourner false
  game_set_piece_orientation(g, i, j, original_orientation);
  return false;
}
// ------------------
// Comptage des solutions avec élagage optimisé
// ------------------
static void _count_solutions_recursive(game g, uint pos, uint* nb_solutions) {
  uint nb_rows = game_nb_rows(g);
  uint nb_cols = game_nb_cols(g);
  uint total = nb_rows * nb_cols;

  if (pos >= total) {
    if (game_won(g)) (*nb_solutions)++;
    return;
  }

  uint i = pos / nb_cols;
  uint j = pos % nb_cols;

  if (!game_is_wrapping(g)) {
    // Pour un jeu non wrapping, la ligne précédente est entièrement assignée
    if (j == 0 && i > 0) {
      for (uint c = 0; c < nb_cols; c++) {
        if (!check_local_edges(g, i - 1, c)) return;
      }
    }
    // Vérification complète pour la case courante
    if (!check_local_edges(g, i, j)) return;
  } else {
    // Pour les jeux wrapping, on effectue uniquement une vérification partielle
    if (!check_partial_edges(g, i, j)) return;
  }

  shape s = game_get_piece_shape(g, i, j);
  direction original_dir = game_get_piece_orientation(g, i, j);
  direction dirs[4];
  uint nb_dirs;
  get_possible_directions(s, dirs, &nb_dirs);

  for (uint k = 0; k < nb_dirs; k++) {
    game_set_piece_orientation(g, i, j, dirs[k]);
    _count_solutions_recursive(g, pos + 1, nb_solutions);
  }
  game_set_piece_orientation(g, i, j, original_dir);
}

// modification de la fonction game_solve :

bool game_solve(
    game g)  // on prends comme parametre le jeu qu'on a envie de résoudre
{
  bool solved = solve_recursive(
      g, 0, NULL, true);  // ici on execute la fonction solve_recursive
  // si une solution est trouvé notre variable solved ==true; sinon false dans
  // le cas contraire
  if (solved) {
    printf(
        "Solution a été trouvé avec succès ! le jeu a été résolu");  // on
                                                                     // affiche
                                                                     // un
                                                                     // message
                                                                     // dans le
                                                                     // terminal
  } else
    printf("Aucune solution n'a été trouvé pour le jeu !");

  // ici on retourne la valeur de solved
  return solved;
}

uint game_nb_solutions(cgame g) {
  if (!g) return 0;
  game g_copy = game_copy(g);
  uint nb_solutions = 0;
  _count_solutions_recursive(g_copy, 0, &nb_solutions);
  game_delete(g_copy);
  return nb_solutions;
}

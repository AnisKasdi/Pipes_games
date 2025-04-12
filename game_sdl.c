#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"
#include "game_tools.h"

#define FONT "res/arial.ttf"
#define FONTSIZE \
  18  // Réduire la taille de la police pour éviter le chevauchement
#define BACKGROUND "res/background.png"
#define EMPTY_IMG "res/empty.png"
#define ENDPOINT_IMG "res/endpoint.png"
#define SEGMENT_IMG "res/segment.png"
#define CORNER_IMG "res/corner.png"
#define TEE_IMG "res/tee.png"
#define CROSS_IMG "res/cross.png"

typedef enum { STATE_MENU, STATE_GAME, STATE_HELP } GameState;
typedef enum { TAB_COMMANDS, TAB_SHORTCUTS, TAB_TIPS } HelpTab;

typedef struct {
  SDL_Rect rect;
  char text[32];
  bool hovered;
  bool clicked;
} Button;

typedef struct {
  uint i, j;
  int dir;  // 1 pour clockwise, -1 pour anti-clockwise
} Move;

struct Env_t {
  game g;
  SDL_Texture* background;
  SDL_Texture* shapes[6];
  GameState state;
  char status_message[256];
  Move* move_history;
  int move_count, move_capacity;
  int redo_count;
  float transition_alpha;
  bool solution_shown;
  Button menu_buttons[3];     // Jouer, Aide, Quitter
  Button toolbar_buttons[1];  // Aide
  SDL_Window* help_popup;
  SDL_Renderer* help_renderer;
  Button help_back_button;  // Bouton "Retour" pour fermer la fenêtre d'aide
  Button help_tabs[3];      // Onglets : Commandes, Raccourcis, Astuces
  HelpTab current_tab;      // Onglet actuellement sélectionné
  float help_fade_alpha;    // Pour l'animation de fondu
};
typedef struct Env_t Env;

// Vérifie si une pièce peut être tournée
bool can_rotate_piece(shape s) { return (s != EMPTY && s != CROSS); }

Env* init(SDL_Window* win, SDL_Renderer* ren, int argc, char* argv[]) {
  Env* env = malloc(sizeof(Env));
  if (!env) {
    fprintf(stderr, "Erreur : allocation mémoire pour Env\n");
    exit(EXIT_FAILURE);
  }

  env->g = (argc > 1) ? game_load(argv[1]) : game_default();
  if (!env->g) {
    fprintf(stderr, "Erreur : chargement jeu\n");
    free(env);
    exit(EXIT_FAILURE);
  }

  env->state = STATE_MENU;
  env->transition_alpha = 0.0f;
  env->help_fade_alpha = 0.0f;
  env->current_tab = TAB_COMMANDS;  // Onglet par défaut
  strcpy(env->status_message, "Bienvenue dans le Puzzle Néon !");
  env->solution_shown = false;

  SDL_Surface* bg_surface = IMG_Load(BACKGROUND);
  if (!bg_surface) {
    fprintf(stderr, "Erreur : chargement de %s - %s\n", BACKGROUND,
            IMG_GetError());
    free(env);
    exit(EXIT_FAILURE);
  }
  env->background = SDL_CreateTextureFromSurface(ren, bg_surface);
  if (!env->background) {
    fprintf(stderr, "Erreur : création de la texture du fond - %s\n",
            SDL_GetError());
    SDL_FreeSurface(bg_surface);
    free(env);
    exit(EXIT_FAILURE);
  }
  SDL_FreeSurface(bg_surface);

  env->shapes[EMPTY] = IMG_LoadTexture(ren, EMPTY_IMG);
  env->shapes[ENDPOINT] = IMG_LoadTexture(ren, ENDPOINT_IMG);
  env->shapes[SEGMENT] = IMG_LoadTexture(ren, SEGMENT_IMG);
  env->shapes[CORNER] = IMG_LoadTexture(ren, CORNER_IMG);
  env->shapes[TEE] = IMG_LoadTexture(ren, TEE_IMG);
  env->shapes[CROSS] = IMG_LoadTexture(ren, CROSS_IMG);

  for (int i = 0; i < 6; i++) {
    if (!env->shapes[i]) {
      fprintf(stderr, "Erreur : chargement de la texture %d - %s\n", i,
              IMG_GetError());
      for (int j = 0; j < i; j++) SDL_DestroyTexture(env->shapes[j]);
      if (env->background) SDL_DestroyTexture(env->background);
      game_delete(env->g);
      free(env);
      exit(EXIT_FAILURE);
    }
  }

  env->move_capacity = 100;
  env->move_history = malloc(env->move_capacity * sizeof(Move));
  if (!env->move_history) {
    fprintf(stderr, "Erreur : allocation mémoire pour move_history\n");
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(env->shapes[i]);
    SDL_DestroyTexture(env->background);
    game_delete(env->g);
    free(env);
    exit(EXIT_FAILURE);
  }
  env->move_count = env->redo_count = 0;

  // Initialisation des boutons du menu
  const char* menu_texts[] = {"Jouer", "Aide", "Quitter"};
  for (int i = 0; i < 3; i++) {
    strcpy(env->menu_buttons[i].text, menu_texts[i]);
    env->menu_buttons[i].hovered = env->menu_buttons[i].clicked = false;
  }

  // Initialiser les boutons de la barre d'outils (seulement "Aide")
  const char* toolbar_texts[] = {"Aide"};
  for (int i = 0; i < 1; i++) {
    strcpy(env->toolbar_buttons[i].text, toolbar_texts[i]);
    env->toolbar_buttons[i].hovered = env->toolbar_buttons[i].clicked = false;
  }

  // Initialiser le bouton "Retour" de la fenêtre d'aide
  strcpy(env->help_back_button.text, "Retour");
  env->help_back_button.hovered = env->help_back_button.clicked = false;

  // Initialiser les onglets de la fenêtre d'aide
  const char* tab_texts[] = {"Commandes", "Raccourcis", "Astuces"};
  for (int i = 0; i < 3; i++) {
    strcpy(env->help_tabs[i].text, tab_texts[i]);
    env->help_tabs[i].hovered = env->help_tabs[i].clicked = false;
  }

  env->help_popup = NULL;
  env->help_renderer = NULL;

  return env;
}

void render_button(SDL_Renderer* ren, Button* btn, TTF_Font* font,
                   bool is_active) {
  // Fond avec dégradé (gris foncé à gris clair)
  SDL_Color base_color = btn->hovered ? (SDL_Color){80, 80, 80, 255}
                                      : (SDL_Color){50, 50, 50, 255};
  if (is_active)
    base_color =
        (SDL_Color){100, 100, 100, 255};  // Surlignage pour l'onglet actif
  for (int y = 0; y < btn->rect.h; y++) {
    float t = (float)y / btn->rect.h;
    SDL_SetRenderDrawColor(ren, base_color.r + t * 50, base_color.g + t * 50,
                           base_color.b + t * 50, 255);
    SDL_RenderDrawLine(ren, btn->rect.x, btn->rect.y + y,
                       btn->rect.x + btn->rect.w, btn->rect.y + y);
  }

  // Bordure blanche
  SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
  SDL_RenderDrawRect(ren, &btn->rect);

  // Effet de clic
  if (btn->clicked) {
    SDL_SetRenderDrawColor(ren, 200, 200, 200, 100);
    SDL_RenderFillRect(ren, &btn->rect);
  }

  // Texte blanc, centré dans le bouton
  SDL_Color text_color = {255, 255, 255, 255};
  SDL_Surface* surf = TTF_RenderUTF8_Blended(font, btn->text, text_color);
  SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
  SDL_Rect text_rect = {btn->rect.x + (btn->rect.w - surf->w) / 2,
                        btn->rect.y + (btn->rect.h - surf->h) / 2, surf->w,
                        surf->h};
  SDL_RenderCopy(ren, tex, NULL, &text_rect);
  SDL_FreeSurface(surf);
  SDL_DestroyTexture(tex);
}

void render_help_popup(SDL_Renderer* ren, Env* env) {
  // Fond gris foncé avec bordure blanche
  SDL_SetRenderDrawColor(ren, 30, 30, 30, 240);
  SDL_Rect bg = {0, 0, 400,
                 400};  // Réduire la hauteur pour une fenêtre plus compacte
  SDL_RenderFillRect(ren, &bg);
  SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
  SDL_RenderDrawRect(ren, &bg);

  // Animation de fondu
  if (env->help_fade_alpha < 1.0f) {
    env->help_fade_alpha += 0.05f;
    SDL_SetRenderDrawColor(ren, 0, 0, 0,
                           (Uint8)((1.0f - env->help_fade_alpha) * 255));
    SDL_RenderFillRect(ren, &bg);
  }

  TTF_Font* font = TTF_OpenFont(FONT, FONTSIZE);
  if (font) {
    // Titre "Aide"
    SDL_Color title_color = {255, 200, 0, 255};  // Jaune pour le titre
    SDL_Surface* title_surf = TTF_RenderUTF8_Blended(font, "Aide", title_color);
    SDL_Texture* title_tex = SDL_CreateTextureFromSurface(ren, title_surf);
    SDL_Rect title_rect = {bg.w / 2 - title_surf->w / 2, 10, title_surf->w,
                           title_surf->h};
    SDL_RenderCopy(ren, title_tex, NULL, &title_rect);
    SDL_FreeSurface(title_surf);
    SDL_DestroyTexture(title_tex);

    // Onglets
    for (int i = 0; i < 3; i++) {
      env->help_tabs[i].rect = (SDL_Rect){10 + i * 130, 40, 120, 30};
      render_button(ren, &env->help_tabs[i], font, env->current_tab == i);
    }

    // Contenu de l'onglet sélectionné
    SDL_Color text_color = {200, 200, 200, 255};  // Gris clair pour le texte
    int y_offset = 80;      // Position de départ pour le contenu
    int line_spacing = 25;  // Espacement entre les lignes

    if (env->current_tab == TAB_COMMANDS) {
      // Objectif du jeu
      SDL_Color section_color = {100, 200, 255,
                                 255};  // Bleu clair pour les sections
      SDL_Surface* section_surf =
          TTF_RenderUTF8_Blended(font, "Objectif du jeu", section_color);
      SDL_Texture* section_tex =
          SDL_CreateTextureFromSurface(ren, section_surf);
      SDL_Rect section_rect = {20, y_offset, section_surf->w, section_surf->h};
      SDL_RenderCopy(ren, section_tex, NULL, &section_rect);
      SDL_FreeSurface(section_surf);
      SDL_DestroyTexture(section_tex);

      const char* objective_text[] = {"Connectez les extremites de la grille",
                                      "en tournant les pieces."};
      y_offset += line_spacing;
      for (int i = 0; i < 2; i++) {
        SDL_Surface* surf =
            TTF_RenderUTF8_Blended(font, objective_text[i], text_color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_Rect rect = {30, y_offset + i * line_spacing, surf->w, surf->h};
        SDL_RenderCopy(ren, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
      }
      y_offset += 2 * line_spacing + 10;

      // Commandes de jeu
      SDL_Surface* section2_surf =
          TTF_RenderUTF8_Blended(font, "Commandes de jeu", section_color);
      SDL_Texture* section2_tex =
          SDL_CreateTextureFromSurface(ren, section2_surf);
      SDL_Rect section2_rect = {20, y_offset, section2_surf->w,
                                section2_surf->h};
      SDL_RenderCopy(ren, section2_tex, NULL, &section2_rect);
      SDL_FreeSurface(section2_surf);
      SDL_DestroyTexture(section2_tex);

      const char* commands_text[] = {
          "Clic gauche : Tourner horaire", "Clic droit : Tourner anti-horaire",
          "Ctrl+Z : Annuler (Undo)", "Ctrl+Y : Refaire (Redo)"};
      y_offset += line_spacing;
      for (int i = 0; i < 4; i++) {
        SDL_Surface* surf =
            TTF_RenderUTF8_Blended(font, commands_text[i], text_color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_Rect rect = {30, y_offset + i * line_spacing, surf->w, surf->h};
        SDL_RenderCopy(ren, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
      }
    } else if (env->current_tab == TAB_SHORTCUTS) {
      SDL_Color section_color = {100, 200, 255, 255};
      SDL_Surface* section_surf =
          TTF_RenderUTF8_Blended(font, "Raccourcis clavier", section_color);
      SDL_Texture* section_tex =
          SDL_CreateTextureFromSurface(ren, section_surf);
      SDL_Rect section_rect = {20, y_offset, section_surf->w, section_surf->h};
      SDL_RenderCopy(ren, section_tex, NULL, &section_rect);
      SDL_FreeSurface(section_surf);
      SDL_DestroyTexture(section_tex);

      const char* shortcuts_text[] = {
          "H : Afficher/fermer cette aide", "M : Melanger la grille",
          "S : Afficher la solution",       "N : Nouveau jeu",
          "C : Compter les solutions",      "R : Retour au menu",
          "Echap : Fermer cette aide"};
      y_offset += line_spacing;
      for (int i = 0; i < 7; i++) {
        SDL_Surface* surf =
            TTF_RenderUTF8_Blended(font, shortcuts_text[i], text_color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_Rect rect = {30, y_offset + i * line_spacing, surf->w, surf->h};
        SDL_RenderCopy(ren, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
      }
    } else if (env->current_tab == TAB_TIPS) {
      SDL_Color section_color = {100, 200, 255, 255};
      SDL_Surface* section_surf =
          TTF_RenderUTF8_Blended(font, "Astuces", section_color);
      SDL_Texture* section_tex =
          SDL_CreateTextureFromSurface(ren, section_surf);
      SDL_Rect section_rect = {20, y_offset, section_surf->w, section_surf->h};
      SDL_RenderCopy(ren, section_tex, NULL, &section_rect);
      SDL_FreeSurface(section_surf);
      SDL_DestroyTexture(section_tex);

      const char* tips_text[] = {
          "Alignez les pieces pour connecter",  "les extremites de la grille.",
          "Commencez par les coins pour mieux", "organiser vos connexions.",
          "Utilisez Undo (Ctrl+Z) si vous",     "etes bloque."};
      y_offset += line_spacing;
      for (int i = 0; i < 6; i++) {
        SDL_Surface* surf =
            TTF_RenderUTF8_Blended(font, tips_text[i], text_color);
        SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_Rect rect = {30, y_offset + i * line_spacing, surf->w, surf->h};
        SDL_RenderCopy(ren, tex, NULL, &rect);
        SDL_FreeSurface(surf);
        SDL_DestroyTexture(tex);
      }
    }

    // Bouton "Retour"
    env->help_back_button.rect = (SDL_Rect){150, 350, 100, 40};
    render_button(ren, &env->help_back_button, font, false);

    TTF_CloseFont(font);
  }
  SDL_RenderPresent(ren);
}

void render(SDL_Window* win, SDL_Renderer* ren, Env* env) {
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  SDL_SetRenderDrawColor(ren, 10, 10, 20, 255);
  SDL_RenderClear(ren);
  SDL_RenderCopy(ren, env->background, NULL, NULL);

  TTF_Font* font = TTF_OpenFont(FONT, FONTSIZE);
  if (!font) {
    fprintf(stderr, "Erreur : police - %s\n", TTF_GetError());
    return;
  }

  if (env->transition_alpha > 0) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, (Uint8)(env->transition_alpha * 255));
    SDL_Rect fade_rect = {0, 0, w, h};
    SDL_RenderFillRect(ren, &fade_rect);
    env->transition_alpha -= 0.05f;
  }

  if (env->state == STATE_MENU) {
    for (int i = 0; i < 3; i++) {
      env->menu_buttons[i].rect =
          (SDL_Rect){w / 2 - 120, h / 2 - 90 + i * 100, 240, 80};
      render_button(ren, &env->menu_buttons[i], font, false);
    }

    SDL_Color title_color = {255, 255, 255, 255};
    SDL_Surface* title_surf =
        TTF_RenderUTF8_Blended(font, "Puzzle Néon", title_color);
    if (title_surf) {
      SDL_Texture* title_tex = SDL_CreateTextureFromSurface(ren, title_surf);
      if (title_tex) {
        SDL_Rect title_rect = {w / 2 - title_surf->w / 2, 50, title_surf->w,
                               title_surf->h};
        SDL_RenderCopy(ren, title_tex, NULL, &title_rect);
        SDL_DestroyTexture(title_tex);
      }
      SDL_FreeSurface(title_surf);
    }
  } else if (env->state == STATE_GAME) {
    int grid_size = (w < h - 100) ? w : h - 100;
    int cell_size = grid_size / (game_nb_rows(env->g) > game_nb_cols(env->g)
                                     ? game_nb_rows(env->g)
                                     : game_nb_cols(env->g));
    int grid_w = cell_size * game_nb_cols(env->g);
    int grid_h = cell_size * game_nb_rows(env->g);
    int offset_x = (w - grid_w) / 2;
    int offset_y = (h - grid_h) / 2;

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
    SDL_Rect grid_bg = {offset_x - 10, offset_y - 10, grid_w + 20, grid_h + 20};
    SDL_RenderFillRect(ren, &grid_bg);

    SDL_SetRenderDrawColor(ren, 150, 150, 150, 255);
    for (int i = 0; i <= game_nb_rows(env->g); i++) {
      SDL_RenderDrawLine(ren, offset_x, offset_y + i * cell_size,
                         offset_x + grid_w, offset_y + i * cell_size);
    }
    for (int j = 0; j <= game_nb_cols(env->g); j++) {
      SDL_RenderDrawLine(ren, offset_x + j * cell_size, offset_y,
                         offset_x + j * cell_size, offset_y + grid_h);
    }

    for (uint i = 0; i < game_nb_rows(env->g); i++) {
      for (uint j = 0; j < game_nb_cols(env->g); j++) {
        shape s = game_get_piece_shape(env->g, i, j);
        if (s != EMPTY) {
          direction d = game_get_piece_orientation(env->g, i, j);
          SDL_Rect rect = {offset_x + j * cell_size, offset_y + i * cell_size,
                           cell_size, cell_size};
          SDL_RenderCopyEx(ren, env->shapes[s], NULL, &rect, d * 90, NULL,
                           SDL_FLIP_NONE);
        }
      }
    }

    for (int i = 0; i < 1; i++) {
      env->toolbar_buttons[i].rect = (SDL_Rect){w / 2 - 50, h - 60, 100, 40};
      render_button(ren, &env->toolbar_buttons[i], font, false);
    }
  }

  if (strlen(env->status_message) > 0) {
    SDL_Color status_color = {255, 255, 255, 255};
    SDL_Surface* status_surf =
        TTF_RenderUTF8_Blended(font, env->status_message, status_color);
    if (status_surf) {
      SDL_Texture* status_tex = SDL_CreateTextureFromSurface(ren, status_surf);
      if (status_tex) {
        SDL_Rect status_bg = {5, 5, status_surf->w + 10, status_surf->h + 10};
        SDL_SetRenderDrawColor(ren, 30, 30, 30, 200);
        SDL_RenderFillRect(ren, &status_bg);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderDrawRect(ren, &status_bg);
        SDL_Rect status_rect = {10, 10, status_surf->w, status_surf->h};
        SDL_RenderCopy(ren, status_tex, NULL, &status_rect);
        SDL_DestroyTexture(status_tex);
      }
      SDL_FreeSurface(status_surf);
    }
  }

  TTF_CloseFont(font);
  SDL_RenderPresent(ren);

  if (env->state == STATE_HELP && env->help_popup) {
    render_help_popup(env->help_renderer, env);
  }
}

bool point_in_rect(int x, int y, SDL_Rect* rect) {
  return (x >= rect->x && x < rect->x + rect->w && y >= rect->y &&
          y < rect->y + rect->h);
}

void add_move(Env* env, uint i, uint j, int dir) {
  if (env->move_count >= env->move_capacity) {
    env->move_capacity *= 2;
    Move* new_history =
        realloc(env->move_history, env->move_capacity * sizeof(Move));
    if (!new_history) {
      fprintf(stderr, "Erreur : realloc move_history\n");
      exit(EXIT_FAILURE);
    }
    env->move_history = new_history;
  }
  env->move_history[env->move_count++] = (Move){i, j, dir};
  env->redo_count = 0;
}

void close_help_window(Env* env) {
  if (env->help_popup) {
    SDL_DestroyRenderer(env->help_renderer);
    SDL_DestroyWindow(env->help_popup);
    env->help_popup = NULL;
    env->help_renderer = NULL;
    env->state = STATE_GAME;
  }
}

bool process(SDL_Window* win, SDL_Renderer* ren, Env* env, SDL_Event* e) {
  int w, h;
  SDL_GetWindowSize(win, &w, &h);
  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);

  if (e->type == SDL_QUIT) return true;

  if (e->type == SDL_KEYDOWN) {
    bool ctrl = (SDL_GetModState() & KMOD_CTRL) != 0;
    switch (e->key.keysym.sym) {
      case SDLK_j:
        if (env->state == STATE_MENU) {
          env->state = STATE_GAME;
          env->transition_alpha = 1.0f;
          strcpy(env->status_message, "Jeu démarré !");
        }
        break;
      case SDLK_z:
        if (ctrl && env->state == STATE_GAME) {
          if (env->move_count > 0) {
            Move m = env->move_history[--env->move_count];
            game_play_move(env->g, m.i, m.j, -m.dir);
            env->redo_count++;
            sprintf(env->status_message, "Undo effectué");
          } else {
            strcpy(env->status_message, "Rien à annuler");
          }
        }
        break;
      case SDLK_y:
        if (ctrl && env->state == STATE_GAME) {
          if (env->redo_count > 0) {
            Move m = env->move_history[env->move_count++];
            game_play_move(env->g, m.i, m.j, m.dir);
            env->redo_count--;
            sprintf(env->status_message, "Redo effectué");
          } else {
            strcpy(env->status_message, "Rien à refaire");
          }
        }
        break;
      case SDLK_h:
        if (env->state == STATE_GAME && !env->help_popup) {
          env->state = STATE_HELP;
          env->help_fade_alpha = 0.0f;
          env->current_tab = TAB_COMMANDS;
          int main_x, main_y, main_w, main_h;
          SDL_GetWindowPosition(win, &main_x, &main_y);
          SDL_GetWindowSize(win, &main_w, &main_h);
          int help_w = 400, help_h = 400;
          int help_x = main_x + (main_w - help_w) / 2;
          int help_y = main_y + (main_h - help_h) / 2;
          env->help_popup = SDL_CreateWindow("Aide", help_x, help_y, help_w,
                                             help_h, SDL_WINDOW_BORDERLESS);
          env->help_renderer =
              SDL_CreateRenderer(env->help_popup, -1, SDL_RENDERER_ACCELERATED);
        } else if (env->state == STATE_HELP && env->help_popup) {
          close_help_window(env);
        }
        break;
      case SDLK_ESCAPE:
        if (env->state == STATE_HELP && env->help_popup) {
          close_help_window(env);
        }
        break;
      case SDLK_m:
        if (env->state == STATE_GAME) {
          game_shuffle_orientation(env->g);
          strcpy(env->status_message, "Grille mélangée !");
          env->move_count = env->redo_count = 0;
        }
        break;
      case SDLK_s:
        if (env->state == STATE_GAME) {
          if (game_solve(env->g)) {
            strcpy(env->status_message, "Solution affichée !");
            env->solution_shown = true;
            render(win, ren, env);
          } else {
            strcpy(env->status_message, "Pas de solution");
          }
          env->move_count = env->redo_count = 0;
        }
        break;
      case SDLK_n:
        if (env->state == STATE_GAME) {
          game_delete(env->g);
          env->g = game_random(rand() % 5 + 4, rand() % 5 + 4, false, 2, 3);
          if (!env->g) {
            strcpy(env->status_message,
                   "Erreur : impossible de générer un nouveau jeu");
            env->state = STATE_MENU;
            break;
          }
          game_shuffle_orientation(env->g);
          strcpy(env->status_message, "Nouveau jeu généré !");
          env->move_count = env->redo_count = 0;
          env->solution_shown = false;
        }
        break;
      case SDLK_c:
        if (env->state == STATE_GAME) {
          char msg[50];
          sprintf(msg, "Solutions : %u", game_nb_solutions(env->g));
          strcpy(env->status_message, msg);
        }
        break;
      case SDLK_r:
        if (env->state == STATE_GAME) {
          env->state = STATE_MENU;
          strcpy(env->status_message, "Retour au menu");
        }
        break;
    }
  }

  if (env->state == STATE_MENU) {
    for (int i = 0; i < 3; i++) {
      env->menu_buttons[i].hovered =
          point_in_rect(mouse_x, mouse_y, &env->menu_buttons[i].rect);
      if (e->type == SDL_MOUSEBUTTONDOWN && env->menu_buttons[i].hovered) {
        env->menu_buttons[i].clicked = true;
        env->transition_alpha = 1.0f;
        switch (i) {
          case 0:  // Jouer
            env->state = STATE_GAME;
            strcpy(env->status_message, "Jeu démarré !");
            break;
          case 1:  // Aide
            env->state = STATE_HELP;
            env->help_fade_alpha = 0.0f;
            env->current_tab = TAB_COMMANDS;
            int main_x, main_y, main_w, main_h;
            SDL_GetWindowPosition(win, &main_x, &main_y);
            SDL_GetWindowSize(win, &main_w, &main_h);
            int help_w = 400, help_h = 400;
            int help_x = main_x + (main_w - help_w) / 2;
            int help_y = main_y + (main_h - help_h) / 2;
            env->help_popup = SDL_CreateWindow("Aide", help_x, help_y, help_w,
                                               help_h, SDL_WINDOW_BORDERLESS);
            env->help_renderer = SDL_CreateRenderer(env->help_popup, -1,
                                                    SDL_RENDERER_ACCELERATED);
            break;
          case 2:  // Quitter
            return true;
        }
      } else if (e->type == SDL_MOUSEBUTTONUP) {
        env->menu_buttons[i].clicked = false;
      }
    }
  } else if (env->state == STATE_GAME) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
      int grid_size = (w < h - 100) ? w : h - 100;
      int cell_size = grid_size / (game_nb_rows(env->g) > game_nb_cols(env->g)
                                       ? game_nb_rows(env->g)
                                       : game_nb_cols(env->g));
      int grid_w = cell_size * game_nb_cols(env->g);
      int grid_h = cell_size * game_nb_rows(env->g);
      int offset_x = (w - grid_w) / 2;
      int offset_y = (h - grid_h) / 2;

      if (mouse_x >= offset_x && mouse_x < offset_x + grid_w &&
          mouse_y >= offset_y && mouse_y < offset_y + grid_h) {
        uint j = (mouse_x - offset_x) / cell_size;
        uint i = (mouse_y - offset_y) / cell_size;
        if (i < game_nb_rows(env->g) && j < game_nb_cols(env->g)) {
          shape s = game_get_piece_shape(env->g, i, j);
          if (!can_rotate_piece(s)) {
            strcpy(env->status_message,
                   "Erreur : cette pièce ne peut pas être tournée");
          } else {
            int dir = (e->button.button == SDL_BUTTON_LEFT) ? 1 : -1;
            if (e->button.button == SDL_BUTTON_LEFT ||
                e->button.button == SDL_BUTTON_RIGHT) {
              env->solution_shown = false;
              game_play_move(env->g, i, j, dir);
              add_move(env, i, j, dir);
            }
          }
        }
      }

      for (int i = 0; i < 1; i++) {
        env->toolbar_buttons[i].hovered =
            point_in_rect(mouse_x, mouse_y, &env->toolbar_buttons[i].rect);
        if (e->type == SDL_MOUSEBUTTONDOWN && env->toolbar_buttons[i].hovered) {
          env->toolbar_buttons[i].clicked = true;
          if (i == 0 && !env->help_popup) {  // Aide
            env->state = STATE_HELP;
            env->help_fade_alpha = 0.0f;
            env->current_tab = TAB_COMMANDS;
            int main_x, main_y, main_w, main_h;
            SDL_GetWindowPosition(win, &main_x, &main_y);
            SDL_GetWindowSize(win, &main_w, &main_h);
            int help_w = 400, help_h = 400;
            int help_x = main_x + (main_w - help_w) / 2;
            int help_y = main_y + (main_h - help_h) / 2;
            env->help_popup = SDL_CreateWindow("Aide", help_x, help_y, help_w,
                                               help_h, SDL_WINDOW_BORDERLESS);
            env->help_renderer = SDL_CreateRenderer(env->help_popup, -1,
                                                    SDL_RENDERER_ACCELERATED);
          }
        } else if (e->type == SDL_MOUSEBUTTONUP) {
          env->toolbar_buttons[i].clicked = false;
        }
      }
    }

    if (game_won(env->g) && !env->solution_shown) {
      strcpy(env->status_message,
             "Bien joué ! Appuyez sur 'N' pour un nouveau jeu ou 'R' pour "
             "retourner au menu");
      env->solution_shown = true;
    }
  } else if (env->state == STATE_HELP && env->help_popup) {
    int wx, wy;
    SDL_GetWindowPosition(env->help_popup, &wx, &wy);
    int global_x, global_y;
    SDL_GetGlobalMouseState(&global_x, &global_y);
    int rel_x = global_x - wx;
    int rel_y = global_y - wy;

    // Gestion des onglets
    for (int i = 0; i < 3; i++) {
      env->help_tabs[i].hovered =
          point_in_rect(rel_x, rel_y, &env->help_tabs[i].rect);
      if (e->type == SDL_MOUSEBUTTONDOWN && env->help_tabs[i].hovered) {
        env->help_tabs[i].clicked = true;
      } else if (e->type == SDL_MOUSEBUTTONUP && env->help_tabs[i].hovered &&
                 env->help_tabs[i].clicked) {
        env->current_tab = (HelpTab)i;
        env->help_tabs[i].clicked = false;
      }
    }

    // Gestion du bouton "Retour"
    env->help_back_button.hovered =
        point_in_rect(rel_x, rel_y, &env->help_back_button.rect);
    if (e->type == SDL_MOUSEBUTTONDOWN && env->help_back_button.hovered) {
      env->help_back_button.clicked = true;
    } else if (e->type == SDL_MOUSEBUTTONUP && env->help_back_button.hovered &&
               env->help_back_button.clicked) {
      close_help_window(env);
      env->help_back_button.clicked = false;
    }
  }

  return false;
}

void clean(SDL_Window* win, SDL_Renderer* ren, Env* env) {
  if (env->help_popup) {
    SDL_DestroyRenderer(env->help_renderer);
    SDL_DestroyWindow(env->help_popup);
  }
  SDL_DestroyTexture(env->background);
  for (int i = 0; i < 6; i++) SDL_DestroyTexture(env->shapes[i]);
  game_delete(env->g);
  free(env->move_history);
  free(env);
}
cmake_minimum_required(VERSION 3.5)
project(game_project C)

include(CTest)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -g")

## Trouver SDL2 via votre script sdl2.cmake
include(sdl2.cmake)
message(STATUS "SDL2 include dir: ${SDL2_ALL_INC}")
message(STATUS "SDL2 all libraries: ${SDL2_ALL_LIBS}")

# Ajouter les répertoires d'inclusion pour SDL2 et les fichiers du projet
include_directories(${SDL2_ALL_INC} ${CMAKE_SOURCE_DIR})

# Création de la bibliothèque statique "game"
add_library(game 
    game.c 
    game_aux.c 
    game_ext.c 
    queue.c 
    game_tools.c 
    game_private.c 
    game_random.c
)

# Déclaration des exécutables
add_executable(game_text game_text.c)
add_executable(game_random game_random.c)
add_executable(game_solve game_solve.c)
add_executable(game_test_ankasdi game_test_ankasdi.c)
add_executable(game_test_whaddadou game_test_whaddadou.c)
add_executable(game_test_lakacimi game_test_lakacimi.c)
add_executable(game_test_ext game_test_ext.c)
add_executable(game_sdl main.c game_sdl.c)

# Lier la bibliothèque "game" aux exécutables
target_link_libraries(game_text game)
target_link_libraries(game_solve game)
target_link_libraries(game_random game)
target_link_libraries(game_test_ankasdi game)
target_link_libraries(game_test_whaddadou game)
target_link_libraries(game_test_lakacimi game)
target_link_libraries(game_test_ext game)

# Lier "demo" à game, SDL2 et libm (math)
target_link_libraries(game_sdl game ${SDL2_ALL_LIBS} m)

# Copier le répertoire "res" dans le dossier de build pour que les ressources soient accessibles
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


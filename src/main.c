#include "game_renderer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    Game game;
    if (!initGameRenderer(&game)) {
        return EXIT_FAILURE;
    }

    while (game.running) {
        checkEvents(&game);
        drawGameStateSync(&game);
    }

    destroyGameRenderer(&game);

    return EXIT_SUCCESS;
}

#include "constants.h"
#include "frontend_renderer.h"
#include "simulation.h"
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
    Sim sim;
    sim.colony.x = SCREEN_W / 4.0;
    sim.colony.y = SCREEN_H / 2.0;
    sim.width = 256;
    sim.height = 144;

    Frontend frontend;
    if (!initFrontendRenderer(&frontend)) {
        return EXIT_FAILURE;
    }

    while (frontend.running) {
        checkEvents(&frontend);
        drawFrameSync(&frontend, sim);
    }

    destroyFrontendRenderer(&frontend);

    return EXIT_SUCCESS;
}

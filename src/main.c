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
    if (!initSim(&sim)) {
        return EXIT_FAILURE;
    }

    Frontend frontend;
    if (!initFrontendRenderer(&frontend)) {
        return EXIT_FAILURE;
    }

    while (frontend.running) {
        checkEvents(&frontend);
        drawFrameSync(&frontend, sim);
        nextSimStep(&sim);
    }

    destroyFrontendRenderer(&frontend);
    destroySim(&sim);

    return EXIT_SUCCESS;
}

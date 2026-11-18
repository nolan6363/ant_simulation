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
    if (!initSim(&sim)) {
        return EXIT_FAILURE;
    }

    Frontend frontend;
    if (!initFrontendRenderer(&frontend)) {
        return EXIT_FAILURE;
    }

    for (int x = SIM_W / 2 - 20; x < SIM_W / 2 - 15; x++) {
        for (int y = 0; y < SIM_H - 50; y++) {
            sim.simGrid[y][x].wall = true;
        }
    }
    for (int x = SIM_W / 2 + 15; x < SIM_W / 2 + 20; x++) {
        for (int y = 50; y < SIM_H; y++) {
            sim.simGrid[y][x].wall = true;
        }
    }

    srand(0);

    while (sim.running) {
        handleEvents(&frontend, &sim);
        drawFrameSync(&frontend, sim);
        if (!sim.paused) {
            SDL_Time start = SDL_GetTicksNS();
            nextSimStep(&sim);
            frontend.tickTime = SDL_GetTicksNS() - start;
        }
    }

    destroyFrontendRenderer(&frontend);
    destroySim(&sim);

    return EXIT_SUCCESS;
}

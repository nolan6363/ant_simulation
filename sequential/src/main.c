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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif

typedef struct {
    Frontend *frontend;
    Sim *snapshot;
    char path[64];
} SaveTask;

void *saveThread(void *arg) {
    SaveTask *task = (SaveTask *)arg;
    saveFrame(task->frontend, *task->snapshot, task->path);
    return NULL;
}

// usage : ./main
//         ./main <steps> [saveEvery] [dumpFile]
//         steps = 0 : jusqu'à épuisement de la nourriture
int main(int argc, char *argv[]) {
    bool headless = argc >= 2;
    long nSteps = headless ? atol(argv[1]) : 0;
    long saveEvery = argc >= 3 ? atol(argv[2]) : 0;
    const char *dumpPath = argc >= 4 ? argv[3] : NULL;

    srand(0);

    Sim sim;
    if (!initSim(&sim)) {
        return EXIT_FAILURE;
    }

    Frontend frontend;
    if (headless) {
        if (!initHeadlessRenderer(&frontend)) {
            return EXIT_FAILURE;
        }
    } else {
        if (!initFrontendRenderer(&frontend)) {
            return EXIT_FAILURE;
        }
    }

    /*
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
    */

    SDL_Time runStart = SDL_GetTicksNS();

    if (headless) {
        Sim snapshot;
        if (!initSim(&snapshot)) {
            return EXIT_FAILURE;
        }
        SaveTask task = {.frontend = &frontend, .snapshot = &snapshot};
        pthread_t saver;
        bool saving = false;
        if (saveEvery > 0)
            SDL_CreateDirectory("frames");

        long step = 0;
        while ((nSteps == 0 || step < nSteps) && sim.foodCells > 0) {
            step++;
            SDL_Time start = SDL_GetTicksNS();
            nextSimStep(&sim);
            frontend.tickTime = SDL_GetTicksNS() - start;
            pushStat(&frontend.tickStats, frontend.tickTime / 1e6);

            if (saveEvery > 0 && step % saveEvery == 0) {
                if (saving)
                    pthread_join(saver, NULL);
                copySim(&snapshot, &sim);
                sprintf(task.path, "frames/step_%06ld.bmp", step);
                pthread_create(&saver, NULL, saveThread, &task);
                saving = true;
            }
        }
        if (saving)
            pthread_join(saver, NULL);

        printf("Steps : %ld, cellules de nourriture restantes : %d\n", step,
               sim.foodCells);
        if (dumpPath)
            dumpSim(&sim, dumpPath);
        destroySim(&snapshot);
    } else {
        while (sim.running) {
            handleEvents(&frontend, &sim);
            drawFrameSync(&frontend, sim);
            if (!sim.paused) {
                SDL_Time start = SDL_GetTicksNS();
                nextSimStep(&sim);
                frontend.tickTime = SDL_GetTicksNS() - start;
                pushStat(&frontend.tickStats, frontend.tickTime / 1e6);
            }
        }
    }

    SDL_Time runTime = SDL_GetTicksNS() - runStart;

#ifdef _OPENMP
    printf("\n=== Stats (%d threads OpenMP) ===\n", omp_get_max_threads());
#else
    printf("\n=== Stats (séquentiel) ===\n");
#endif
    printf("Temps total : %.3f s\n", runTime / 1e9);
    printStatsHeader();
    if (!headless)
        printStats("Frame time", &frontend.frameStats);
    printStats("Tick time", &frontend.tickStats);

    destroyFrontendRenderer(&frontend);
    destroySim(&sim);

    return EXIT_SUCCESS;
}

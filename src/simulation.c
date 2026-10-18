#include "simulation.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>

bool initSim(Sim *sim) {
    sim->colony.x = SCREEN_W / 4.0;
    sim->colony.y = SCREEN_H / 2.0;
    sim->width = 256;
    sim->height = 144;

    sim->simGrid = (simCell **)malloc(sim->height * sizeof(simCell *));
    if (sim->simGrid == NULL)
        return false;
    for (int i = 0; i < sim->height; i++) {
        sim->simGrid[i] = (simCell *)calloc(sim->width, sizeof(simCell));
        if (sim->simGrid[i] == NULL)
            return false;
    }

    Coord2D foodCenter = {.x = (sim->width / 4) * 3, .y = sim->height / 2};
    float foodRadius = 50;

    // put food on cells
    for (int y = 0; y < sim->height; y++) {
        for (int x = 0; x < sim->width; x++) {
            if ((x - foodCenter.x) * (x - foodCenter.x) +
                    (y - foodCenter.y) * (y - foodCenter.y) <
                foodRadius) {
                sim->simGrid[y][x].food = 1.0;
            }
        }
    }

    return true;
}

void destroySim(Sim *sim) {
    for (int i = 0; i < sim->height; i++) {
        free(sim->simGrid[i]);
    }
    free(sim->simGrid);
}

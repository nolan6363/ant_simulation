#include "simulation.h"
#include "constants.h"
#include <math.h>
#include <stdlib.h>

bool initSim(Sim *sim) {
    sim->width = 256;
    sim->height = 144;
    sim->colony.x = sim->width / 4.0;
    sim->colony.y = sim->height / 2.0;
    sim->antSpeed = 0.2;

    sim->simGrid = (simCell **)malloc(sim->height * sizeof(simCell *));
    if (sim->simGrid == NULL)
        return false;
    for (int i = 0; i < sim->height; i++) {
        sim->simGrid[i] = (simCell *)calloc(sim->width, sizeof(simCell));
        if (sim->simGrid[i] == NULL)
            return false;
    }

    Coord2D foodCenter = {.x = ((float)sim->width / 4.0) * 3,
                          .y = (float)sim->height / 2.0};
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

    // summon ants
    sim->ants = (Ant *)calloc(N_ANTS, sizeof(Ant));
    for (int i = 0; i < N_ANTS; i++) {
        sim->ants[i].pos.x = sim->colony.x;
        sim->ants[i].pos.y = sim->colony.y;
        sim->ants[i].theta = ((2 * M_PI) / (float)N_ANTS) * i;
    }

    return true;
}

void nextSimStep(Sim *sim) {
    for (int i = 0; i < N_ANTS; i++) {
        sim->ants[i].pos.x += cos(sim->ants[i].theta) * sim->antSpeed;
        sim->ants[i].pos.y += sin(sim->ants[i].theta) * sim->antSpeed;
        if ((sim->ants[i].pos.x <= 0) | (sim->ants[i].pos.x > sim->width)) {
            sim->ants[i].theta = M_PI - sim->ants[i].theta;
        }
        if ((sim->ants[i].pos.y <= 0) | (sim->ants[i].pos.y > sim->height)) {
            sim->ants[i].theta = -sim->ants[i].theta;
        }
    }
}

void destroySim(Sim *sim) {
    for (int i = 0; i < sim->height; i++) {
        free(sim->simGrid[i]);
    }
    free(sim->simGrid);
    free(sim->ants);
}

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
        Ant *ant = &sim->ants[i];

        // walk the ant
        ant->pos.x += cos(ant->theta) * sim->antSpeed;
        ant->pos.y += sin(ant->theta) * sim->antSpeed;
        if (ant->pos.x <= 0) {
            ant->pos.x = 0;
            ant->theta = M_PI - ant->theta;
        } else if (ant->pos.x > sim->width - 1) {
            ant->pos.x = sim->width - 1;
            ant->theta = M_PI - ant->theta;
        }
        if (ant->pos.y <= 0) {
            ant->pos.y = 0;
            ant->theta = -ant->theta;
        } else if (ant->pos.y > sim->height - 1) {
            ant->pos.y = sim->height - 1;
            ant->theta = -ant->theta;
        }

        // drop pheromone
        if (ant->has_food == false) {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneA += 0.1;
        } else {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneB += 0.1;
        }

        if (sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].food > 0.0) {
            ant->has_food = true;
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

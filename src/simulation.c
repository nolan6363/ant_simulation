#include "simulation.h"
#include "constants.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>

bool initSim(Sim *sim) {
    sim->colony.x = SIM_W / 4.0;
    sim->colony.y = SIM_H / 2.0;
    sim->antSpeed = 0.5;
    sim->running = true;
    sim->paused = false;

    sim->simGrid = (simCell **)malloc(SIM_H * sizeof(simCell *));
    if (sim->simGrid == NULL)
        return false;
    for (int i = 0; i < SIM_H; i++) {
        sim->simGrid[i] = (simCell *)calloc(SIM_W, sizeof(simCell));
        if (sim->simGrid[i] == NULL)
            return false;
    }

    Coord2D foodCenter = {.x = ((float)SIM_W / 4.0) * 3,
                          .y = (float)SIM_H / 2.0};
    float foodRadius = 10;

    // put food on cells
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if ((x - foodCenter.x) * (x - foodCenter.x) +
                    (y - foodCenter.y) * (y - foodCenter.y) <
                foodRadius * foodRadius) {
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
        sim->ants[i].pheromoneStrenght = 1.0;
    }

    return true;
}

// the ant looks for pheromones in its field of view and goes to the average
// direction
float antGetDirectionSignal(Sim *sim, const Ant ant) {
    float wAvgAngle = 0.0f;
    float totalWheight = 0.0f;
    for (int x = max((int)ant.pos.x - ANT_FOV_RADIUS, 0);
         x <= min((int)ant.pos.x + ANT_FOV_RADIUS, SIM_W - 1); x++) {
        for (int y = max((int)ant.pos.y - ANT_FOV_RADIUS, 0);
             y <= min((int)ant.pos.y + ANT_FOV_RADIUS, SIM_H - 1); y++) {
            if ((x - ant.pos.x) * (x - ant.pos.x) +
                    (y - ant.pos.y) * (y - ant.pos.y) <
                ANT_FOV_RADIUS * ANT_FOV_RADIUS) {
                double angle = remainder(
                    atan2((double)(y - ant.pos.y), (double)(x - ant.pos.x)) -
                        ant.theta,
                    2 * M_PI);
                if (angle < ANT_FOV_ANGLE && angle > -ANT_FOV_ANGLE) {
                    if (ant.has_food) {
                        if ((x - sim->colony.x) * (x - sim->colony.x) +
                                (y - sim->colony.y) * (y - sim->colony.y) <
                            (COLONY_RADIUS * COLONY_RADIUS)) {
                            wAvgAngle += angle * 100.0;
                            totalWheight += 100.0;
                        } else {
                            wAvgAngle += angle * sim->simGrid[y][x].pheromoneA;
                            totalWheight += sim->simGrid[y][x].pheromoneA;
                        }
                    } else if (sim->simGrid[y][x].food > 0.0) {
                        wAvgAngle += angle * 100.0;
                        totalWheight += 100.0;
                    } else {
                        wAvgAngle += angle * sim->simGrid[y][x].pheromoneB;
                        totalWheight += sim->simGrid[y][x].pheromoneB;
                    }
                }
            }
        }
    }

    return (totalWheight == 0.0f) ? 0.0 : wAvgAngle / totalWheight;
}

void nextSimStep(Sim *sim) {
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            sim->simGrid[y][x].pheromoneA =
                fmax(sim->simGrid[y][x].pheromoneA * 0.999, 0.0);
            sim->simGrid[y][x].pheromoneB =
                fmax(sim->simGrid[y][x].pheromoneB * 0.999, 0.0);
        }
    }

    for (int i = 0; i < N_ANTS; i++) {
        Ant *ant = &sim->ants[i];
        ant->theta += antGetDirectionSignal(sim, sim->ants[i]);
        ant->pheromoneStrenght *= 0.99;

        // walk the ant
        ant->pos.x += cos(ant->theta) * sim->antSpeed;
        ant->pos.y += sin(ant->theta) * sim->antSpeed;
        if (ant->pos.x <= 0) {
            ant->pos.x = 0;
            ant->theta = M_PI - ant->theta;
        } else if (ant->pos.x > SIM_W - 1) {
            ant->pos.x = SIM_W - 1;
            ant->theta = M_PI - ant->theta;
        }
        if (ant->pos.y <= 0) {
            ant->pos.y = 0;
            ant->theta = -ant->theta;
        } else if (ant->pos.y > SIM_H - 1) {
            ant->pos.y = SIM_H - 1;
            ant->theta = -ant->theta;
        }

        // drop pheromone
        if (ant->has_food == false) {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneA +=
                ant->pheromoneStrenght;
        } else {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneB +=
                ant->pheromoneStrenght;
        }

        // si food trouvé
        if (sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].food > 0.0 &&
            ant->has_food == false) {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].food -= 0.05;
            ant->has_food = true;
            ant->pheromoneStrenght = 1.0;
            ant->theta = remainder(ant->theta + M_PI, 2 * M_PI);
        }

        // si food déposé
        if ((ant->pos.x - sim->colony.x) * (ant->pos.x - sim->colony.x) +
                    (ant->pos.y - sim->colony.y) *
                        (ant->pos.y - sim->colony.y) <
                COLONY_RADIUS * COLONY_RADIUS &&
            ant->has_food == true) {
            ant->has_food = false;
            ant->pheromoneStrenght = 1.0;
            ant->theta = remainder(ant->theta + M_PI, 2 * M_PI);
        }
    }
}

void destroySim(Sim *sim) {
    for (int i = 0; i < SIM_H; i++) {
        free(sim->simGrid[i]);
    }
    free(sim->simGrid);
    free(sim->ants);
}

#include "simulation.h"
#include "constants.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    sim->rowLocks = (pthread_mutex_t *)malloc(SIM_H * sizeof(pthread_mutex_t));
    if (sim->rowLocks == NULL)
        return false;
    for (int i = 0; i < SIM_H; i++) {
        pthread_mutex_init(&sim->rowLocks[i], NULL);
    }

    Coord2D foodCenter = {.x = ((float)SIM_W / 4.0) * 3,
                          .y = (float)SIM_H / 2.0};

    // put food on cells
    sim->foodCells = 0;
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            if (isInCircle((Coord2D){x, y}, foodCenter, FOOD_RADIUS)) {
                sim->simGrid[y][x].food = 1.0;
                sim->foodCells++;
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
        sim->ants[i].xorshift32state = rand();
    }

    return true;
}

bool isWall(Sim *sim, Coord2D pos) {
    if (pos.x < 0 || pos.x >= SIM_W || pos.y < 0 || pos.y >= SIM_H ||
        sim->simGrid[(int)pos.y][(int)pos.x].wall) {
        return true;
    } else
        return false;
}

// the ant looks for pheromones in its field of view and goes to the average
// direction
float antGetDirectionSignal(Sim *sim, Ant ant) {
    float directionSignal = 0.0f;
    float repulsion = 0.0f;
    float maxPheromone = 0.0f;

    Coord2D scanStart = {(ant.pos.x - ANT_FOV_RADIUS),
                         (ant.pos.y - ANT_FOV_RADIUS)};
    Coord2D scanStop = {(ant.pos.x + ANT_FOV_RADIUS),
                        (ant.pos.y + ANT_FOV_RADIUS)};

    for (int y = (int)(scanStart.y); y <= (int)(scanStop.y); y++) {
        for (int x = (int)(scanStart.x); x <= (int)(scanStop.x); x++) {
            Coord2D scanPos = {x + 0.5, y + 0.5};
            float distToAnt = dist(scanPos, ant.pos);
            // hors de la distance de vision
            if (!isInCircle(scanPos, ant.pos, ANT_FOV_RADIUS)) {
                continue;
            }
            double cellAngle =
                remainder(atan2f((float)(scanPos.y - ant.pos.y),
                                 (float)(scanPos.x - ant.pos.x)) -
                              ant.theta,
                          2 * M_PI);
            // hors de l'angle de vision
            if (!(cellAngle < ANT_FOV_ANGLE && cellAngle > -ANT_FOV_ANGLE)) {
                continue;
            }

            if (isWall(sim, scanPos)) {
                float force = 1.0f / (distToAnt * distToAnt);
                repulsion += (cellAngle > 0) ? -force : force;
                continue;
            }

            if (ant.has_food) {
                if (isInCircle(scanPos, sim->colony, COLONY_RADIUS)) {
                    return cellAngle;
                } else {
                    if (sim->simGrid[y][x].pheromoneA > maxPheromone) {
                        maxPheromone = sim->simGrid[y][x].pheromoneA;
                        directionSignal = cellAngle;
                    }
                }
            } else {
                if (sim->simGrid[y][x].food > 0.0) {
                    return cellAngle;
                } else {
                    if (sim->simGrid[y][x].pheromoneB > maxPheromone) {
                        maxPheromone = sim->simGrid[y][x].pheromoneB;
                        directionSignal = cellAngle;
                    }
                }
            }
        }
    }

    return directionSignal +
           fclamp(K_MUR * repulsion, -ANT_FOV_ANGLE, ANT_FOV_ANGLE);
}

void nextSimStep(Sim *sim) {
    int foodCells = 0;
#pragma omp parallel for reduction(+ : foodCells)
    for (int y = 0; y < SIM_H; y++) {
        for (int x = 0; x < SIM_W; x++) {
            sim->simGrid[y][x].pheromoneA *= EVAPORATION_RATE;
            sim->simGrid[y][x].pheromoneB *= EVAPORATION_RATE;
            if (sim->simGrid[y][x].pheromoneA <= MIN_PHEROMONE_SENS)
                sim->simGrid[y][x].pheromoneA = 0.0f;
            if (sim->simGrid[y][x].pheromoneB <= MIN_PHEROMONE_SENS)
                sim->simGrid[y][x].pheromoneB = 0.0f;
            if (sim->simGrid[y][x].food > 0.0)
                foodCells++;
        }
    }
    sim->foodCells = foodCells;

#pragma omp parallel for
    for (int i = 0; i < N_ANTS; i++) {
        Ant *ant = &sim->ants[i];
        ant->deriv *= 0.98;
        ant->deriv += randNormalAnt(ant, 0.01);
        ant->theta += antGetDirectionSignal(sim, sim->ants[i]) + ant->deriv;
        ant->pheromoneStrenght *= PHEROMONE_STRENGHT_DEPRECIATION;

        // walk the ant
        float dx = cos(ant->theta) * sim->antSpeed;
        float dy = sin(ant->theta) * sim->antSpeed;

        if (isWall(sim, (Coord2D){ant->pos.x + dx, ant->pos.y})) {
            dx = 0;
        }
        if (isWall(sim, (Coord2D){ant->pos.x, ant->pos.y + dy})) {
            dy = 0;
        }

        ant->pos.x += dx;
        ant->pos.y += dy;

        ant->onFood = sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].food > 0.0;
    }

#pragma omp parallel for
    for (int i = 0; i < N_ANTS; i++) {
        Ant *ant = &sim->ants[i];

        pthread_mutex_lock(&sim->rowLocks[(int)ant->pos.y]);

        // drop pheromone
        if (!ant->has_food) {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneA =
                fmax(ant->pheromoneStrenght,
                     sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneA);
        } else {
            sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneB =
                fmax(ant->pheromoneStrenght,
                     sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].pheromoneB);
        }

        // si food trouvé
        if (ant->onFood) {
            if (!ant->has_food) {
                sim->simGrid[(int)ant->pos.y][(int)ant->pos.x].food -= 0.02;
                ant->has_food = true;
                ant->theta = remainder(ant->theta + M_PI, 2 * M_PI);
            }
            ant->pheromoneStrenght = 1.0;
        }

        pthread_mutex_unlock(&sim->rowLocks[(int)ant->pos.y]);

        // si food déposé
        if (isInCircle(ant->pos, sim->colony, COLONY_RADIUS)) {
            ant->pheromoneStrenght = 1.0;
            if (ant->has_food) {
                ant->has_food = false;
                ant->theta = remainder(ant->theta + M_PI, 2 * M_PI);
            }
        }
    }
}

void copySim(Sim *dst, Sim *src) {
    for (int i = 0; i < SIM_H; i++) {
        memcpy(dst->simGrid[i], src->simGrid[i], SIM_W * sizeof(simCell));
    }
    memcpy(dst->ants, src->ants, N_ANTS * sizeof(Ant));
}

void dumpSim(Sim *sim, const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        printf("Unable to open %s\n", path);
        return;
    }
    for (int i = 0; i < SIM_H; i++) {
        fwrite(sim->simGrid[i], sizeof(simCell), SIM_W, file);
    }
    fwrite(sim->ants, sizeof(Ant), N_ANTS, file);
    fclose(file);
}

void destroySim(Sim *sim) {
    for (int i = 0; i < SIM_H; i++) {
        free(sim->simGrid[i]);
        pthread_mutex_destroy(&sim->rowLocks[i]);
    }
    free(sim->simGrid);
    free(sim->rowLocks);
    free(sim->ants);
}

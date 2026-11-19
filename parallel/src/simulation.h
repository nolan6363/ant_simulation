#ifndef INCLUDEsrcsimulationsimulation
#define INCLUDEsrcsimulationsimulation

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float x, y;
} Coord2D;

typedef struct {
    float pheromoneA, pheromoneB;
    float food;
    bool wall;
    bool debug_flag;
} simCell;

typedef struct {
    Coord2D pos;
    float theta, deriv;
    bool has_food, onFood;
    float pheromoneStrenght;
    uint32_t xorshift32state;
} Ant;

typedef struct {
    bool running, paused;
    Coord2D colony;
    simCell **simGrid;
    simCell **simGridBuffer;
    Ant *ants;
    float antSpeed;
    pthread_mutex_t *rowLocks;
    int foodCells;
} Sim;

bool initSim(Sim *sim);
void nextSimStep(Sim *sim);
void copySim(Sim *dst, Sim *src);
void dumpSim(Sim *sim, const char *path);
void destroySim(Sim *sim);

#endif // INCLUDEsrcsimulationsimulation.h_

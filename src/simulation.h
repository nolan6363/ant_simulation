#ifndef INCLUDEsrcsimulationsimulation
#define INCLUDEsrcsimulationsimulation

#include <stdbool.h>

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
    bool has_food;
    float pheromoneStrenght;
} Ant;

typedef struct {
    bool running, paused;
    Coord2D colony;
    simCell **simGrid;
    Ant *ants;
    float antSpeed;
} Sim;

bool initSim(Sim *sim);
void nextSimStep(Sim *sim);
void destroySim(Sim *sim);

#endif // INCLUDEsrcsimulationsimulation.h_

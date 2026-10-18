#ifndef INCLUDEsrcsimulationsimulation
#define INCLUDEsrcsimulationsimulation

#include <stdbool.h>

typedef struct {
    float x, y;
} Coord2D;

typedef struct {
    float pheromoneA, pheromoneB;
    float food;
} simCell;

typedef struct {
    Coord2D pos;
    float theta;
    bool has_food;
} Ant;

typedef struct {
    int width, height;
    Coord2D colony;
    simCell **simGrid;
    Ant *ants;
    float antSpeed;
} Sim;

bool initSim(Sim *sim);
void nextSimStep(Sim *sim);
void destroySim(Sim *sim);

#endif // INCLUDEsrcsimulationsimulation.h_

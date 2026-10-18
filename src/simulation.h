#ifndef INCLUDEsrcsimulationsimulation
#define INCLUDEsrcsimulationsimulation

#include <stdbool.h>

typedef struct {
    int x, y;
} Coord2D;

typedef struct {
    float pheromoneA, pheromoneB;
    float food;
} simCell;

typedef struct {
    int width, height;
    Coord2D colony;
    simCell **simGrid;
} Sim;

bool initSim(Sim *sim);
void destroySim(Sim *sim);

#endif // INCLUDEsrcsimulationsimulation.h_

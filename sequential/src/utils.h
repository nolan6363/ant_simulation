#ifndef INCLUDEsrcutilsutils
#define INCLUDEsrcutilsutils

#include "simulation.h"
#include <stdint.h>

// float fmin(float a, float b);
int min(int a, int b);
// float fmax(float a, float b);
int max(int a, int b);
float fclamp(float a, float min, float max);
int clamp(int a, int min, int max);
bool isInCircle(Coord2D point, Coord2D center, float radius);
float gaussian(float x, float sigma);
float frandAnt(Ant *ant, float min, float max);
float randNormalAnt(Ant *ant, float sigma);
float dist(Coord2D A, Coord2D B);
float norm(Coord2D B);
uint32_t xorshift32(uint32_t *state);

#endif // INCLUDEsrcutilsutils.h_

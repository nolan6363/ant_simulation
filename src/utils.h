#ifndef INCLUDEsrcutilsutils
#define INCLUDEsrcutilsutils

#include "simulation.h"

// float fmin(float a, float b);
int min(int a, int b);
// float fmax(float a, float b);
int max(int a, int b);
float fclamp(float a, float min, float max);
int clamp(int a, int min, int max);
bool isInCircle(Coord2D point, Coord2D center, float radius);
float gaussian(float x, float sigma);
float frand(float min, float max);
float randNormal(float sigma);
float dist(Coord2D A, Coord2D B);
float norm(Coord2D B);

#endif // INCLUDEsrcutilsutils.h_

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

#endif // INCLUDEsrcutilsutils.h_

#include "utils.h"
#include "simulation.h"
#include <math.h>

int min(int a, int b) { return (a <= b) ? a : b; }
int max(int a, int b) { return (a >= b) ? a : b; }
float fclamp(float a, float min_val, float max_val) {
    return fmin(fmax(a, min_val), max_val);
}
int clamp(int a, int min_val, int max_val) {
    return min(max(a, min_val), max_val);
}

bool isInCircle(Coord2D point, Coord2D center, float radius) {
    float dx_sq = (point.x - center.x) * (point.x - center.x);
    float dy_sq = (point.y - center.y) * (point.y - center.x);
    float r_sq = radius * radius;

    return (dx_sq + dy_sq < r_sq) ? true : false;
}

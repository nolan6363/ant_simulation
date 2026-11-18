#include "utils.h"
#include "simulation.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

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
    float dy_sq = (point.y - center.y) * (point.y - center.y);
    float r_sq = radius * radius;

    return (dx_sq + dy_sq < r_sq) ? true : false;
}

float gaussian(float x, float sigma) {
    float num = -(x * x);
    float den = 2 * sigma * sigma;
    return exp(num / den);
}

float frand(float min, float max) {
    return ((float)rand() / RAND_MAX) * (max - min) + min;
}

float randNormal(float sigma) {
    float u1 = frand(1e-7f, 1.0f);
    float u2 = frand(0.0f, 1.0f);
    return sigma * sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
}

float dist(Coord2D A, Coord2D B) {
    return norm((Coord2D){A.x - B.x, A.y - B.y});
}

float norm(Coord2D B) {
    float x_2 = B.x * B.x;
    float y_2 = B.y * B.y;

    return sqrtf(x_2 + y_2);
}

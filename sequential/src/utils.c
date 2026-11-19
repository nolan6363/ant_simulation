#include "utils.h"
#include "simulation.h"
#include <math.h>
#include <stdint.h>
#include <sys/types.h>

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

float frandAnt(Ant *ant, float min, float max) {
    return ((float)xorshift32(&ant->xorshift32state) / UINT32_MAX) *
               (max - min) +
           min;
}

float randNormalAnt(Ant *ant, float sigma) {
    float u1 = frandAnt(ant, 1e-7f, 1.0f);
    float u2 = frandAnt(ant, 0.0f, 1.0f);
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

// from https://en.wikipedia.org/wiki/Xorshift
// The state must be initialized to non-zero
uint32_t xorshift32(uint32_t *state) {
    // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}

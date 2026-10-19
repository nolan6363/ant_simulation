#include "utils.h"
#include <math.h>

int min(int a, int b) { return (a <= b) ? a : b; }
int max(int a, int b) { return (a >= b) ? a : b; }
float fclamp(float a, float min_val, float max_val) {
    return fmin(fmax(a, min_val), max_val);
}
int clamp(int a, int min_val, int max_val) {
    return min(max(a, min_val), max_val);
}

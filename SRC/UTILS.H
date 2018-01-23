#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

static float f_clamp(float value, float min, float max) {
    if(value > max)
        return max;
    if(value < min)
        return min;

    return value;
}

static int i_clamp(int value, int min, int max) {
    if(value > max)
        return max;
    if(value < min)
        return min;

    return value;
}

static float remap(float value, float oldmin, float oldmax, float newmin, float newmax) {
    float real_range = oldmax - oldmin;
    float new_range = newmax - newmin;
    return (((value - oldmin) * new_range) / real_range) + newmin;
}

static int i_range_rand(int min, int max) {
    return rand()%(max-min)+min;
}

static float f_range_rand(float min, float max) {
    return 0.0001 * (float)i_range_rand((int)(min * 10000), (int)(max * 10000));
}

#endif /* UTILS_H */

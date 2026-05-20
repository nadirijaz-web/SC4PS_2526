#include "rng.h"

void lcg32_seed(Lcg32 *rng, uint32_t seed) {
    rng->state = seed;
}

uint32_t lcg32_next_u32(Lcg32 *rng) {
    const uint32_t a = 1664525u;
    const uint32_t c = 1013904223u;

    rng->state = a * rng->state + c;
    return rng->state;
}

double lcg32_next_double(Lcg32 *rng) {
    return lcg32_next_u32(rng) / 4294967296.0;
}

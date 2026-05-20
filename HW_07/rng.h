#ifndef RNG_H
#define RNG_H

#include <stdint.h>

typedef struct {
    uint32_t state;
} Lcg32;

void lcg32_seed(Lcg32 *rng, uint32_t seed);
uint32_t lcg32_next_u32(Lcg32 *rng);
double lcg32_next_double(Lcg32 *rng);

#endif

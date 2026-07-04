// rng_seed_impl.h -- SplitMix64 implementation.

#ifndef __RNG_SEED_IMPL_H__
#define __RNG_SEED_IMPL_H__

#include <stdint.h>
typedef uint64_t rng_seed_t;

rng_seed_t rng_seed_impl_next(rng_seed_t*);

#endif

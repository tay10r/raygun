#pragma once

#include <stdint.h>

struct rg_random
{
  uint32_t state;
};

#ifdef RG_RANDOM_IMPL

static inline float
rg_random_floatbits(uint32_t in)
{
  union
  {
    uint32_t i;
    float f;
  } v;

  v.i = in;

  return v.f;
}

static inline void
rg_random_init(struct rg_random* self, const uint32_t state)
{
  self->state = state;
}

static inline uint32_t
rg_random_int(struct rg_random* self)
{
  self->state = self->state * 747796405u + 2891336453u;
  const uint32_t word = ((self->state >> ((self->state >> 28u) + 4u)) ^ self->state) * 277803737u;
  return (word >> 22u) ^ word;
}

static inline float
rg_random_float(struct rg_random* self)
{
  const uint32_t value = rg_random_int(self);
  return rg_random_floatbits((value & 0x007fffff) | 0x3f800000) - 1.0f;
}

#endif /* RG_RANDOM_IMPL */

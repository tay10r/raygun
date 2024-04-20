#pragma once

#include <stdint.h>

struct rg_pipeline;
struct rg_random;

struct rg_pipeline*
rg_pipeline_new(int w, int h);

void
rg_pipeline_delete(struct rg_pipeline* self);

float*
rg_pipeline_color_buffer(struct rg_pipeline* self);

struct rg_random*
rg_pipeline_random_buffer(struct rg_pipeline* self);

void
rg_pipeline_size(struct rg_pipeline* self, int* w, int* h);

void
rg_pipeline_sync_textures(struct rg_pipeline* self);

void
rg_pipeline_bind_textures(struct rg_pipeline* self, int texture_unit_offset);

void
rg_pipeline_next_frame(struct rg_pipeline* self);

uint32_t
rg_pipeline_frame_index(const struct rg_pipeline* self);

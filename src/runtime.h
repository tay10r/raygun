#pragma once

#include <raygun.h>

struct rg_runtime;

struct rg_runtime*
rg_runtime_new(void* caller_data,
               const struct raygun_interface* interface,
               const char* window_title,
               const char* embree_config);

void
rg_runtime_delete(struct rg_runtime* self);

/**
 * @brief Iterates the pipeline by one frame.
 *
 * @param should_close Whether or not the user has requested a shutdown.
 * */
void
rg_runtime_iterate(struct rg_runtime* self, int* should_close);

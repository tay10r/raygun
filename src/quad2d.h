#pragma once

struct rg_quad2d;

struct rg_quad2d*
rg_quad2d_new(void);

void
rg_quad2d_delete(struct rg_quad2d* self);

/**
 * @brief Draws the quad.
 *
 * @param self The quad instance to draw.
 * */
void
rg_quad2d_draw(const struct rg_quad2d* self);

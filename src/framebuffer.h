#pragma once

#include <glad/glad.h>

struct rg_framebuffer;

struct rg_framebuffer*
rg_framebuffer_new(int w, int h);

void
rg_framebuffer_delete(struct rg_framebuffer* self);

void
rg_framebuffer_bind(const struct rg_framebuffer* self);

GLuint
rg_framebuffer_texture(const struct rg_framebuffer* self);

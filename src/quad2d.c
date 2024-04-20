#include "quad2d.h"

#include <glad/glad.h>

#include <stdlib.h>

struct rg_quad2d
{
  GLuint buffer;

  GLuint vertex_array;
};

struct rg_quad2d*
rg_quad2d_new(void)
{
  struct rg_quad2d* self = malloc(sizeof(struct rg_quad2d));
  if (!self) {
    return NULL;
  }

  glGenVertexArrays(1, &self->vertex_array);

  glBindVertexArray(self->vertex_array);

  glGenBuffers(1, &self->buffer);

  glBindBuffer(GL_ARRAY_BUFFER, self->buffer);

  const float data[12] = {
    // clang-format off
    0, 0,
    0, 1,
    1, 1,
    1, 1,
    1, 0,
    0, 0,
    // clang-format on
  };

  glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(data), data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

  return self;
}

void
rg_quad2d_delete(struct rg_quad2d* self)
{
  if (self) {
    glDeleteVertexArrays(1, &self->vertex_array);

    glDeleteBuffers(1, &self->buffer);
  }

  free(self);
}

void
rg_quad2d_draw(const struct rg_quad2d* self)
{
  glBindVertexArray(self->vertex_array);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

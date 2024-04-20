#include "framebuffer.h"

#include <stdlib.h>
#include <string.h>

struct rg_framebuffer
{
  GLuint id;

  GLuint texture;
};

struct rg_framebuffer*
rg_framebuffer_new(int w, int h)
{
  struct rg_framebuffer* self = malloc(sizeof(struct rg_framebuffer));
  if (!self) {
    return NULL;
  }

  memset(self, 0, sizeof(struct rg_framebuffer));

  glGenTextures(1, &self->texture);

  glBindTexture(GL_TEXTURE_2D, self->texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_FLOAT, NULL);

  glGenFramebuffers(1, &self->id);

  glBindFramebuffer(GL_FRAMEBUFFER, self->id);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->texture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    glDeleteTextures(1, &self->texture);
    glDeleteFramebuffers(1, &self->id);
    free(self);
    return NULL;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return self;
}

void
rg_framebuffer_delete(struct rg_framebuffer* self)
{
  if (self) {

    glDeleteFramebuffers(1, &self->id);

    glDeleteTextures(1, &self->texture);
  }

  free(self);
}

void
rg_framebuffer_bind(const struct rg_framebuffer* self)
{
  glBindFramebuffer(GL_FRAMEBUFFER, self->id);
}

GLuint
rg_framebuffer_texture(const struct rg_framebuffer* self)
{
  return self->texture;
}

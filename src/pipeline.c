#include "pipeline.h"

#include "framebuffer.h"
#include "random.h"

#include <glad/glad.h>

#include <stdlib.h>
#include <string.h>

struct rg_pipeline
{
  int width;

  int height;

  float* color_buffer;

  struct rg_random* random_buffer;

  GLuint textures[3];

  int textures_allocated;

  struct rg_framebuffer* accumulate_fb[2];

  struct rg_framebuffer* tone_fb;

  uint32_t frame_index;
};

#if 0
static struct rg_framebuffer*
rg_get_write_framebuffer(struct rg_pipeline* self, struct rg_framebuffer** framebuffers)
{
  const int index = self->frame_index % 2;
  return framebuffers[index];
}
#endif

static struct rg_framebuffer*
rg_get_read_framebuffer(struct rg_pipeline* self, struct rg_framebuffer** framebuffers)
{
  const int index = (self->frame_index + 1) % 2;
  return framebuffers[index];
}

struct rg_pipeline*
rg_pipeline_new(int w, int h)
{
  struct rg_pipeline* self = malloc(sizeof(struct rg_pipeline));
  if (!self) {
    return NULL;
  }

  memset(self, 0, sizeof(struct rg_pipeline));

  self->width = w;

  self->height = h;

  self->color_buffer = malloc(sizeof(float) * (size_t)w * (size_t)h * 3u);

  if (!self->color_buffer) {
    free(self);
    return NULL;
  }

  self->random_buffer = malloc(sizeof(struct rg_random) * (size_t)w * (size_t)h);
  if (!self->random_buffer) {
    free(self->color_buffer);
    free(self);
    return NULL;
  }

  for (int i = 0; i < (w * h); i++) {
    self->random_buffer[i].state = (uint32_t)i;
  }

  glGenTextures(3, self->textures);

  self->textures_allocated = 1;

  for (int i = 0; i < 3; i++) {

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_FLOAT, NULL);
  }

  self->accumulate_fb[0] = rg_framebuffer_new(w, h);
  if (!self->accumulate_fb[0]) {
    rg_pipeline_delete(self);
    return NULL;
  }

  self->accumulate_fb[1] = rg_framebuffer_new(w, h);
  if (!self->accumulate_fb[1]) {
    rg_pipeline_delete(self);
    return NULL;
  }

  self->tone_fb = rg_framebuffer_new(w, h);
  if (!self->tone_fb) {
    rg_pipeline_delete(self);
    return NULL;
  }

  return self;
}

void
rg_pipeline_delete(struct rg_pipeline* self)
{
  if (self) {

    free(self->color_buffer);

    free(self->random_buffer);

    if (self->textures_allocated) {
      glDeleteTextures(3, self->textures);
    }

    rg_framebuffer_delete(self->accumulate_fb[0]);
    rg_framebuffer_delete(self->accumulate_fb[1]);

    rg_framebuffer_delete(self->tone_fb);
  }

  free(self);
}

struct rg_random*
rg_pipeline_random_buffer(struct rg_pipeline* self)
{
  return self->random_buffer;
}

float*
rg_pipeline_color_buffer(struct rg_pipeline* self)
{
  return self->color_buffer;
}

void
rg_pipeline_size(struct rg_pipeline* self, int* w, int* h)
{
  *w = self->width;
  *h = self->height;
}

void
rg_pipeline_sync_textures(struct rg_pipeline* self)
{
  float* ptr = self->color_buffer;

  const int stride = self->width * self->height;

  for (int i = 0; i < 3; i++) {

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self->width, self->height, GL_ALPHA, GL_FLOAT, ptr);

    ptr += stride;
  }
}

void
rg_pipeline_bind_textures(struct rg_pipeline* self, const int texture_unit_offset)
{
  for (int i = 0; i < 3; i++) {

    glActiveTexture((GLenum)(GL_TEXTURE0 + i + texture_unit_offset));

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);
  }

  glActiveTexture((GLenum)(GL_TEXTURE0 + 3));

  glBindTexture(GL_TEXTURE_2D, rg_framebuffer_texture(rg_get_read_framebuffer(self, self->accumulate_fb)));
}

void
rg_pipeline_next_frame(struct rg_pipeline* self)
{
  self->frame_index += 1;
}

uint32_t
rg_pipeline_frame_index(const struct rg_pipeline* self)
{
  return self->frame_index;
}

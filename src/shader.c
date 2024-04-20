#include "shader.h"

#include <stdlib.h>
#include <string.h>

static char rg_shader_oom[] = "Failed to allocate log data.";

static char*
compile_shader(GLuint shader, const char* src)
{
  GLint length = (GLint)strlen(src);

  glShaderSource(shader, 1, &src, &length);

  glCompileShader(shader);

  GLint status = 0;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status == GL_TRUE) {
    return NULL;
  }

  GLint log_length = 0;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

  if (log_length < 0) {
    log_length = 0;
  }

  char* log = malloc((size_t)log_length + 1);
  if (!log) {
    return rg_shader_oom;
  }

  GLsizei read_size = 0;

  glGetShaderInfoLog(shader, (GLsizei)log_length, &read_size, log);

  log[read_size] = 0;

  return log;
}

struct rg_shader
{
  GLuint id;
};

struct rg_shader*
rg_shader_new(void)
{
  struct rg_shader* self = malloc(sizeof(struct rg_shader));
  if (!self) {
    return NULL;
  }

  self->id = glCreateProgram();

  return self;
}

void
rg_shader_delete(struct rg_shader* self)
{
  if (self) {
    glDeleteProgram(self->id);
  }

  free(self);
}

char*
rg_shader_setup(struct rg_shader* self, const char* vert_source, const char* frag_source)
{
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);

  char* err = compile_shader(vert_shader, vert_source);
  if (err != NULL) {
    glDeleteShader(vert_shader);
    return err;
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

  err = compile_shader(frag_shader, frag_source);
  if (err != NULL) {
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return err;
  }

  glAttachShader(self->id, vert_shader);
  glAttachShader(self->id, frag_shader);

  glLinkProgram(self->id);

  glDetachShader(self->id, vert_shader);
  glDetachShader(self->id, frag_shader);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  GLint status = 0;

  glGetProgramiv(self->id, GL_LINK_STATUS, &status);

  if (status == GL_TRUE) {
    return NULL;
  }

  GLint log_length = 0;

  glGetProgramiv(self->id, GL_INFO_LOG_LENGTH, &log_length);

  if (log_length < 0) {
    log_length = 0;
  }

  char* log = malloc((size_t)log_length + 1);
  if (!log) {
    return rg_shader_oom;
  }

  GLsizei read_size = 0;

  glGetProgramInfoLog(self->id, (GLsizei)log_length, &read_size, log);

  log[read_size] = 0;

  return log;
}

GLuint
rg_shader_id(const struct rg_shader* self)
{
  return self->id;
}

void
rg_shader_log_free(char* ptr)
{
  if (ptr != rg_shader_oom) {
    free(ptr);
  }
}

GLint
rg_shader_uniform(const struct rg_shader* self, const char* name)
{
  return glGetUniformLocation(self->id, name);
}

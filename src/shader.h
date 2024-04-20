#pragma once

#include <glad/glad.h>

struct rg_shader;

struct rg_shader*
rg_shader_new(void);

void
rg_shader_delete(struct rg_shader* self);

/**
 * @brief Compiles and links the shader.
 *
 * @param vert_source The source code of the vertex shader.
 *
 * @param frag_source The source code of the fragment shader.
 *
 * @param cb_data The data to pass to the callback function.
 *
 * @return On success, a null pointer is returned. On failure, a pointer to an error log is returned (which must be
 *         freed when it is no longer needed.
 * */
char*
rg_shader_setup(struct rg_shader* self, const char* vert_source, const char* frag_source);

/**
 * @brief Gets the ID of the shader program.
 *
 * @return The ID of the shader program.
 * */
GLuint
rg_shader_id(const struct rg_shader* self);

void
rg_shader_log_free(char* ptr);

GLint
rg_shader_uniform(const struct rg_shader* self, const char* name);

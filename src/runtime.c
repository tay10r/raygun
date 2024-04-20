#include "runtime.h"

#define RG_RANDOM_IMPL

#include "pipeline.h"
#include "quad2d.h"
#include "random.h"
#include "shader.h"

// generated
#include "shaders.h"

#include <GLFW/glfw3.h>

#include <glad/glad.h>

#include <embree3/rtcore.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

struct accumulate_shader_info
{
  GLint r_location;

  GLint g_location;

  GLint b_location;

  GLint prev_location;
};

struct rg_runtime
{
  void* caller_data;

  const struct raygun_interface* interface;

  GLFWwindow* window;

  RTCDevice device;

  RTCScene scene;

  struct rg_quad2d* quad;

  struct rg_shader* accumulate_shader;

  struct rg_shader* tone_shader;

  struct rg_pipeline* pipeline;

  struct accumulate_shader_info accumulate_shader_info;

  struct raygun_camera camera;

  int should_close;
};

static struct rg_runtime*
get_runtime(GLFWwindow* window)
{
  return (struct rg_runtime*)glfwGetWindowUserPointer(window);
}

static void
notify_error(struct rg_runtime* self, const char* msg)
{
  if (self->interface->error) {
    self->interface->error(self->caller_data, msg);
  }
}

static void
on_glfw_key(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
{
  struct rg_runtime* rt = get_runtime(window);

  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
    rt->should_close = 1;
  }

  (void)scancode;
  (void)mods;
}

static void
setup_accumulate_shader(struct rg_runtime* self)
{
  self->accumulate_shader_info.r_location = rg_shader_uniform(self->accumulate_shader, "r_texture");
  self->accumulate_shader_info.g_location = rg_shader_uniform(self->accumulate_shader, "g_texture");
  self->accumulate_shader_info.b_location = rg_shader_uniform(self->accumulate_shader, "b_texture");
  self->accumulate_shader_info.prev_location = rg_shader_uniform(self->accumulate_shader, "previous");
}

struct rg_runtime*
rg_runtime_new(void* caller,
               const struct raygun_interface* interface,
               const char* window_title,
               const char* embree_config)
{
  if (glfwInit() == GLFW_FALSE) {
    if (interface->error) {
      interface->error(caller, "Failed to initialize GLFW.");
    }
    return NULL;
  }

  struct rg_runtime* self = malloc(sizeof(struct rg_runtime));
  if (!self) {
    if (interface->error) {
      interface->error(caller, "Failed to allocate memory for runtime object.");
    }
    return NULL;
  }

  memset(self, 0, sizeof(struct rg_runtime));

  self->caller_data = caller;
  self->interface = interface;

  self->should_close = 0;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

  const int init_w = 640;
  const int init_h = 480;

  self->window = glfwCreateWindow(init_w, init_h, window_title, NULL, NULL);
  if (self->window == NULL) {
    notify_error(self, "Failed to create GLFW window.");
    glfwTerminate();
    free(self);
    return NULL;
  }

  glfwMakeContextCurrent(self->window);

  gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);

  glfwSwapInterval(1);

  glfwSetWindowUserPointer(self->window, self);

  glfwSetKeyCallback(self->window, on_glfw_key);

  self->device = rtcNewDevice(embree_config);
  if (!self->device) {
    notify_error(self, "Failed to create Embree device.");
    rg_runtime_delete(self);
    return NULL;
  }

  self->scene = rtcNewScene(self->device);
  if (!self->scene) {
    notify_error(self, "Failed to create Embree scene.");
    rg_runtime_delete(self);
    return NULL;
  }

  self->quad = rg_quad2d_new();
  if (!self->quad) {
    notify_error(self, "Failed to create OpenGL quad.");
    rg_runtime_delete(self);
    return NULL;
  }

  self->accumulate_shader = rg_shader_new();
  if (!self->accumulate_shader) {
    notify_error(self, "Failed to create accumulate shader.");
    rg_runtime_delete(self);
    return NULL;
  }

  self->tone_shader = rg_shader_new();
  if (!self->tone_shader) {
    notify_error(self, "Failed to create tone shader.");
    rg_runtime_delete(self);
    return NULL;
  }

  char* shader_err = NULL;

  shader_err = rg_shader_setup(self->accumulate_shader, rg_shaders_quad_vert, rg_shaders_accumulate_frag);
  if (shader_err) {
    notify_error(self, shader_err);
    rg_shader_log_free(shader_err);
    rg_runtime_delete(self);
    return NULL;
  }

  shader_err = rg_shader_setup(self->tone_shader, rg_shaders_quad_vert, rg_shaders_tone_frag);
  if (shader_err) {
    notify_error(self, shader_err);
    rg_shader_log_free(shader_err);
    rg_runtime_delete(self);
    return NULL;
  }

  self->pipeline = rg_pipeline_new(init_w, init_h);
  if (!self->pipeline) {
    notify_error(self, "Failed to allocate pipeline.");
    rg_runtime_delete(self);
    return NULL;
  }

  setup_accumulate_shader(self);

  if (interface->setup) {
    interface->setup(caller, self->device, self->scene);
  }

  return self;
}

void
rg_runtime_delete(struct rg_runtime* self)
{
  if (self) {

    if (self->interface->teardown) {
      self->interface->teardown(self->caller_data, self->device, self->scene);
    }

    if (self->pipeline) {
      rg_pipeline_delete(self->pipeline);
    }

    if (self->accumulate_shader) {
      rg_shader_delete(self->accumulate_shader);
    }

    if (self->tone_shader) {
      rg_shader_delete(self->tone_shader);
    }

    if (self->quad) {
      rg_quad2d_delete(self->quad);
    }

    if (self->scene) {
      rtcReleaseScene(self->scene);
    }

    if (self->device) {
      rtcReleaseDevice(self->device);
    }

    if (self->window) {

      glfwDestroyWindow(self->window);

      glfwTerminate();
    }
  }

  free(self);
}

static void
rg_runtime_render(struct rg_runtime* self)
{
  int w = 0;
  int h = 0;
  rg_pipeline_size(self->pipeline, &w, &h);

  const int num_pixels = w * h;
  const float x_scale = 1.0f / ((float)w);
  const float y_scale = 1.0f / ((float)h);

  struct rg_random* rng_buffer = rg_pipeline_random_buffer(self->pipeline);

  float* r_ptr = rg_pipeline_color_buffer(self->pipeline);
  float* g_ptr = r_ptr + w * h;
  float* b_ptr = g_ptr + w * h;

#pragma omp parallel for

  for (int i = 0; i < num_pixels; i++) {

    const int x = i % w;
    const int y = i / w;

    const float u = (((float)x) + rg_random_float(&rng_buffer[i])) * x_scale;
    const float v = (((float)y) + rg_random_float(&rng_buffer[i])) * y_scale;

    const float dx = u * 2.0f - 1.0f;
    const float dy = v * 2.0f - 1.0f;
    const float dz = -1.0f;

    const float rcp_mag = 1.0f / sqrtf(dx * dx + dy * dy + dz * dz);

    const float ndx = dx * rcp_mag;
    const float ndy = dy * rcp_mag;
    const float ndz = dz * rcp_mag;

    struct RTCRayHit ray_hit;

    ray_hit.ray.flags = 0;
    ray_hit.ray.id = 0;
    ray_hit.ray.mask = ~0u;

    ray_hit.ray.tnear = 0;
    ray_hit.ray.tfar = 1000;

    ray_hit.ray.org_x = self->camera.pos[0];
    ray_hit.ray.org_y = self->camera.pos[1];
    ray_hit.ray.org_z = self->camera.pos[2];

    ray_hit.ray.dir_x = ndx;
    ray_hit.ray.dir_y = ndy;
    ray_hit.ray.dir_z = ndz;

    ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    ray_hit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    struct RTCIntersectContext context;

    rtcInitIntersectContext(&context);

    rtcIntersect1(self->scene, &context, &ray_hit);

    self->interface->trace(self->caller_data, self->scene, 1, &ray_hit, r_ptr + i, g_ptr + i, b_ptr + i);
  }
}

static void
rg_add_previous_render(struct rg_runtime* self)
{
  glUseProgram(rg_shader_id(self->accumulate_shader));

  glUniform1i(self->accumulate_shader_info.r_location, 0);
  glUniform1i(self->accumulate_shader_info.g_location, 1);
  glUniform1i(self->accumulate_shader_info.b_location, 2);
  glUniform1i(self->accumulate_shader_info.prev_location, 3);

  rg_quad2d_draw(self->quad);
}

void
rg_runtime_iterate(struct rg_runtime* self, int* should_close)
{
  glfwPollEvents();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  rg_runtime_render(self);

  rg_pipeline_sync_textures(self->pipeline);

  rg_pipeline_bind_textures(self->pipeline, 0);

  rg_add_previous_render(self);

  self->should_close = glfwWindowShouldClose(self->window) ? 1 : self->should_close;

  *should_close = self->should_close;

  glfwSwapBuffers(self->window);

  rg_pipeline_next_frame(self->pipeline);
}

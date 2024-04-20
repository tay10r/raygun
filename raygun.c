#include "raygun.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "src/quad2d.h"

#define DEF_WIDTH 640

#define DEF_HEIGHT 480

#include <stdlib.h>
#include <string.h>

static const char vert_shader_src[] = "#version 100\n"
                                      "\n"
                                      "in vec2 position;\n"
                                      "\n"
                                      "varying vec2 texcoords;\n"
                                      "\n"
                                      "void\n"
                                      "main()\n"
                                      "{\n"
                                      "  texcoords = position;\n"
                                      "  gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);\n"
                                      "}\n";

static const char frag_shader_src[] = "#version 100\n"
                                      "\n"
                                      "uniform sampler2D r_texture;\n"
                                      "\n"
                                      "uniform sampler2D g_texture;\n"
                                      "\n"
                                      "uniform sampler2D b_texture;\n"
                                      "\n"
                                      "varying highp vec2 texcoords;\n"
                                      "\n"
                                      "void\n"
                                      "main()\n"
                                      "{\n"
                                      "  lowp float r = texture2D(r_texture, texcoords).a;\n"
                                      "  lowp float g = texture2D(g_texture, texcoords).a;\n"
                                      "  lowp float b = texture2D(b_texture, texcoords).a;\n"
                                      "  gl_FragColor = vec4(r, g, b, 1.0);\n"
                                      "}\n";

static void
notify_error(void* callback_data, const struct raygun_interface* interface, const char* what)
{
  if (interface->error) {
    interface->error(callback_data, what);
  }
}

struct key_state
{
  int is_pressed;

  int is_repeated;
};

static void
update_key_state(GLFWwindow* window, struct key_state* s, const int key)
{
  const int state = glfwGetKey(window, key);

  if (state == GLFW_PRESS) {
    if (s->is_pressed) {
      s->is_repeated = 1;
    }
    s->is_pressed = 1;
  } else {
    s->is_pressed = 0;
    s->is_repeated = 0;
  }
}

struct session
{
  void* callback_data;

  const struct raygun_interface* interface;

  struct raygun_camera camera;

  float* color_buffer;

  float* r_ptr;

  float* g_ptr;

  float* b_ptr;

  int fb_width;

  int fb_height;

  GLuint textures[3];

  GLuint shader_program;

  GLint pos_location;

  GLint r_location;

  GLint g_location;

  GLint b_location;

  struct rg_quad2d* quad;
};

static void
session_init(struct session* self, void* callback_data, const struct raygun_interface* interface)
{
  self->camera.pos[0] = 0.0f;
  self->camera.pos[1] = 0.0f;
  self->camera.pos[2] = 1.0f;

  self->camera.dir[0] = 0.0f;
  self->camera.dir[1] = 1.0f;
  self->camera.dir[2] = 0.0f;

  self->camera.tnear = 0.0f;
  self->camera.tfar = 1.0e5f;

  self->color_buffer = NULL;
  self->r_ptr = NULL;
  self->g_ptr = NULL;
  self->b_ptr = NULL;

  self->fb_width = 0;
  self->fb_height = 0;

  self->callback_data = callback_data;
  self->interface = interface;

  glGenTextures(3, self->textures);

  for (GLenum i = 0; i < 3; i++) {

    glActiveTexture(GL_TEXTURE0 + i);

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  self->shader_program = glCreateProgram();

  self->r_location = -1;
  self->g_location = -1;
  self->b_location = -1;

  self->pos_location = -1;
}

static void
session_shutdown(struct session* self)
{
  glDeleteProgram(self->shader_program);

  glDeleteTextures(3, self->textures);

  free(self->color_buffer);
}

static int
session_compile_shader(struct session* self, GLuint shader, const char* source)
{
  GLint len = (GLint)strlen(source);

  glShaderSource(shader, 1, &source, &len);

  glCompileShader(shader);

  GLint status = 0;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status == GL_TRUE) {
    return 0;
  }

  GLint log_length = 0;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

  if (log_length < 0) {
    notify_error(self->callback_data, self->interface, "Shader log length is negative.");
    return -1;
  }

  char* log = malloc((size_t)log_length + 1);
  if (!log) {
    notify_error(self->callback_data, self->interface, "Failed to allocate shader log.");
    return -1;
  }

  GLsizei read_size = 0;

  glGetShaderInfoLog(shader, log_length, &read_size, log);

  if (read_size < 0) {
    notify_error(self->callback_data, self->interface, "Shader log read size is negative.");
    return -1;
  }

  const GLsizei end = (read_size > log_length) ? log_length : read_size;

  log[end] = 0;

  notify_error(self->callback_data, self->interface, log);

  free(log);

  return -1;
}

static int
session_setup_shader_program(struct session* self)
{
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

  if (session_compile_shader(self, vert_shader, vert_shader_src) != 0) {
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return -1;
  }

  if (session_compile_shader(self, frag_shader, frag_shader_src) != 0) {
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return -1;
  }

  glAttachShader(self->shader_program, vert_shader);
  glAttachShader(self->shader_program, frag_shader);

  glLinkProgram(self->shader_program);

  glDetachShader(self->shader_program, vert_shader);
  glDetachShader(self->shader_program, frag_shader);

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  self->r_location = glGetUniformLocation(self->shader_program, "r_texture");
  self->g_location = glGetUniformLocation(self->shader_program, "g_texture");
  self->b_location = glGetUniformLocation(self->shader_program, "b_texture");

  self->pos_location = glGetAttribLocation(self->shader_program, "position");

  return 0;
}

static int
session_resize_frame(struct session* self, int w, int h)
{
  free(self->color_buffer);

  self->r_ptr = NULL;
  self->g_ptr = NULL;
  self->b_ptr = NULL;

  self->fb_width = 0;
  self->fb_height = 0;

  self->color_buffer = calloc((size_t)w * (size_t)h * 3, sizeof(float));
  if (!self->color_buffer) {
    notify_error(self->callback_data, self->interface, "Failed to allocate color buffer.");
    return -1;
  }

  self->r_ptr = self->color_buffer;
  self->g_ptr = self->color_buffer + w * h;
  self->b_ptr = self->color_buffer + w * h * 2;

  self->fb_width = w;
  self->fb_height = h;

  for (GLenum i = 0; i < 3; i++) {

    glActiveTexture(GL_TEXTURE0 + i);

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_FLOAT, NULL);
  }

  return 0;
}

static void
session_sync_textures(struct session* self)
{
  float* ptrs[3] = { self->r_ptr, self->g_ptr, self->b_ptr };

  for (GLenum i = 0; i < 3; i++) {

    glActiveTexture(GL_TEXTURE0 + i);

    glBindTexture(GL_TEXTURE_2D, self->textures[i]);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self->fb_width, self->fb_height, GL_ALPHA, GL_FLOAT, ptrs[i]);
  }
}

static void
session_render(struct session* self, RTCDevice device, RTCScene scene)
{
  const int num_pixels = self->fb_width * self->fb_height;

  float* r = self->r_ptr;
  float* g = self->g_ptr;
  float* b = self->b_ptr;

#pragma omp parallel for

  for (int i = 0; i < num_pixels; i++) {

    struct RTCRayHit ray_hit;

    ray_hit.ray.org_x = self->camera.pos[0];
    ray_hit.ray.org_y = self->camera.pos[1];
    ray_hit.ray.org_z = self->camera.pos[2];

    ray_hit.ray.dir_x = 0;
    ray_hit.ray.dir_y = 0;
    ray_hit.ray.dir_z = 0;

    ray_hit.ray.flags = 0;
    ray_hit.ray.id = 0;
    ray_hit.ray.mask = ~0u;
    ray_hit.ray.time = 0;
    ray_hit.ray.tnear = self->camera.tfar;
    ray_hit.ray.tfar = self->camera.tfar;

    struct RTCIntersectContext ctx;

    rtcInitIntersectContext(&ctx);

    rtcIntersect1(scene, &ctx, &ray_hit);

    self->interface->trace(self->callback_data, device, scene, &ray_hit, r + i, g + i, b + i);
  }
}

void
raygun_exec(void* callback_data,
            const struct raygun_interface* interface,
            const char* window_title,
            const char* embree_config)
{
  if (glfwInit() != GLFW_TRUE) {
    notify_error(callback_data, interface, "Failed to initialize GLFW.");
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

  GLFWwindow* window = glfwCreateWindow(DEF_WIDTH, DEF_HEIGHT, window_title, NULL, NULL);
  if (!window) {
    notify_error(callback_data, interface, "Failed to create GLFW window.");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window);

  gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress);

  glClearColor(0, 0, 0, 1);

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();

  const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);

  struct session session;

  session_init(&session, callback_data, interface);

  if (session_resize_frame(&session, DEF_WIDTH, DEF_HEIGHT) != 0) {
    session_shutdown(&session);
    glfwDestroyWindow(window);
    glfwTerminate();
    return;
  }

  if (session_setup_shader_program(&session) != 0) {
    session_shutdown(&session);
    glfwDestroyWindow(window);
    glfwTerminate();
    return;
  }

  struct key_state f_key_state = { 0, 0 };

  int is_fullscreen = 0;

  RTCDevice device = rtcNewDevice(embree_config);

  RTCScene scene = rtcNewScene(device);

  if (interface->setup) {
    interface->setup(callback_data, device, scene);
  }

#if 0
  const int r16 = rtcGetDeviceProperty(device, RTC_DEVICE_PROPERTY_NATIVE_RAY16_SUPPORTED);
  const int r8 = rtcGetDeviceProperty(device, RTC_DEVICE_PROPERTY_NATIVE_RAY8_SUPPORTED);
  const int r4 = rtcGetDeviceProperty(device, RTC_DEVICE_PROPERTY_NATIVE_RAY4_SUPPORTED);
#endif

  while (!glfwWindowShouldClose(window)) {

    glfwPollEvents();

    if (interface->frame) {
      interface->frame(callback_data, device, scene);
    }

    if (interface->trace) {
      session_render(&session, device, scene);
    }

    session_sync_textures(&session);

    glClear(GL_COLOR_BUFFER_BIT);

    int w = 0;
    int h = 0;
    glfwGetFramebufferSize(window, &w, &h);

    glViewport(0, 0, w, h);

    glfwSwapBuffers(window);

    /* handle input */

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    update_key_state(window, &f_key_state, GLFW_KEY_F);

    if (f_key_state.is_pressed && !f_key_state.is_repeated) {
      if (is_fullscreen) {
        glfwSetWindowMonitor(window, NULL, 16, 16, DEF_WIDTH, DEF_HEIGHT, video_mode->refreshRate);
      } else {
        glfwSetWindowMonitor(window, monitor, 0, 0, video_mode->width, video_mode->height, video_mode->refreshRate);
      }
      is_fullscreen = !is_fullscreen;
    }
  }

  if (interface->shutdown) {
    interface->shutdown(callback_data, device, scene);
  }

  session_shutdown(&session);

  glfwDestroyWindow(window);

  glfwTerminate();
}

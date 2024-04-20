/**
 * @brief file raygun.h
 * */

#pragma once

#include <embree3/rtcore.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * */
  struct raygun_camera
  {
    float pos[3];

    float dir[3];

    float tnear;

    float tfar;
  };

  struct raygun_interface
  {
    void (*setup)(void* caller, RTCDevice device, RTCScene scene);

    void (*teardown)(void* caller, RTCDevice device, RTCScene scene);

    void (*frame)(void* caller, RTCDevice device, RTCScene scene, struct raygun_camera* camera);

    void (*trace)(void* caller,
                  RTCScene scene,
                  uint32_t num_rays,
                  const struct RTCRayHit* ray,
                  float* r,
                  float* g,
                  float* b);

    void (*trace4)(void* caller,
                   RTCScene scene,
                   uint32_t num_rays,
                   const struct RTCRayHit4* ray,
                   float* r,
                   float* g,
                   float* b);

    void (*trace8)(void* caller,
                   RTCScene scene,
                   uint32_t num_rays,
                   const struct RTCRayHit8* ray,
                   float* r,
                   float* g,
                   float* b);

    void (*trace16)(void* caller,
                    RTCScene scene,
                    uint32_t num_rays,
                    const struct RTCRayHit16* ray,
                    float* r,
                    float* g,
                    float* b);

    void (*error)(void* caller, const char* what);
  };

  void raygun_exec(void* caller_data,
                   const struct raygun_interface* interface,
                   const char* window_title,
                   const char* embree_config);

#ifdef __cplusplus
} /* extern "C" */
#endif

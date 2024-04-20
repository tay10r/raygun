#include <raygun.h>

#include <iostream>

#include <cstdlib>

namespace {

void
setup(void* ptr, RTCDevice device, RTCScene scene)
{
  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

  float* vertices =
    (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), 3);

  unsigned* indices =
    (unsigned*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned), 1);

  if (vertices && indices) {

    vertices[0] = -1.f;
    vertices[1] = -1.f;
    vertices[2] = -1.f;

    vertices[3] = 1.f;
    vertices[4] = -1.f;
    vertices[5] = -1.f;

    vertices[6] = 0.f;
    vertices[7] = 1.f;
    vertices[8] = -1.f;

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
  }

  rtcCommitGeometry(geom);

  rtcAttachGeometry(scene, geom);

  rtcReleaseGeometry(geom);

  rtcCommitScene(scene);
}

void
trace(void* ptr, RTCScene scene, const uint32_t num_rays, const RTCRayHit* ray_hit, float* r, float* g, float* b)
{
  for (uint32_t i = 0; i < num_rays; i++) {
    if (ray_hit[i].hit.geomID != RTC_INVALID_GEOMETRY_ID) {
      r[i] = 1;
      g[i] = 1;
      b[i] = 1;
    } else {
      r[i] = 0;
      g[i] = 0;
      b[i] = 0;
    }
  }
}

void
on_error(void* ptr, const char* msg)
{
  std::cerr << "ERROR: " << msg << std::endl;
}

const raygun_interface interface {
  // clang-format off
  /*setup=*/setup,
  /*teardown=*/nullptr,
  /*frame=*/nullptr,
  trace,
  /*trace4=*/nullptr,
  /*trace8=*/nullptr,
  /*trace16=*/nullptr,
  on_error
  // clang-format on
};

} // namespace

auto
main() -> int
{
  raygun_exec(/*caller_data=*/nullptr, &interface, "Raygun Demo", /*embree_config=*/nullptr);

  return EXIT_SUCCESS;
}

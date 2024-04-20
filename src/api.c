#include <raygun.h>

#include "runtime.h"

void
raygun_exec(void* caller_data,
            const struct raygun_interface* interface,
            const char* window_title,
            const char* embree_config)
{
  struct rg_runtime* rt = rg_runtime_new(caller_data, interface, window_title, embree_config);
  if (!rt) {
    return;
  }

  int should_close = 0;

  while (!should_close) {

    rg_runtime_iterate(rt, &should_close);
  }

  rg_runtime_delete(rt);
}

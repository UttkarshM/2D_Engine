#include "engine_libs.h"

#include "platform.h"

#define APIENTRY
#include "../third_party/glcorearb.h"

#ifdef _WIN32
#include "windows_platform.cpp"
#endif

#include "gl_renderer.h"


int main()
{
  platform_create_window(1200, 720, "Engine window");

  while(running)
  {
    // Update
    platform_update_window();
  }

  return 0;
}
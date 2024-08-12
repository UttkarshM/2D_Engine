#include "engine_libs.h"

#include "input.h"

#include "game.cpp"

#include "platform.h"

#include <cstdlib>

#define APIENTRY
#define GL_GLEXT_PROTOTYPES

#include "../third_party/glcorearb.h"

#ifdef _WIN32

#include "windows_platform.cpp"
#endif

#include "gl_renderer.cpp"


int main()
{
  BumpAllocator transientStorage = make_bump_allocator(MB(50));
  platform_create_window(1200, 720, "Engine window");

  input.screenSizeX = 1200;
  input.screenSizeY = 720;

  gl_init(&transientStorage);
  while(running)
  {
    // Update
    platform_update_window();
    update_game();
    gl_render();
    platform_swap_buffers();
  }

  return 0;
}
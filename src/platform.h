#pragma  once
static bool running = true;

bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_function(char* function_name);
void platform_swap_buffers();
void platform_set_vsync(bool vSync);
void* platform_load_dynamic_function(void *dll,char *funName);
void* platform_load_dynamic_library(char *dll);
bool platform_free_dynamic_library(void *dll);
void platform_fill_keycode_lookup_table();
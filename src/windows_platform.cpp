#include "platform.h"
#include "engine_libs.h"
#include "../third_party/glcorearb.h"
#include "../third_party/wglext.h"
#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

#define NOMINMAX
#include <windows.h>


// #############################################################################
//                           Windows Globals
// #############################################################################
static HWND window;

// #############################################################################
//                           Platform Implementations
// #############################################################################
LRESULT CALLBACK windows_window_callback(HWND window, UINT msg,
                                         WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  switch(msg)
  {
    case WM_CLOSE:
    {
      running = false;
      break;
    }

    default:
    {
      // Let windows handle the default input for now
      result = DefWindowProcA(window, msg, wParam, lParam);
    }
  }

  return result;
}

bool platform_create_window(int width, int height, char* title)
{
  HINSTANCE instance = GetModuleHandleA(0);

  WNDCLASSA wc = {};
  wc.hInstance = instance;
  wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);       // This means we decide the look of the cursor(arrow)
  wc.lpszClassName = title;                       // This is NOT the title, just a unique identifier(ID)
  wc.lpfnWndProc = windows_window_callback;       // Callback for Input into the Window

  if(!RegisterClassA(&wc))
  {
    return false;
  }

  // WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
  int dwStyle = WS_OVERLAPPEDWINDOW;
  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
  {

  window = CreateWindowExA(0, title, // This references lpszClassName from wc
                           title,    // This is the actual Title
                           dwStyle,
                           100,
                           100,
                           width,
                           height,
                           NULL,     // parent
                           NULL,     // menu
                           instance,
                           NULL);    // lpParam

  if(window == NULL)
  {
    EN_ASSERT(false,"failed to create a windows");
    return false;
  }
  
  HDC fakeDC = GetDC(window);

  if(!fakeDC){
    EN_ASSERT(false,"error in generating ");
    return false;
  }

  PIXELFORMATDESCRIPTOR pfd = {0};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cAlphaBits = 8;
  pfd.cDepthBits = 24;

  int pixelFormat = ChoosePixelFormat(fakeDC,&pfd);
  if(!pixelFormat){
    EN_ASSERT(false, "failed to choose pixel format");
    return false;
  }
  if(!SetPixelFormat(fakeDC, pixelFormat, &pfd)){
    EN_ASSERT(false, "failed to set pixel format");
    return false;
  }
  // Create a Handle to a fake OpenGL Rendering Context
  HGLRC fakeRC = wglCreateContext(fakeDC);
  if(!fakeRC){
    EN_ASSERT(false, "failed to create a rendering context");
    return false;
  }



  if(!wglMakeCurrent(fakeDC, fakeRC)){
    EN_ASSERT(false, "Failed to make current");
    return false;
  }

  wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");

  if(!wglChoosePixelFormatARB || !wglCreateContextAttribsARB){
    EN_ASSERT(false, "Failed to load OpenGL functions");
    return false;
  }

  wglMakeCurrent(fakeDC, 0);
  wglDeleteContext(fakeRC);
  ReleaseDC(window, fakeDC);


  DestroyWindow(window);
  }


  { //actual opengl initialzation

  {
    RECT borderRect = {};
    AdjustWindowRectEx(&borderRect,dwStyle,0,0);

    width +=borderRect.right -borderRect.left;
    height += borderRect.bottom - borderRect.top;
  }
    window = CreateWindowExA(0, title, // This references lpszClassName from wc
                           title,    // This is the actual Title
                           dwStyle,
                           100,
                           100,
                           width,
                           height,
                           NULL,     // parent
                           NULL,     // menu
                           instance,
                           NULL);    // lpParam

  if(window == NULL)
  {
    EN_ASSERT(false,"failed to create a windows");
    return false;
  }

  HDC dc = GetDC(window);

  if(!dc){
    EN_ASSERT(false,"error in generating ");
    return false;
  }
  
  const int pixelAttribs[] =
  {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
    WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB,     32,
    WGL_ALPHA_BITS_ARB,     8,
    WGL_DEPTH_BITS_ARB,     24,
    0 // Terminate with 0, otherwise OpenGL will throw an Error!
  };

  UINT numPixelFormats;
  int pixelFormat = 0;
  if(!wglChoosePixelFormatARB(dc,pixelAttribs,0,1,&pixelFormat,&numPixelFormats)){
    EN_ASSERT(false, "Failed to wglChoosePixelFormatARB");
    return false;
  }


  }

  ShowWindow(window, SW_SHOW);

  return true;
}

void platform_update_window()
{
  MSG msg;

  while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessageA(&msg); // Calls the callback specified when creating the window
  }
}
void* platform_load_gl_function(char* function_name){
    PROC proc = wglGetProcAddress("glCreateProgram");
    if(!proc){
      static HMODULE openglDLL = LoadLibraryA("opengl32.dll");
      proc = GetProcAddress(openglDLL, function_name);
      EN_ASSERT(false,"Failed to load gl function %s","glCreateProgram");
    }
    return (void*)proc;
}
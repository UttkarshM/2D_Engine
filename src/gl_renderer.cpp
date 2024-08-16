#include "gl_renderer.h"
#include "engine_libs.h"
#include "render_interface.h"
#include <iostream>
#include <ostream>


#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
struct GLContext
{
  GLuint programID;
  GLuint textureID;
  GLuint transformSBOID;
  GLuint screenSizeID;
  GLuint orthoProjectionID;
};

static GLContext glContext;

static void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                         GLsizei length, const GLchar* message, const void* user)
{
  if(severity == GL_DEBUG_SEVERITY_LOW || 
     severity == GL_DEBUG_SEVERITY_MEDIUM ||
     severity == GL_DEBUG_SEVERITY_HIGH)
  {
    EN_ASSERT(false, "OpenGL Error: %s", message);
  }
  else
  {
    EN_TRACE((char*)message);
  }
}


bool gl_init(BumpAllocator* transientStorage)
{
  load_gl_functions();

  glDebugMessageCallback(&gl_debug_callback, nullptr);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glEnable(GL_DEBUG_OUTPUT);

  GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  int fileSize = 0;
  char* vertShader = read_file("C:/Users/uttka/Desktop/god/assets/shaders/quad.vert", &fileSize, transientStorage);
  char* fragShader = read_file("C:/Users/uttka/Desktop/god/assets/shaders/quad.frag", &fileSize, transientStorage);

  if(!vertShader || !fragShader)
  {
    EN_ASSERT(false, "Failed to load shaders");
    return false;
  }

  glShaderSource(vertShaderID, 1, &vertShader, 0);
  glShaderSource(fragShaderID, 1, &fragShader, 0);

  glCompileShader(vertShaderID);
  glCompileShader(fragShaderID);

  {
    int success;
    char shaderLog[2048] = {};

    glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(vertShaderID, 2048, 0, shaderLog);
      EN_ASSERT(false, "Failed to compile Vertex Shaders %s", shaderLog);
    }
  }

  {
    int success;
    char shaderLog[2048] = {};

    glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(fragShaderID, 2048, 0, shaderLog);
      EN_ASSERT(false, "Failed to compile Fragment Shaders %s", shaderLog);
    }
  }

  glContext.programID = glCreateProgram();
  glAttachShader(glContext.programID, vertShaderID);
  glAttachShader(glContext.programID, fragShaderID);
  glLinkProgram(glContext.programID);

  glDetachShader(glContext.programID, vertShaderID);
  glDetachShader(glContext.programID, fragShaderID);
  glDeleteShader(vertShaderID);
  glDeleteShader(fragShaderID);

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  //Texture loading using stbi

// Load the texture
    {
    int width, height, channels;
    char* data = (char*)stbi_load("C:/Users/uttka/Desktop/god/assets/texture/dice.png", &width, &height, &channels, 4);
    if(!data)
    {
      EN_ASSERT(false, "Failed to load texture");
      return false;
    }

    glGenTextures(1, &glContext.textureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glContext.textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // This setting only matters when using the GLSL texture() function
    // When you use texelFetch() this setting has no effect,
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
  }

  {
    glGenBuffers(1, &glContext.transformSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform)*MAX_TRANSFORMS ,renderData->transforms,GL_DYNAMIC_DRAW); //we use dynamic as we will still keep the same allocation when we change the data
  }

  // Uniforms
  {
    glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screen_size");
    glContext.orthoProjectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
  }
  
  // Your font is not using sRGB, for example (not that it matters there, because no actual color is sampled from it)
  glEnable(GL_FRAMEBUFFER_SRGB); // human eye doesnt interpret color in linear space , soo we need to convert the color in srgb back to human viewable form.
  glDisable(0x809D);

  

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);

  glUseProgram(glContext.programID);

  return true;
}


void gl_render() {
    glClearColor(119.0f / 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, input->screenSize.x, input->screenSize.y);
  
    Vec2 screen_size = { (float)input->screenSize.x, (float)input->screenSize.y };
    glUniform2fv(glContext.screenSizeID, 1, &screen_size.x);

    OrthographicCamera2D camera = renderData->gameCamera;
    Mat4 orthoProjection = orthographic_projection(
        camera.position.x - camera.dimensions.x / 2.0f,
        camera.position.x + camera.dimensions.x / 2.0f,
        camera.position.y - camera.dimensions.y / 2.0f,
        camera.position.y + camera.dimensions.y / 2.0f
    );

    std::cout << "Ortho Projection: " << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << orthoProjection[i][j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Camera Position: (" << camera.position.x << ", " << camera.position.y << ")" << std::endl;
    std::cout << "Camera Dimensions: (" << camera.dimensions.x << ", " << camera.dimensions.y << ")" << std::endl;

    glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

    // Copies transforms to the GPU
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transformCount, renderData->transforms);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transformCount); // The transform count is the glIndex that we use in the shader language
    renderData->transformCount = 0;
}

#include "gl_renderer.h"
#include "engine_libs.h"
#include "render_interface.h"


#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"


struct GLContext
{
  GLuint programID;
  GLuint textureID;
  GLuint transformSBOID;
  GLuint screenSizeID;
  GLuint orthoProjectionID;

  long long textureTimestamp;
  long long shaderTimestamp;
};

static GLContext glContext;

const char* TEXTURE_PATH = "C:/Users/uttka/Desktop/god/assets/texture/TEXTURE_ATLAS (2).png";
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

GLuint gl_create_shader(int shaderType, char* shaderPath, BumpAllocator* transientStorage){
  int fileSize = 0;
  char *vertShader = read_file(shaderPath,&fileSize,transientStorage);
  if(!vertShader){
    EN_ASSERT(false, "Failed to read shader file");
    return 0;
  }
  GLuint shaderID = glCreateShader(shaderType);
  glShaderSource(shaderID,1,&vertShader,0);
  glCompileShader(shaderID);
   {
    int success;
    char shaderLog[2048] = {};

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);
      EN_ASSERT(false, "Failed to compile %s Shaders %s \n %s",shaderType , shaderLog, shaderPath);
      return 0;
    }
  }
  return shaderID;
}


bool gl_init(BumpAllocator* transientStorage)
{
  load_gl_functions();

  glDebugMessageCallback(&gl_debug_callback, nullptr);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glEnable(GL_DEBUG_OUTPUT);

  GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, "C:/Users/uttka/Desktop/god/assets/shaders/quad.vert",transientStorage);
  
  GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, "C:/Users/uttka/Desktop/god/assets/shaders/quad.frag",transientStorage);

  if(!vertShaderID || !fragShaderID){
    EN_ASSERT(false, "failed to create shaders");
    return false;
  }

  long long timestampVert = get_timestamp("C:/Users/uttka/Desktop/god/assets/shaders/quad.vert");
  long long timestampFrag = get_timestamp("C:/Users/uttka/Desktop/god/assets/shaders/quad.frag");

  glContext.shaderTimestamp = max(timestampVert,timestampFrag);

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
    char* data = (char*)stbi_load(TEXTURE_PATH, &width, &height, &channels, 4);
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

    glContext.textureTimestamp = get_timestamp(TEXTURE_PATH);

    stbi_image_free(data);
  }

  {
    glGenBuffers(1, &glContext.transformSBOID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform)*renderData->transforms.maxElements ,renderData->transforms.elements,GL_DYNAMIC_DRAW); //we use dynamic as we will still keep the same allocation when we change the data
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


void gl_render(BumpAllocator* transientStorage) {
  {
    long long currentTimestamp = get_timestamp(TEXTURE_PATH);
    if(currentTimestamp > glContext.textureTimestamp){
      glActiveTexture(GL_TEXTURE0);
      int width, height, nChannels;
      char* data = (char *)stbi_load(TEXTURE_PATH, &width, &height, &nChannels, 4);
      if(data){
        glContext.textureTimestamp = currentTimestamp;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
      }
    }
  }

  {
    long long timestampVert = get_timestamp("assets/shaders/quad.vert");
    long long timestampFrag = get_timestamp("assets/shaders/quad.frag");
    
    if(timestampVert > glContext.shaderTimestamp ||
       timestampFrag > glContext.shaderTimestamp)
    {
      GLuint vertShaderID = gl_create_shader(GL_VERTEX_SHADER, 
                                              "assets/shaders/quad.vert", transientStorage);
      GLuint fragShaderID = gl_create_shader(GL_FRAGMENT_SHADER, 
                                              "assets/shaders/quad.frag", transientStorage);
      if(!vertShaderID || !fragShaderID)
      {
        EN_ASSERT(false, "Failed to create Shaders")
        return;
      }
      glAttachShader(glContext.programID, vertShaderID);
      glAttachShader(glContext.programID, fragShaderID);
      glLinkProgram(glContext.programID);

      glDetachShader(glContext.programID, vertShaderID);
      glDetachShader(glContext.programID, fragShaderID);
      glDeleteShader(vertShaderID);
      glDeleteShader(fragShaderID);

      glContext.shaderTimestamp = max(timestampVert, timestampFrag);
    }
  }


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


    glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

    // Copies transforms to the GPU
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transforms.count, renderData->transforms.elements);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transforms.count); // The transform count is the glIndex that we use in the shader language
    renderData->transforms.clear();
}

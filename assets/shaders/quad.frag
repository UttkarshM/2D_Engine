#version 430 core

layout (location = 0) in vec2 textureCoordsIn;

layout (location = 0) out vec4 fragColor;

//bindings
layout (location = 0) uniform sampler2D textureAtlas;

void main()
{
  vec4 textureColor = texelFetch(textureAtlas,ivec2(textureCoordsIn),0);
  if(textureColor.a == 0){
    discard;
  }
  // fragColor = vec4(0.0, 0.5, 0.0, 1.0);
  fragColor = textureColor;
}
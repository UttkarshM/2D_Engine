#version 430 core

//structs
struct Transform{
    ivec2 atlas_offset;
    ivec2 sprite_size;
    vec2 pos;
    vec2 size;
};

//input
layout (std430, binding = 0) buffer TransformSBO
{
  Transform transforms[];
};


uniform vec2 screen_size;
uniform mat4 orthoProjection;

//output
layout (location = 0) out vec2 textureCoordsOut;


void main()
{

  Transform transform = transforms[gl_InstanceID];

  vec2 vertices[6] = {
    transform.pos,    //Top Left (anchor point)
    vec2(transform.pos + vec2(0.0, transform.size.y)),    // Bottom Left
    vec2(transform.pos + vec2(transform.size.x, 0.0)),    // Top Right
    vec2(transform.pos + vec2(transform.size.x, 0.0)),    // Top Right
    vec2(transform.pos + vec2(0.0, transform.size.y)),    // Bottom Left
    transform.pos + transform.size  // Bottom Right
  };

  float left = transform.atlas_offset.x;
  float top = transform.atlas_offset.y;
  float right = transform.atlas_offset.x + transform.sprite_size.x;
  float bottom = transform.atlas_offset.y + transform.sprite_size.y;

  vec2 textureCoords[6] = {
    vec2(left,top),
    vec2(left,bottom),
    vec2(right,top),
    vec2(right,top),
    vec2(left,bottom),
    vec2(right,bottom),
  };

  {
    vec2 vertexPos = vertices[gl_VertexID];
    gl_Position = orthoProjection*vec4(vertexPos, 0.0, 1.0);
  }
  
  textureCoordsOut = textureCoords[gl_VertexID];
}


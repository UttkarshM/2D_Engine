#pragma once
#include "core.h"
#include "sprites.h"
#include "input.h"


int RENDER_OPTION_FLIP_X = BIT(0);
int RENDER_OPTION_FLIP_Y = BIT(1);

struct OrthographicCamera2D
{
    float zoom =1.0f;
    Vec2 position;
    Vec2 dimensions;
};
struct DrawData
{
  int animationIdx;
  int renderOptions;
};

struct Transform{ //imp keep vec2 first cause its crashing in the wrong order

    IVec2 atlas_offset;  //for textures
    IVec2 sprite_size;   //for textures
    Vec2 pos;           //for the vertex structure
    Vec2 size;          //for the vertex structure
    int animationIdx;
    int renderOptions;
};

struct RenderData{
    OrthographicCamera2D gameCamera;
    OrthographicCamera2D uiCamera;
    int transformCount;
    // Transform transforms[MAX_TRANSFORMS];
    Array<Transform,10000> transforms;
};



//globals

static RenderData* renderData;

//ivec2 makes u handle only single pixel even if due to mapping a single pixel in screen might map to 3 pixels in world
//if we return vec2 it is possible as decimals 
IVec2 screen_to_world(IVec2 screenPos)
{
  OrthographicCamera2D camera = renderData->gameCamera;

  int xPos = (float)screenPos.x / 
             (float)input->screenSize.x * 
             camera.dimensions.x; // [0; dimensions.x]

  // Offset using dimensions and position
  xPos += -camera.dimensions.x / 2.0f + camera.position.x;

  int yPos = (float)screenPos.y / 
             (float)input->screenSize.y * 
             camera.dimensions.y; // [0; dimensions.y]

  // Offset using dimensions and position
  yPos += camera.dimensions.y / 2.0f + camera.position.y;

  return {xPos, yPos};
}

void draw_quad(Transform transform)
{
  renderData->transforms.add(transform);
}

void draw_quad(Vec2 pos, Vec2 size)
{
  Transform transform = {};
  transform.pos = pos - size / 2.0f;
  transform.size = size;
  transform.atlas_offset = {0, 0};
  transform.sprite_size = {1, 1}; // Indexing into white

  renderData->transforms.add(transform);
}
void draw_sprite(SpriteID spriteID,Vec2 pos,Vec2 size){
    Sprite sprite = get_sprite(spriteID);
    Transform transform = {};
    // transform.pos.x = pos.x - size.x/2.0f;
    // transform.pos.y = pos.y - size.y/2.0f;
    transform.pos = pos;
    transform.size=size;
    // transform.size = vec_2(sprite.sprite_size);
    transform.atlas_offset = sprite.atlas_offset;
    transform.sprite_size = sprite.sprite_size;

    renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, Vec2 pos, DrawData drawData = {})
{
  Sprite sprite = get_sprite(spriteID);
  // For Animations, this is a multiple of the sprites size,
  sprite.atlas_offset.x += drawData.animationIdx * sprite.sprite_size.x;

  Transform transform = {};
  transform.pos = pos - vec_2(sprite.sprite_size) / 2.0f;
  transform.size = vec_2(sprite.sprite_size);
  transform.atlas_offset = sprite.atlas_offset;
  transform.sprite_size = sprite.sprite_size;
  transform.renderOptions = drawData.renderOptions;

  renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, IVec2 pos, DrawData drawData = {})
{
  draw_sprite(spriteID, vec_2(pos), drawData);
}

int animate(float* time, int frameCount, float duration = 1.0f)
{
  while(*time > duration)
  {
    *time -= duration;
  }
  
  int animationIdx = (int)((*time / duration) * frameCount);
  
  if (animationIdx >= frameCount)
  {
    animationIdx = frameCount - 1;
  }
  
  return animationIdx;
} 
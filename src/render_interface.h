#pragma once
#include "engine_libs.h"
#include "assets.h"
#include "input.h"

constexpr int MAX_TRANSFORMS = 1000;

struct OrthographicCamera2D
{
    float zoom =1.0f;
    Vec2 position;
    Vec2 dimensions;
};

struct Transform{ //imp keep vec2 first cause its crashing in the wrong order

    IVec2 atlas_offset;  //for textures
    IVec2 sprite_size;   //for textures
    Vec2 pos;           //for the vertex structure
    Vec2 size;          //for the vertex structure

};

struct RenderData{
    OrthographicCamera2D gameCamera;
    OrthographicCamera2D uiCamera;
    int transformCount;
    Transform transforms[MAX_TRANSFORMS];
};
//globals

static RenderData* renderData;

//ivec2 makes u handle only single pixel even if due to mapping a single pixel in screen might map to 3 pixels in world
//if we return vec2 it is possible as decimals 
IVec2 screen_to_world(IVec2 screenPos) //mapping the (mouse)screen to the camera coords
{
  OrthographicCamera2D camera = renderData->gameCamera;

  int x_pos = screenPos.x / input->screenSize.x * (int)camera.dimensions.x;

  // we are offsetting as the origin points for camera is at the center whereas the origin point for the screen is top left
  x_pos += -camera.dimensions.x / 2.0f + camera.position.x;

  int y_pos = screenPos.y / input->screenSize.y * (int)camera.dimensions.y;

  // same offsetting used above for xpos
  y_pos += -camera.dimensions.y / 2.0f - camera.position.y;

  return {x_pos, y_pos};
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

    renderData->transforms[renderData->transformCount++]=transform;
}
void draw_sprite(SpriteID spriteID, Vec2 pos)
{
  Sprite sprite = get_sprite(spriteID);

  Transform transform = {};
  transform.pos.x = pos.x - vec_2(sprite.sprite_size).x / 2.0f;
  transform.pos.y = pos.y - vec_2(sprite.sprite_size).y / 2.0f;
  
  transform.size = vec_2(sprite.sprite_size);
  transform.atlas_offset = sprite.atlas_offset;
  transform.sprite_size = sprite.sprite_size;

  renderData->transforms[renderData->transformCount++] = transform;
}

void draw_sprite(SpriteID spriteID, IVec2 pos)
{
    draw_sprite(spriteID,vec_2(pos));
}
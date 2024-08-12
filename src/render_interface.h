#pragma once
#include "engine_libs.h"
#include "assets.h"

constexpr int MAX_TRANSFORMS = 1000;

struct Transform{ //imp keep vec2 first cause its crashing in the wrong order

    IVec2 atlas_offset;  //for textures
    IVec2 sprite_size;   //for textures
    Vec2 pos;           //for the vertex structure
    Vec2 size;          //for the vertex structure

};

struct RenderData{
    int transformCount;
    Transform transforms[MAX_TRANSFORMS];
};
//globals

static RenderData renderData;

void draw_sprite(SpriteID spriteID,Vec2 pos,Vec2 size){
    Sprite sprite = get_sprite(spriteID);
    Transform transform = {};
    transform.pos = pos;
    transform.size = size;
    transform.atlas_offset = sprite.atlas_offset;
    transform.sprite_size = sprite.sprite_size;

    renderData.transforms[renderData.transformCount++]=transform;
}

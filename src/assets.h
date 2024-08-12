#pragma once
#include "engine_libs.h"

//assets
enum SpriteID{
    SPRITE_DICE,
    SPRITE_COUNT
};


struct Sprite{
    IVec2 atlas_offset;
    IVec2 sprite_size;
};

Sprite get_sprite(SpriteID spriteID){
    Sprite sprite ={};
    switch(spriteID){
        case SPRITE_DICE:{
            sprite.atlas_offset = {0,0};
            sprite.sprite_size = {16,16};
        }
    }
    return sprite;
}
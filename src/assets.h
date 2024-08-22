#pragma once
#include "engine_libs.h"

//assets
enum SpriteID{
    SPRITE_WHITE,
    SPRITE_DICE,
    SPRITE_COUNT,
    SPRITE_CELESTE,
    SPRITE_SOLID_01,
    SPRITE_SOLID_02    
};


struct Sprite{
    IVec2 atlas_offset;
    IVec2 sprite_size;
};

Sprite get_sprite(SpriteID spriteID){
    Sprite sprite ={};
    switch(spriteID){
        case SPRITE_WHITE:{
            sprite.atlas_offset={0,0};
            sprite.sprite_size={1,1}; 
            break;
        }

        case SPRITE_DICE:{
            sprite.atlas_offset = {16,0};
            sprite.sprite_size= {16,16};
            break;
        }
        case SPRITE_CELESTE:{
            sprite.atlas_offset = {112, 0};
            sprite.sprite_size = {17, 20};
            break;
        }
        case SPRITE_SOLID_01:{
        sprite.atlas_offset = {0, 16};
        sprite.sprite_size = {28, 18};
        break;
        }

        case SPRITE_SOLID_02:{
        sprite.atlas_offset = {32, 16};
        sprite.sprite_size = {16, 13};
        break;
        }
    }
    return sprite;
}
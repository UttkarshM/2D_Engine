#pragma once

#include "sprites.h"
#include "core.h"
#include "render_interface.h"
#include "input.h"
#include "sound.h"
#include "game_libs.h"

// extern GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(GameState* gameStateIn,RenderData* renderDataIn ,Input* inputIn,float dt,SoundState* soundStateIn); 
}
//exposes the function in the dll

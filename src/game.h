#pragma once

#include "engine_libs.h"
#include "render_interface.h"
#include "input.h"

constexpr int tset=5;

struct GameState{
    bool initialized = false;
    IVec2 playerPos;
};


static GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(GameState* gameStateIn,RenderData* renderDataIn ,Input* inputIn); 
}
//exposes the function in the dll

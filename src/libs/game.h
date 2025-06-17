#pragma once

#include "sprites.h"
#include "core.h"
#include "render_interface.h"
#include "input.h"
#include "sound.h"
#include "game_libs.h"

constexpr float runSpeed = 2.0f;
constexpr float runAcceleration = 10.0f;
constexpr float runReduce = 22.0f;
constexpr float flyReduce = 10.0f;
constexpr float gravity = 13.0f;
constexpr float fallSpeed = 2.6f;
constexpr float jumpSpeed = -3.0f;

// extern GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(GameState* gameStateIn,RenderData* renderDataIn ,Input* inputIn,float dt,SoundState* soundStateIn); 
}
//exposes the function in the dll

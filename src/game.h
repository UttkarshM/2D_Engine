#pragma once

#include "assets.h"
#include "engine_libs.h"
#include "render_interface.h"
#include "input.h"
#include "sound.h"

//global
constexpr int tset=5;

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int UPDATES_PER_SECOND = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
constexpr int TILESIZE = 8;

constexpr IVec2 WORLD_GRID = {WORLD_WIDTH/TILESIZE,WORLD_HEIGHT/TILESIZE};


enum GameInputType
{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,

    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,

    SAVE_STATE,

    GAME_INPUT_COUNT
};

struct KeyMapping
{
    Array<KeyCodeID,3> keys;
};

struct Tile{
    int neighbourMask;
    bool isVisible;
};

enum PlayerAnimateState
{
    PLAYER_ANIMATE_IDLE,
    PLAYER_ANIMATE_RUN,
    PLAYER_ANIMATE_JUMP,
    PLAYER_ANIMATE_COUNT
};

struct Player{
    IVec2 pos;
    IVec2 prevPos;
    Vec2 speed;
    Vec2 solidSpeed;
    int renderOptions;
    float runAnimateTime;
    PlayerAnimateState animationState;
    SpriteID animationSprites[PLAYER_ANIMATE_COUNT];
};

struct Solid{
    SpriteID spriteID;
    IVec2 pos;
    IVec2 prevPos;
    Vec2 remainder;
    Vec2 speed;
    int keyframeIdx;
    Array<IVec2,2>keyframes;
    
    float slip;
};

struct Static_solids{
    SpriteID spriteID;
    IVec2 pos;
};

struct GameState{
    float updateTimer;
    bool initialized = false;
    Player player;
    Array<Solid, 1000> solids;
    Array<Static_solids,1000> static_solids;
    Array<IVec2,21> tileCoords; //21 because we use 21 texture points
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
    KeyMapping keyMappings[GAME_INPUT_COUNT];
};


static GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(GameState* gameStateIn,RenderData* renderDataIn ,Input* inputIn,float dt,SoundState* soundStateIn); 
}
//exposes the function in the dll

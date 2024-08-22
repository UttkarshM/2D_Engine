#pragma once

#include "engine_libs.h"
#include "render_interface.h"
#include "input.h"

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

struct Player{
    IVec2 pos;
    IVec2 prevPos;
    Vec2 speed;
    Vec2 solidSpeed;
};

struct Solid{
    SpriteID spriteID;
    IVec2 pos;
    IVec2 prevPos;
    Vec2 remainder;
    Vec2 speed;
    int keyframeIdx;
    Array<IVec2,2>keyframes;
};

struct GameState{
    float updateTimer;
    bool initialized = false;
    Player player;
    Array<Solid, 100> solids;
    Array<IVec2,21> tileCoords; //21 because we use 21 texture points
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
    KeyMapping keyMappings[GAME_INPUT_COUNT];
};


static GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(GameState* gameStateIn,RenderData* renderDataIn ,Input* inputIn,float dt); 
}
//exposes the function in the dll

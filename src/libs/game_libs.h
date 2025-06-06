#pragma once
#include "core.h"
#include "input.h"
#include "sprites.h"
#include <fstream>
#include <string>
#include <vector>



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

bool just_pressed(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for(int idx = 0; idx < mapping.keys.count; idx++)
  {
    if(input->keys[mapping.keys[idx]].justPressed)
    {
      return true;
    }
  }

  return false;
}

bool is_down(GameInputType type)
{
  KeyMapping mapping = gameState->keyMappings[type];
  for (int idx = 0; idx < mapping.keys.count; idx++)
  {
    if (input->keys[mapping.keys[idx]].isDown)
    {
      return true;
    }
  }

  return false;
}

IVec2 get_grid_pos(IVec2 worldPos){
  return {worldPos.x/TILESIZE,worldPos.y/TILESIZE};
}

Tile* get_tile(int x,int y){
  Tile* tile = nullptr;
  if (x >= 0 && x < WORLD_GRID.x && y >= 0 && y < WORLD_GRID.y) {
    tile = &gameState->worldGrid[x][y];
  }
  return tile;
}


Tile* get_tile(IVec2 worldPos)
{
  IVec2 gridPos = get_grid_pos(worldPos);
  return get_tile(gridPos.x, gridPos.y);
}


IVec2 get_tile_pos(int x,int y){
  return {x*TILESIZE,y*TILESIZE};
}

IRect get_tile_rect(int x,int y){
  return {get_tile_pos(x,y), 8, 8};
}


IRect get_player_rect(){
  return{
    gameState->player.pos.x - 4,
    gameState->player.pos.y - 8,
    8,
    16
  };
}


// hitboxes
bool inside_rect(IRect hitbox, IVec2 pos){
  if(hitbox.pos.x<=pos.x && hitbox.pos.x+hitbox.size.x>=pos.x
    && hitbox.pos.y<=pos.y && hitbox.pos.y+hitbox.size.y>=pos.y
  ){
    return true;
  }
  return false;
}

bool inside_static_rect(IRect hitbox, IVec2 pos){
  if(((hitbox.pos.x<=pos.x && hitbox.pos.x+hitbox.size.x>=pos.x) || (hitbox.pos.x <= pos.x - 4 && hitbox.pos.x+hitbox.size.x>=pos.x - 4)) //dies as he reaches halfway of the block
    && ((hitbox.pos.y<=pos.y && hitbox.pos.y+hitbox.size.y>=pos.y) || (hitbox.pos.y <= pos.y - 4 && hitbox.pos.y+hitbox.size.y>=pos.y -4))
  ){
    return true;
  }
  return false;
}


IRect get_solid_rect(Solid solid){
  Sprite sprite = get_sprite(solid.spriteID);
  return {solid.pos - sprite.sprite_size/2,sprite.sprite_size};
}

bool save_tile_set() {
  std::fstream file, file1;
  std::string filePath = std::string(ROOT_DIR) + "src/tiles/test2.tile";

  file.open(filePath, std::ios::out);
  if (!file) {
    EN_ERROR("failed to open file");
    return false; // Moved this line inside the if-block.
  }

  file << ""; // Empties the file.
  file.close();

  std::string file_path = std::string(ROOT_DIR) + "src/tiles/test2.tile";

  file1.open(file_path, std::ios::app);
  
  // Open file in append mode to write data.
  if (!file1) {
    EN_ERROR("failed to open file in append mode");
    return false;
  }
  std::string coOrds = "";
  for (int y = 0; y < WORLD_GRID.y; y++) {
    for (int x = 0; x < WORLD_GRID.x; x++) {
      Tile* tile = get_tile(x, y);
      if(tile->isVisible){
        coOrds+=std::to_string(x)+"-"+std::to_string(y)+"\n";
      }
    }
  }
  coOrds.pop_back();
  file1 << coOrds;

  file1.close(); // Correctly close the file opened in append mode.
  return true;
}

IVec2 parse_coords(std::string str) {
    int ind = 0;
    int res[2] = {0, 0}; // initialize both coordinates to zero
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '-') {
            ind++; // Move to the next index when a '-' is encountered
            if (ind > 1) break; // Prevent out-of-bounds error
        } else {
            res[ind] = res[ind] * 10 + (str[i] - '0'); // Accumulate the current number
        }
    }
    IVec2 r = {res[0], res[1]};
    return r;
}

std::vector<std::pair<int, int>> load_tile_set(GameState* gameState){
    std::fstream file;
    std::string temp;

    std::vector<std::pair<int, int>> tiles = {};
    std::string filePath = std::string(ROOT_DIR) + "src/tiles/test2.tile";

    file.open(filePath,std::ios::in);
    if (!file) {
      EN_ERROR("failed to open file");
      return {}; // Moved this line inside the if-block.
    }
    while(std::getline(file,temp)){
      // EN_TRACE("in loop");
      if(temp=="\n"){
        continue;
      }
      
      IVec2 coords = parse_coords(temp);
      gameState->worldGrid[coords.x][coords.y].isVisible=true;
    }
    file.close();
    return tiles;
}
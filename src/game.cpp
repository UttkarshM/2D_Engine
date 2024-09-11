#include "game.h"
#include "assets.h"
#include "engine_libs.h"
#include "input.h"
#include "render_interface.h"
#include "sound.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#define step 3


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

  file.open("C:/Users/uttka/Desktop/god/src/tiles/test2.tile", std::ios::out);
  if (!file) {
    EN_ERROR("failed to open file");
    return false; // Moved this line inside the if-block.
  }

  file << ""; // Empties the file.
  file.close();

  file1.open("C:/Users/uttka/Desktop/god/src/tiles/test2.tile", std::ios::app);
  
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

    file.open("C:/Users/uttka/Desktop/god/src/tiles/test2.tile",std::ios::in);
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


void simulate(std::vector<std::pair<int,int>>& loaded_tiles){

  float dt = UPDATE_DELAY;
  {
    Player& player = gameState->player;
    player.prevPos = player.pos;

    static Vec2 remainder = {};
    static bool grounded = false;
    constexpr float runSpeed = 2.0f;
    constexpr float runAcceleration = 10.0f;
    constexpr float runReduce = 22.0f;
    constexpr float flyReduce =10.0f;
    constexpr float gravity = 13.0f;
    constexpr float fallSpeed = 2.6f;
    constexpr float jumpSpeed = -3.0f;

//bounds    
    if(player.pos.x>320){ //exit
      player.pos={0,0};
      EN_TRACE("Won");
    }
    if(player.pos.x<0){ //stops from falling off of the map
      player.pos={0,0};
    }




    if((just_pressed(JUMP) || just_pressed(MOVE_UP)) && grounded){
      player.speed.y = jumpSpeed;
      player.speed.x+=player.solidSpeed.x;
      player.speed.y+=player.solidSpeed.y;
      grounded = false;

      play_sound("jump");
    }
    if(!grounded){
      player.animationState = PLAYER_ANIMATE_JUMP;
    }

    if(is_down(MOVE_LEFT))
    {
      if(grounded)
      {
        player.animationState = PLAYER_ANIMATE_RUN;
      }


      float mult = 1.0f;
      if(player.speed.x > 0.0f){
        mult = 3.0f;
      }
      player.speed.x = approach(player.speed.x, -runSpeed, runAcceleration*mult*dt);
    }
    if(is_down(MOVE_RIGHT))
    {
      float mult = 1.0f;
      if(player.speed.x < 0.0f){
        mult = 3.0f;
      }
      player.runAnimateTime += dt; 
      player.speed.x = approach(player.speed.x, runSpeed, runAcceleration*mult*dt);
    }
    if(player.speed.x >0){
      player.renderOptions = 0;
    }
    if(player.speed.x < 0){
      player.renderOptions = RENDER_OPTION_FLIP_X;
    }

    {
      IRect playerRect = get_player_rect();
      for(int i=0;i<gameState->static_solids.count;i++){
      if(inside_static_rect(playerRect, gameState->static_solids[i].pos)){
        player.pos={-160,-90};
        EN_TRACE("player killed");
      }
    }
    }

    //Map
    if(just_pressed(MOUSE_MIDDLE))
      {

        if(loaded_tiles.empty()){
          EN_ERROR("couldnt load");
        }
        else{
          EN_TRACE("loaded tile set");
        }

        player.pos ={8*2,8+2};
        player.speed ={0,0};
    }

    if(just_pressed(SAVE_STATE)){ //semicolon
        if(!save_tile_set()){
          EN_ERROR("couldnt save");
        }
        else{
          EN_TRACE("saved tile set");
        }
    }
    //Physics
    //Friction
    if(!is_down(MOVE_LEFT) && !is_down(MOVE_RIGHT)){
      if(grounded){
        player.speed.x = approach(player.speed.x, 0.0f, runReduce*dt);
      }
      else{
        player.speed.x = approach(player.speed.x, 0.0f, flyReduce*dt);
      }
    }

    //gravity
    player.speed.y = approach(player.speed.y, fallSpeed,gravity *dt);


    {
      IRect playerRect = get_player_rect();

      remainder.x += player.speed.x;
      int moveX = round(remainder.x);
      if(moveX != 0)
      {
        remainder.x -= moveX;
        int moveSign = sign(moveX);
        bool collisionHappened = false;

        auto movePlayerX = [&]
        {
          while(moveX)
          {
            playerRect.pos.x += moveSign;

            {
              for(int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++)
              {
                Solid& solid = gameState->solids[solidIdx];
                IRect solidRect = get_solid_rect(solid);

                if(rect_collision(playerRect, solidRect))
                {
                  player.speed.x = 0;
                  return;
                }
              }
            }

            IVec2 playerGridPos = get_grid_pos(player.pos);
            for(int x = playerGridPos.x -1 ; x <= playerGridPos.x +1; x++)
            {
              for(int y = playerGridPos.y -2; y <= playerGridPos.y +2 ; y++)
              {
                Tile* tile = get_tile(x, y);

                if(!tile || !tile->isVisible)
                {
                  continue;
                }

                IRect tileRect = get_tile_rect(x, y);
                if(rect_collision(playerRect, tileRect))
                {
                  player.speed.x = 0;
                  return;
                }
              }
            }

            player.pos.x += moveSign;
            moveX -= moveSign;
          }
        };
        movePlayerX();
      }
    }

    // Move Y
    {
      IRect playerRect = get_player_rect();

      remainder.y += player.speed.y;
      int moveY = round(remainder.y);
      if(moveY != 0)
      {
        remainder.y -= moveY;
        int moveSign = sign(moveY);
        bool collisionHappened = false;

        auto movePlayerY = [&]
        {
          while(moveY)
          {
            playerRect.pos.y += moveSign;

            // Test collision against Solids
            {
              for(int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++)
              {
                Solid& solid = gameState->solids[solidIdx];
                IRect solidRect = get_solid_rect(solid);

                if(rect_collision(playerRect, solidRect))
                {
                  // Moving down/falling
                  if(player.speed.y > 0.0f)
                  {
                    grounded = true;
                  }

                  player.speed.y = 0;
                  return;
                }
              }
            }

            // Loop through local Tiles
            IVec2 playerGridPos = get_grid_pos(player.pos);
            for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
            {
              for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
              {
                Tile* tile = get_tile(x, y);

                if(!tile || !tile->isVisible)
                {
                  continue;
                }

                IRect tileRect = get_tile_rect(x, y);
                if(rect_collision(playerRect, tileRect))
                {
                  // Moving down/falling
                  if(player.speed.y > 0.0f)
                  {
                    grounded = true;
                  }

                  player.speed.y = 0;
                  return;
                }
              }
            }

            // Move the Player
            player.pos.y += moveSign;
            moveY -= moveSign;
          }
        };
        movePlayerY();
      }
    }
  }

  // Update Solids
  {
    Player& player = gameState->player;
    player.solidSpeed = {};

    for(int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++)
    {
      Solid& solid = gameState->solids[solidIdx];
      solid.prevPos = solid.pos;

      IRect solidRect = get_solid_rect(solid);
      solidRect.pos -= 1;
      solidRect.size += 2;

      int nextKeyframeIdx = solid.keyframeIdx + 1;
      nextKeyframeIdx %= solid.keyframes.count;

      // Move X
      {
        solid.remainder.x += solid.speed.x * dt;
        int moveX = round(solid.remainder.x);
        if(moveX != 0)
        {
          solid.remainder.x -= moveX;
          int moveSign = sign(solid.keyframes[nextKeyframeIdx].x - 
                              solid.keyframes[solid.keyframeIdx].x); //for direction

          // Move the player in Y until collision or moveY is exausted
          auto moveSolidX = [&]
          {
            while(moveX)
            {
              IRect playerRect = get_player_rect();
              bool standingOnTop = 
                playerRect.pos.y - 1 + playerRect.size.y == solidRect.pos.y;

              solidRect.pos.x += moveSign;

              // Collision happend on left or right, push the player
              bool tileCollision = false;
              if(rect_collision(playerRect, solidRect))
              {
                // Move the player rect
                playerRect.pos.x += moveSign;
                player.solidSpeed.x = solid.speed.x * (float)moveSign *solid.slip / 20.0f;

                // destroy player if there is a collision
                IVec2 playerGridPos = get_grid_pos(player.pos);
                for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
                {
                  for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                  {
                    Tile* tile = get_tile(x, y);

                    if(!tile || !tile->isVisible)
                    {
                      continue;
                    }

                    IRect tileRect = get_tile_rect(x, y);
                    if(rect_collision(playerRect, tileRect))
                    {
                      tileCollision = true;

                      if(!standingOnTop)
                      {
                        // Death
                      player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                      }
                    }
                  }
                }

                if(!tileCollision)
                {
                  player.pos.x += moveSign;
                }
              }
              solid.pos.x += moveSign;
              moveX -= 1;

              if(solid.pos.x == solid.keyframes[nextKeyframeIdx].x)
              {
                solid.keyframeIdx = nextKeyframeIdx;
                nextKeyframeIdx++;
                nextKeyframeIdx %= solid.keyframes.count;
              }
            }
          };
          moveSolidX();
        }
      }

      // Move Y
      {
        solid.remainder.y += solid.speed.y * dt;
        int moveY = round(solid.remainder.y);
        if(moveY != 0)
        {
          solid.remainder.y -= moveY;
          int moveSign = sign(solid.keyframes[nextKeyframeIdx].y - solid.keyframes[solid.keyframeIdx].y);

          auto moveSolidY = [&]
          {
            while(moveY)
            {
              IRect playerRect = get_player_rect();
              solidRect.pos.x += moveSign;

              if(rect_collision(playerRect, solidRect))
              {

                player.pos.y += moveSign;
                player.solidSpeed.y = solid.speed.y * (float)moveSign / 40.0f;

                IVec2 playerGridPos = get_grid_pos(player.pos);
                for(int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++)
                {
                  for(int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++)
                  {
                    Tile* tile = get_tile(x, y);

                    if(!tile || !tile->isVisible)
                    {
                      continue;
                    }

                    IRect tileRect = get_tile_rect(x, y);
                    if(rect_collision(playerRect, tileRect))
                    {
                      player.pos = {WORLD_WIDTH / 2,  WORLD_HEIGHT - 24};
                    }
                  }
                }
              }

              // Move the Solid
              solid.pos.y += moveSign;
              moveY -= 1;

              if(solid.pos.y == solid.keyframes[nextKeyframeIdx].y)
              {
                solid.keyframeIdx = nextKeyframeIdx;
                nextKeyframeIdx++;
                nextKeyframeIdx %= solid.keyframes.count;
              }
            }
          };
          moveSolidY();
        }
      }
    }
  }

  bool updateTiles = false;
  if(is_down(MOUSE_LEFT))
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if(tile)
    {
      tile->isVisible = true;
      updateTiles = true;
    }
  }


  int once = 0; //initial bitmasking for the tiles loaded
  

  if(is_down(MOUSE_RIGHT))
  {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if(tile)
    {
      tile->isVisible = false;
      updateTiles = true;
    }
    save_tile_set();
  }

  if(updateTiles || !once)
  {
    once++;
    int neighbourOffsets[24] = { 0,-1,  -1, 0,     1, 0,       0, 1,   

                                -1,-1,   1,-1,    -1, 1,       1, 1,

                                 0,-2,  -2, 0,     2, 0,       0, 2};

    
    
    for(int y = 0; y < WORLD_GRID.y; y++)
    {
      for(int x = 0; x < WORLD_GRID.x; x++)
      {

        Tile* tile = get_tile(x, y);

        if(!tile->isVisible)
        {
          continue;
        }

        tile->neighbourMask = 0;
        int neighbourCount = 0;
        int extendedNeighbourCount = 0;
        int emptyNeighbourSlot = 0;


        for(int n = 0; n < 12; n++)
        {
          Tile* neighbour = get_tile(x + neighbourOffsets[n * 2],
                                     y + neighbourOffsets[n * 2 + 1]);

          if(!neighbour || neighbour->isVisible)
          {
            tile->neighbourMask |= BIT(n);
            if(n < 8)
            {
              neighbourCount++;
            }
            else
            {
              extendedNeighbourCount++;
            }
          }
          else if(n < 8)
          {
            emptyNeighbourSlot = n;
          }
        }

        if(neighbourCount == 7 && emptyNeighbourSlot >= 4)
        {
          tile->neighbourMask = 16 + (emptyNeighbourSlot - 4);
        }
        else if(neighbourCount == 8 && extendedNeighbourCount == 4)
        {
          tile->neighbourMask = 20;
        }
        else
        {
          tile->neighbourMask = tile->neighbourMask & 0b1111;
        }
      }
    }

  }

}


EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn,float dt,SoundState* soundStateIn)

{
  {
  std::vector<std::pair<int,int>> loaded_tiles = load_tile_set(gameStateIn);
  // if(loaded_tiles.size()<10){
  //   EN_ERROR("NOTHING");
  // }
  if(renderData != renderDataIn)
  {
    gameState = gameStateIn;
    renderData = renderDataIn;
    input = inputIn;
    soundState = soundStateIn;
  }

  if(!gameState->initialized)
  {
    renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
    gameState->initialized = true;

    {
      Player& player = gameState->player;
      player.animationSprites[PLAYER_ANIMATE_IDLE] = SPRITE_CELESTE;
      player.animationSprites[PLAYER_ANIMATE_JUMP] = SPRITE_CELESTE_JUMP;
      player.animationSprites[PLAYER_ANIMATE_RUN] = SPRITE_CELESTE_RUN;
    }
    // Tileset
    {
      IVec2 tilesPosition = {48, 0};

      for(int y = 0; y < 5; y++)
      {
        for(int x = 0; x < 4; x++)
        {
          gameState->tileCoords.add({tilesPosition.x +  x * 8, tilesPosition.y + y * 8});
        }
      }
      gameState->tileCoords.add({tilesPosition.x, tilesPosition.y + 5 * 8});
    }

    {
      gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
      gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
      gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
      gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
      gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);
      gameState->keyMappings[MOUSE_LEFT].keys.add(KEY_MOUSE_LEFT);
      gameState->keyMappings[MOUSE_RIGHT].keys.add(KEY_MOUSE_RIGHT);
      gameState->keyMappings[JUMP].keys.add(KEY_SPACE);
      gameState->keyMappings[MOUSE_MIDDLE].keys.add(KEY_MOUSE_MIDDLE);
      gameState->keyMappings[SAVE_STATE].keys.add(KEY_SEMICOLON);
    }


    renderData->gameCamera.position.x = 160;
    renderData->gameCamera.position.y = -90;
  
    {
      Sprite sprite = get_sprite(SPRITE_SOLID_01);
      Solid solid = {};
      int platform_size = sprite.sprite_size.x;

      solid.spriteID = SPRITE_SOLID_01;
      solid.keyframes.add({8 * 3,  8 * 10});
      solid.keyframes.add({8 * 10, 8 * 10});
      solid.pos = {8 * 2, 8 * 10};
      solid.speed.x = 50.0f;
      solid.slip = 0.100f;
      gameState->solids.add(solid);
      
    //static fire block
    // for(int i=0;i<8*2;i++){
    // for(int j=0;j<8*6;j++){
    //       Static_solids statSolid = {};
    //       statSolid.spriteID = SPRITE_FIRE;
    //       statSolid.pos = {8*29+j,8*17+i};
    //       gameState->static_solids.add(statSolid);
    //   }
    // }
    Static_solids statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*29,8*18+4};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*30,8*18+4};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*31,8*18+4};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*32,8*18+4};
    gameState->static_solids.add(statSolid);

    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*33,8*18+4};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*34,8*18+4};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID =SPRITE_MAGMA;
    statSolid.pos = {4+8*35,8*18+4};
    gameState->static_solids.add(statSolid);

    
    //     for(int i=0;i<8*1;i++){
    //       for(int j=0;j<8*2;j++){
    //       Static_solids statSolid = {};
    //       statSolid.spriteID = SPRITE_FIRE;
    //       statSolid.pos = {8*2+j,8*(17) +i};
    //       gameState->static_solids.add(statSolid);
    //   }
    // } 
    //hardcoding for now
    statSolid = {};
    statSolid.spriteID = SPRITE_MAGMA;
    statSolid.pos = {12,8*19};
    gameState->static_solids.add(statSolid);
    
    statSolid = {};
    statSolid.spriteID = SPRITE_MAGMA;
    statSolid.pos = {20,8*19};
    gameState->static_solids.add(statSolid);

    statSolid = {};
    statSolid.spriteID = SPRITE_MAGMA;
    statSolid.pos = {28,8*19};
    gameState->static_solids.add(statSolid);
    }
    
  }

  {
    gameState->updateTimer += dt;
    while(gameState->updateTimer >= UPDATE_DELAY){
      gameState->updateTimer -= UPDATE_DELAY;
      simulate(loaded_tiles);

      input->relMouse = input->mousePos - input->prevMousePos;
      input->prevMousePos = input->mousePos;

      {
        for(int keyCode = 0 ;keyCode <KEY_COUNT;keyCode++){
          input->keys[keyCode].justReleased = false;
          input->keys[keyCode].justPressed = false;
          input->keys[keyCode].halfTransitionCount = 0;
        }
      }
    }
  }
  float interpolatedDT = (float)(gameState->updateTimer/UPDATE_DELAY);
  
  {//solids
    for(int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++)
    {
      Solid& solid = gameState->solids[solidIdx];
      IVec2 solidPos = lerp(solid.prevPos, solid.pos, interpolatedDT);
      draw_sprite(solid.spriteID, solidPos);
    }
  }
  {
    for(int statsolidIdx = 0; statsolidIdx<gameState->static_solids.count;statsolidIdx++){
      Static_solids& solid = gameState->static_solids[statsolidIdx];
      draw_sprite(solid.spriteID, solid.pos);
    }
  }
  
  {//player
    {
    Player& player = gameState->player;
    IVec2 playerPos = lerp(player.prevPos, player.pos, interpolatedDT);
    
    Sprite sprite = get_sprite(player.animationSprites[player.animationState]);
    int animationIdx = animate(&player.runAnimateTime, sprite.frameCount, 0.6f);
    draw_sprite(player.animationSprites[player.animationState], playerPos, 
                {
                  .animationIdx = animationIdx,
                  .renderOptions = player.renderOptions
                });
  }
  }

  {//draw tileset
    for(int y = 0; y < WORLD_GRID.y; y++)
    {
      for(int x = 0; x < WORLD_GRID.x; x++)
      {
        Tile* tile = get_tile(x, y);

        if(!tile->isVisible)
        {
          continue;
        }

        Transform transform = {};
        transform.pos = {x * (float)TILESIZE, y * (float)TILESIZE};
        transform.size = {8, 8};
        transform.sprite_size = {8, 8};
        transform.atlas_offset = gameState->tileCoords[tile->neighbourMask];
        draw_quad(transform);
      }
    }
  }
}
}

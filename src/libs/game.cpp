#include "game.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <utility>
#include <vector>
#define step 3

void handle_player_input_and_movement(float dt, bool& grounded);
void handle_player_physics(Vec2& remainder, bool& grounded);
void handle_solid_movement_and_collision(float dt);
void handle_tile_editing_and_bitmask_update(std::vector<std::pair<int, int>>& loaded_tiles);

void simulate(std::vector<std::pair<int, int>>& loaded_tiles)
{
  float dt = UPDATE_DELAY;
  static Vec2 remainder = {};
  static bool grounded = false;

  Player& player = gameState->player;
  player.prevPos = player.pos;

  handle_player_input_and_movement(dt, grounded);
  handle_player_physics(remainder, grounded);
  handle_solid_movement_and_collision(dt);
  handle_tile_editing_and_bitmask_update(loaded_tiles);
}

void handle_player_input_and_movement(float dt, bool& grounded)
{
  Player& player = gameState->player;

  //bounds
  if (player.pos.x > 320) {  //exit
    player.pos = { 0, 0 };
    EN_TRACE("Won");
  }
  if (player.pos.x < 0) {  //stops from falling off of the map
    player.pos = { 0, 0 };
  }

  if ((just_pressed(JUMP) || just_pressed(MOVE_UP)) && grounded) {
    player.speed.y = jumpSpeed;
    player.speed.x += player.solidSpeed.x;
    player.speed.y += player.solidSpeed.y;
    grounded = false;

    play_sound("jump");
  }
  if (!grounded) {
    player.animationState = PLAYER_ANIMATE_JUMP;
  }

  if (is_down(MOVE_LEFT)) {
    if (grounded) {
      player.animationState = PLAYER_ANIMATE_RUN;
    }

    float mult = 1.0f;
    if (player.speed.x > 0.0f) {
      mult = 3.0f;
    }
    player.speed.x =
      approach(player.speed.x, -runSpeed, runAcceleration * mult * dt);
  }
  if (is_down(MOVE_RIGHT)) {
    float mult = 1.0f;
    if (player.speed.x < 0.0f) {
      mult = 3.0f;
    }
    player.runAnimateTime += dt;
    player.speed.x =
      approach(player.speed.x, runSpeed, runAcceleration * mult * dt);
  }
  if (player.speed.x > 0) {
    player.renderOptions = 0;
  }
  if (player.speed.x < 0) {
    player.renderOptions = RENDER_OPTION_FLIP_X;
  }

  {
    IRect playerRect = get_player_rect();
    for (int i = 0; i < gameState->static_solids.count; i++) {
      if (inside_static_rect(playerRect, gameState->static_solids[i].pos)) {
        player.pos = { -160, -90 };
        EN_TRACE("player killed");
      }
    }
  }

  //Map
  if (just_pressed(MOUSE_MIDDLE)) {
    player.pos = { 8 * 2, 8 + 2 };
    player.speed = { 0, 0 };
  }

  if (just_pressed(SAVE_STATE)) {  //semicolon
    if (!save_tile_set()) {
      EN_ERROR("couldnt save");
    }
    else {
      EN_TRACE("saved tile set");
    }
  }
  //Physics
  //Friction
  if (!is_down(MOVE_LEFT) && !is_down(MOVE_RIGHT)) {
    if (grounded) {
      player.speed.x = approach(player.speed.x, 0.0f, runReduce * dt);
    }
    else {
      player.speed.x = approach(player.speed.x, 0.0f, flyReduce * dt);
    }
  }

  //gravity
  player.speed.y = approach(player.speed.y, fallSpeed, gravity * dt);
}

void handle_player_physics(Vec2& remainder, bool& grounded)
{
  Player& player = gameState->player;

  // Move X
  {
    IRect playerRect = get_player_rect();

    remainder.x += player.speed.x;
    int moveX = round(remainder.x);
    if (moveX != 0) {
      remainder.x -= moveX;
      int moveSign = sign(moveX);
      bool collisionHappened = false;

      auto movePlayerX = [&] {
        while (moveX) {
          playerRect.pos.x += moveSign;

          {
            for (int solidIdx = 0; solidIdx < gameState->solids.count;
                 solidIdx++) {
              Solid& solid = gameState->solids[solidIdx];
              IRect solidRect = get_solid_rect(solid);

              if (rect_collision(playerRect, solidRect)) {
                player.speed.x = 0;
                return;
              }
            }
          }

          IVec2 playerGridPos = get_grid_pos(player.pos);
          for (int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++) {
            for (int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++) {
              Tile* tile = get_tile(x, y);

              if (!tile || !tile->isVisible) {
                continue;
              }

              IRect tileRect = get_tile_rect(x, y);
              if (rect_collision(playerRect, tileRect)) {
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
    if (moveY != 0) {
      remainder.y -= moveY;
      int moveSign = sign(moveY);
      bool collisionHappened = false;

      auto movePlayerY = [&] {
        while (moveY) {
          playerRect.pos.y += moveSign;

          // Test collision against Solids
          {
            for (int solidIdx = 0; solidIdx < gameState->solids.count;
                 solidIdx++) {
              Solid& solid = gameState->solids[solidIdx];
              IRect solidRect = get_solid_rect(solid);

              if (rect_collision(playerRect, solidRect)) {
                // Moving down/falling
                if (player.speed.y > 0.0f) {
                  grounded = true;
                }

                player.speed.y = 0;
                return;
              }
            }
          }

          // Loop through local Tiles
          IVec2 playerGridPos = get_grid_pos(player.pos);
          for (int x = playerGridPos.x - 1; x <= playerGridPos.x + 1; x++) {
            for (int y = playerGridPos.y - 2; y <= playerGridPos.y + 2; y++) {
              Tile* tile = get_tile(x, y);

              if (!tile || !tile->isVisible) {
                continue;
              }

              IRect tileRect = get_tile_rect(x, y);
              if (rect_collision(playerRect, tileRect)) {
                // Moving down/falling
                if (player.speed.y > 0.0f) {
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

void handle_solid_movement_and_collision(float dt)

{
  Player& player = gameState->player;
  player.solidSpeed = {};

  for (int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++) {
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
      if (moveX != 0) {
        solid.remainder.x -= moveX;
        int moveSign =
          sign(solid.keyframes[nextKeyframeIdx].x -
               solid.keyframes[solid.keyframeIdx].x);  //for direction

        // Move the player in Y until collision or moveY is exausted
        auto moveSolidX = [&] {
          while (moveX) {
            IRect playerRect = get_player_rect();
            bool standingOnTop =
              playerRect.pos.y - 1 + playerRect.size.y == solidRect.pos.y;

            solidRect.pos.x += moveSign;

            //collision happend on left or right, push the player
            bool tileCollision = false;
            if (rect_collision(playerRect, solidRect)) {
              //move the player rect
              playerRect.pos.x += moveSign;
              player.solidSpeed.x =
                solid.speed.x * (float)moveSign * solid.slip / 20.0f;

              // destroy player if there is a collision
              IVec2 playerGridPos = get_grid_pos(player.pos);
              for (int x = playerGridPos.x - 1; x <= playerGridPos.x + 1;
                   x++) {
                for (int y = playerGridPos.y - 2; y <= playerGridPos.y + 2;
                     y++) {
                  Tile* tile = get_tile(x, y);

                  if (!tile || !tile->isVisible) {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if (rect_collision(playerRect, tileRect)) {
                    tileCollision = true;

                    if (!standingOnTop) {
                      //dead
                      player.pos = { WORLD_WIDTH / 2, WORLD_HEIGHT - 24 };
                    }
                  }
                }
              }

              if (!tileCollision) {
                player.pos.x += moveSign;
              }
            }
            solid.pos.x += moveSign;
            moveX -= 1;

            if (solid.pos.x == solid.keyframes[nextKeyframeIdx].x) {
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
      if (moveY != 0) {
        solid.remainder.y -= moveY;
        int moveSign = sign(solid.keyframes[nextKeyframeIdx].y -
                            solid.keyframes[solid.keyframeIdx].y);

        auto moveSolidY = [&] {
          while (moveY) {
            IRect playerRect = get_player_rect();
            solidRect.pos.x += moveSign;

            if (rect_collision(playerRect, solidRect)) {
              player.pos.y += moveSign;
              player.solidSpeed.y = solid.speed.y * (float)moveSign / 40.0f;

              IVec2 playerGridPos = get_grid_pos(player.pos);
              for (int x = playerGridPos.x - 1; x <= playerGridPos.x + 1;
                   x++) {
                for (int y = playerGridPos.y - 2; y <= playerGridPos.y + 2;
                     y++) {
                  Tile* tile = get_tile(x, y);

                  if (!tile || !tile->isVisible) {
                    continue;
                  }

                  IRect tileRect = get_tile_rect(x, y);
                  if (rect_collision(playerRect, tileRect)) {
                    player.pos = { WORLD_WIDTH / 2, WORLD_HEIGHT - 24 };
                  }
                }
              }
            }

            // Move the Solid
            solid.pos.y += moveSign;
            moveY -= 1;

            if (solid.pos.y == solid.keyframes[nextKeyframeIdx].y) {
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

void handle_tile_editing_and_bitmask_update(std::vector<std::pair<int, int>>& loaded_tiles)
{
  bool updateTiles = false;
  if (is_down(MOUSE_LEFT)) {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if (tile) {
      tile->isVisible = true;
      updateTiles = true;
    }
  }

  static int once = 0;  //initial bitmasking for the tiles loaded

  if (is_down(MOUSE_RIGHT)) {
    IVec2 worldPos = screen_to_world(input->mousePos);
    IVec2 mousePosWorld = input->mousePosWorld;
    Tile* tile = get_tile(worldPos);
    if (tile) {
      tile->isVisible = false;
      updateTiles = true;
    }
    save_tile_set();
  }

  //bitmasking algo
  /*
  if (updateTiles || !once) {
    once++;
    ...
    for each tile:
        if tile is visible:
            for each of 12 neighbors:
                if neighbor is visible:
                    set corresponding bit in mask
                else:
                    track the empty slot
            Based on neighbors, adjust mask
} 
            int neighbourOffsets[24] = {
              0,  -1,  -1, 0,   1, 0,   0, 1,     // Up, Left, Right, Down (4 cardinal)
              -1, -1,  1, -1, -1, 1,  1, 1,       // Diagonals (4 total)
              0, -2, -2, 0,   2, 0,   0, 2        // Extended neighbors (4 further away)
            };

  */


  if (updateTiles || !once) {
    once++;
    //bitmapping
    int neighbourOffsets[24] = { 0,  -1, -1, 0,  1,  0, 0, 1,

                                 -1, -1, 1,  -1, -1, 1, 1, 1,

                                 0,  -2, -2, 0,  2,  0, 0, 2 };

    for (int y = 0; y < WORLD_GRID.y; y++) {
      for (int x = 0; x < WORLD_GRID.x; x++) {
        Tile* tile = get_tile(x, y);

        if (!tile->isVisible) {
          continue;
        }

        tile->neighbourMask = 0;
        int neighbourCount = 0;
        int extendedNeighbourCount = 0;
        int emptyNeighbourSlot = 0;

        for (int n = 0; n < 12; n++) {
          Tile* neighbour = get_tile(x + neighbourOffsets[n * 2],
                                     y + neighbourOffsets[n * 2 + 1]);

          if (!neighbour || neighbour->isVisible) {
            tile->neighbourMask |= BIT(n);
            if (n < 8) {
              neighbourCount++;
            }
            else {
              extendedNeighbourCount++;
            }
          }
          else if (n < 8) {
            emptyNeighbourSlot = n;
          }
        }

        if (neighbourCount == 7 &&
            emptyNeighbourSlot >= 4) {  //shift to the next row
          tile->neighbourMask = 16 + (emptyNeighbourSlot - 4);
        }
        else if (neighbourCount == 8 && extendedNeighbourCount == 4) {
          tile->neighbourMask = 20;
        }
        else {
          tile->neighbourMask = tile->neighbourMask & 0b1111;
        }
      }
    }
  }
}

EXPORT_FN void update_game(GameState* gameStateIn,
                           RenderData* renderDataIn,
                           Input* inputIn,
                           float dt,
                           SoundState* soundStateIn)

{
  {
    std::vector<std::pair<int, int>> loaded_tiles = load_tile_set(gameStateIn);
    // if(loaded_tiles.size()<10){
    //   EN_ERROR("NOTHING");
    // }
    if (renderData != renderDataIn) {
      gameState = gameStateIn;
      renderData = renderDataIn;
      input = inputIn;
      soundState = soundStateIn;
    }

    if (!gameState->initialized) {
      renderData->gameCamera.dimensions = { WORLD_WIDTH, WORLD_HEIGHT };
      gameState->initialized = true;

      {
        Player& player = gameState->player;
        player.animationSprites[PLAYER_ANIMATE_IDLE] = SPRITE_CELESTE;
        player.animationSprites[PLAYER_ANIMATE_JUMP] = SPRITE_CELESTE_JUMP;
        player.animationSprites[PLAYER_ANIMATE_RUN] = SPRITE_CELESTE_RUN;
      }
      //tiles
      {
        
        IVec2 tiles_texture_Position = { 48, 0 };
        // IVec2 tiles_texture_Position = { 15, 0 };


        for (int y = 0; y < 5; y++) {
          for (int x = 0; x < 4; x++) {
            gameState->tileCoords.add(
              { tiles_texture_Position.x + x * 8, tiles_texture_Position.y + y * 8 });
          }
        }
        gameState->tileCoords.add({ tiles_texture_Position.x, tiles_texture_Position.y + 5 * 8 });
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
        solid.keyframes.add({ 8 * 3, 8 * 10 });
        solid.keyframes.add({ 8 * 10, 8 * 10 });
        solid.pos = { 8 * 2, 8 * 10 };
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
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 29, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 30, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 31, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 32, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 33, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 34, 8 * 18 + 4 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 4 + 8 * 35, 8 * 18 + 4 };
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
        statSolid.pos = { 12, 8 * 19 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 20, 8 * 19 };
        gameState->static_solids.add(statSolid);

        statSolid = {};
        statSolid.spriteID = SPRITE_MAGMA;
        statSolid.pos = { 28, 8 * 19 };
        gameState->static_solids.add(statSolid);
      }
    }

    {
      gameState->updateTimer += dt;
      while (gameState->updateTimer >= UPDATE_DELAY) {
        gameState->updateTimer -= UPDATE_DELAY;
        simulate(loaded_tiles);

        input->relMouse = input->mousePos - input->prevMousePos;
        input->prevMousePos = input->mousePos;

        {
          for (int keyCode = 0; keyCode < KEY_COUNT; keyCode++) {
            input->keys[keyCode].justReleased = false;
            input->keys[keyCode].justPressed = false;
            input->keys[keyCode].halfTransitionCount = 0;
          }
        }
      }
    }
    float interpolatedDT = (float)(gameState->updateTimer / UPDATE_DELAY);

    {  //solids
      for (int solidIdx = 0; solidIdx < gameState->solids.count; solidIdx++) {
        Solid& solid = gameState->solids[solidIdx];
        IVec2 solidPos = lerp(solid.prevPos, solid.pos, interpolatedDT);
        draw_sprite(solid.spriteID, solidPos);
      }
    }
    {
      for (int statsolidIdx = 0; statsolidIdx < gameState->static_solids.count;
           statsolidIdx++) {
        Static_solids& solid = gameState->static_solids[statsolidIdx];
        draw_sprite(solid.spriteID, solid.pos);
      }
    }

    {  //player
      {
        Player& player = gameState->player;
        IVec2 playerPos = lerp(player.prevPos, player.pos, interpolatedDT);

        Sprite sprite =
          get_sprite(player.animationSprites[player.animationState]);
        int animationIdx =
          animate(&player.runAnimateTime, sprite.frameCount, 0.6f);

        DrawData playerRenderOptions = {};
        playerRenderOptions.animationIdx = animationIdx;  
        playerRenderOptions.renderOptions = player.renderOptions;
        draw_sprite(player.animationSprites[player.animationState],
                    playerPos,playerRenderOptions
        );
      }
    }

    {  //draw tileset
      for (int y = 0; y < WORLD_GRID.y; y++) {
        for (int x = 0; x < WORLD_GRID.x; x++) {
          Tile* tile = get_tile(x, y);

          if (!tile->isVisible) {
            continue;
          }

          Transform transform = {};
          transform.pos = { x * (float)TILESIZE, y * (float)TILESIZE };
          transform.size = { 8, 8 };
          transform.sprite_size = { 8, 8 };
          
          // transform.atlas_offset = gameState->tileCoords[tile->neighbourMask%2];
          transform.atlas_offset = gameState->tileCoords[tile->neighbourMask];
          draw_quad(transform);
        }
      }
    }
  }
}

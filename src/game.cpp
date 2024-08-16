#include "game.h"
#include "assets.h"
#include "input.h"
#include "render_interface.h"

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;

EXPORT_FN void update_game(GameState* gameStateIn,RenderData *renderDataIn,Input* inputIn)
{
  if(renderData != renderDataIn)
  {
    gameState = gameStateIn;
    renderData = renderDataIn;
    input = inputIn;
  }
  if(!gameState->initialized){
    
    renderData->gameCamera.dimensions = {WORLD_WIDTH,WORLD_HEIGHT};
    gameState->initialized = true;
  }

  renderData->gameCamera.position.x = 0;
  renderData->gameCamera.position.y = 0;

  // draw_sprite(SPRITE_DICE, {0.0f, 0.0f});
  
  draw_sprite(SPRITE_DICE, gameState->playerPos);

  if(key_is_down(KEY_A)){
    gameState->playerPos.x-=1;
  }
  if(key_is_down(KEY_D)){
    gameState->playerPos.x+=1;
  }
    if(key_is_down(KEY_W)){
    gameState->playerPos.y-=1;
  }
    if(key_is_down(KEY_S)){
    gameState->playerPos.y+=1;
  }
}

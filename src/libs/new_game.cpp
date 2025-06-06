#include "game.h"
#include "game_libs.h"

EXPORT_FN void update_game(GameState *gameStateIn, RenderData *renderDataIn, Input *inputIn, float dt, SoundState *soundStateIn){
    {
        if(renderData != renderDataIn) {
            gameState = gameStateIn;
            renderData = renderDataIn;
            input = inputIn;
            soundState = soundStateIn;
        }
        if(!gameState->initialized) {
            gameState->initialized = true;
            renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        }
    }
}
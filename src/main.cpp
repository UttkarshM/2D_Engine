
#include <chrono>
#include "libs/core.h"

#include "libs/input.h"

#include "libs/game.h"
#include "libs/sound.h"

#define GL_GLEXT_PROTOTYPES
#include "../third_party/glcorearb.h"

static KeyCodeID KeyCodeLookupTable[KEY_COUNT];

#include "libs/platform.h"
#ifdef _WIN32
#include "libs/windows_platform.cpp"
#endif

#include "libs/gl_renderer.cpp"

// This is the function pointer to update_game in game.cpp
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;

double get_delta_time();

void reload_game_dll(BumpAllocator* transientStorage);


int main()
{
  {
  BumpAllocator transientStorage = make_bump_allocator(MB(600));
  BumpAllocator persistentStorage = make_bump_allocator(MB(600));

  input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));
  if(!input)
  {
    EN_ERROR("Failed to allocate Input");
    return -1;
  }

  renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));
  if(!renderData)
  {
    EN_ERROR("Failed to allocate RenderData");
    return -1;
  }

  //game state

  gameState = (GameState*)bump_alloc(&persistentStorage, sizeof(GameState));
  if(!gameState){
    EN_ERROR("Failed to allocate GameState");
    return -1;
  }
  soundState = (SoundState*)bump_alloc(&persistentStorage, sizeof(SoundState));
  if(!soundState)
  {
    EN_ERROR("Failed to allocate SoundState");
    return -1;
  }
  soundState->transientStorage = &transientStorage;
  soundState->allocatedsoundsBuffer = bump_alloc(&persistentStorage, SOUNDS_BUFFER_SIZE);
  if(!soundState->allocatedsoundsBuffer)
  {
    EN_ERROR("Failed to allocated Sounds Buffer");
    return -1;
  }
  platform_fill_keycode_lookup_table();
  platform_create_window(1280, 720, "Window");
  platform_set_vsync(true);
  if(!platform_init_audio()){
    EN_ERROR( "Failed to initialize Audio");
    return -1;
  }

  gl_init(&transientStorage);

  while(running)
  {
    float dt = get_delta_time();
    reload_game_dll(&transientStorage);

    // Update
    platform_update_window();
    update_game(gameState,renderData, input,dt,soundState);
    gl_render(&transientStorage);
    platform_update_audio(dt);

    platform_swap_buffers();

    transientStorage.used = 0;
  }
  }

  return 0;
}

void update_game(GameState* gameStateIn,RenderData* renderDataIn, Input* inputIn,float dt,SoundState* soundState)
{
  update_game_ptr(gameStateIn, renderDataIn, inputIn, dt,soundState);
}

double get_delta_time(){
  static auto lastTime = std::chrono::steady_clock::now();//since static it compiles it only once
  auto currentTime = std::chrono::steady_clock::now();

  double delta = std::chrono::duration<double>(currentTime - lastTime).count();
  lastTime = currentTime;

  return delta;
}

void reload_game_dll(BumpAllocator* transientStorage)
{
  static void* gameDLL;
  static long long lastEditTimestampGameDLL;

  long long currentTimestampGameDLL = get_timestamp("game.dll");
  if(currentTimestampGameDLL > lastEditTimestampGameDLL)
  {
    if(gameDLL)
    {
      bool freeResult = platform_free_dynamic_library(gameDLL);
      EN_ASSERT(freeResult, "Failed to free game.dll");
      gameDLL = nullptr;
      EN_TRACE("Freed game.dll");
    }

    while(!copy_file("game.dll", "game_load.dll", transientStorage))
    {
      Sleep(10);
    }
    EN_TRACE("Copied game.dll into game_load.dll");

    gameDLL = platform_load_dynamic_library("game_load.dll");
    EN_ASSERT(gameDLL, "Failed to load game.dll");

    update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
    EN_ASSERT(update_game_ptr, "Failed to load update_game function");
    lastEditTimestampGameDLL = currentTimestampGameDLL;
  }
}


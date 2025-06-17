#pragma once
#include "core.h"
#include <stdio.h>

static constexpr int MAX_CONCURRENT_SOUNDS = 16;
static constexpr int SOUNDS_BUFFER_SIZE = MB(128);
static constexpr int PATH_LENGTH = 256;

static constexpr float FADE_DURATION = 1.0f;

enum SoundOptionBits
{
    SOUND_OPTION_FADE_OUT = BIT(0),
    SOUND_OPTION_FADE_IN = BIT(1),
    SOUND_OPTION_START = BIT(2),
    SOUND_OPTION_LOOP = BIT(3)
};

typedef int SoundOptions;

struct Sound{
    char file[PATH_LENGTH];
    SoundOptions options;
    int size;
    char* data;
};

struct SoundState
{
    int bytesUsed;
    char* allocatedsoundsBuffer;

    BumpAllocator* transientStorage;

    Array<Sound, MAX_CONCURRENT_SOUNDS>allocatedSounds;
    Array<Sound,MAX_CONCURRENT_SOUNDS> playingSounds;
};

static SoundState* soundState;





void play_sound(char* soundName,SoundOptions options = 0){
    EN_ASSERT(soundName, "No sound name supplied!");

    options = options? options: SOUND_OPTION_START;
    if(!(options & SOUND_OPTION_START) &&
    !(options & SOUND_OPTION_FADE_IN)  &&
    !(options & SOUND_OPTION_FADE_OUT)){
        options  |= SOUND_OPTION_START;
    }

    Sound sound = {};
    sound.options = options;
    sprintf(sound.file,"assets/sounds/%s.wav",soundName);

    for(int soundIdx=0;soundIdx < soundState->allocatedSounds.count;soundIdx++)
    {
        Sound allocatedSound = soundState->allocatedSounds[soundIdx];
        soundState->playingSounds.add(allocatedSound);
        return;
    }

    //when the sound cant be loaded
    WAVFile* wavFile = load_wav(sound.file, soundState->transientStorage);
	if(wavFile)
	{
		if(wavFile->header.dataChunkSize > SOUNDS_BUFFER_SIZE - soundState->bytesUsed)
		{
			EN_ASSERT(0, "Exausted Sounds Buffer\nCapacity:\t%d\nBytes Used:\t%d\nSound Path:\t%s\nSound Size:\t%d",
									 SOUNDS_BUFFER_SIZE, soundState->bytesUsed, sound.file, wavFile->header.dataChunkSize);
			return;
		}
		sound.size = wavFile->header.dataChunkSize;
		sound.data = &soundState->allocatedsoundsBuffer[soundState->bytesUsed];
		soundState->bytesUsed += sound.size;
		memcpy(sound.data, &wavFile->dataBegin, sound.size);

		soundState->allocatedSounds.add(sound);
		soundState->playingSounds.add(sound);
	}
}

void stop_sound(char* soundName)
{
	play_sound(soundName, SOUND_OPTION_FADE_OUT);
}
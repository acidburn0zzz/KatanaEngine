/*	Copyright (C) 2014 OldTimes Software
*/
#include "platform.h"

/*
	Basic client for testing OpenAL implementation.
*/

#include "AL\al.h"
#include "AL\alc.h"
#include "AL\alext.h"

#define	AUDIO_MAX_BUFFERS	1
#define	AUDIO_MAX_SOURCES	512

bool	bAudioInitialized = false;

ALCdevice	*aAudioDevice;
ALCcontext	*aAudioContext;

ALuint	aAudioBuffer[AUDIO_MAX_BUFFERS];

void Audio_Shutdown(void);

void Audio_Initialize(void)
{
	int iAttributes[]=
	{
		ALC_FREQUENCY,
		22050,
		0
	};

	if(bAudioInitialized)
		return;

	printf("Initializing audio...\n");
	printf(" Device: %s\n",alcGetString(NULL,ALC_DEFAULT_DEVICE_SPECIFIER));

	aAudioDevice = alcOpenDevice(NULL);
	if(!aAudioDevice)
	{
		printf("Failed to open audio device!\nCheck that OpenAL is installed.\n");
		return;
	}

	aAudioContext = alcCreateContext(aAudioDevice,iAttributes);
	if(!aAudioContext)
	{
		printf("Failed to create audio context!\n%s",alcGetString(aAudioDevice,alcGetError(aAudioDevice)));

		Audio_Shutdown();
	}

	alcMakeContextCurrent(aAudioContext);

	alGetError();
	alGenBuffers(AUDIO_MAX_BUFFERS,aAudioBuffer);
	alGetError();

	Audio_UpdateListenerPosition();

	bAudioInitialized = true;
}

void Audio_UpdateListenerPosition(void)
{
#if 0
	alListener3f(AL_POSITION,
		r_refdef.vieworg[X],
		r_refdef.vieworg[Y],
		r_refdef.vieworg[Z]);
	alListener3f(AL_ORIENTATION,
		r_refdef.viewangles[X],
		r_refdef.viewangles[Y],
		r_refdef.viewangles[Z]);
#endif
}

void Audio_Process(void)
{
	Audio_UpdateListenerPosition();
}

void Audio_Shutdown(void)
{
	if(!bAudioInitialized)
		return;

	alDeleteBuffers(sizeof(aAudioBuffer),&aAudioBuffer);

	aAudioContext = alcGetCurrentContext();
	if(aAudioContext)
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(aAudioContext);
	}

	aAudioDevice = alcGetContextsDevice(aAudioContext);

	bAudioInitialized = false;
}

int main(int argc,char *argv[])
{
	Audio_Initialize();

	Audio_Shutdown();

	return 0;
}
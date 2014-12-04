/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "engine_audio.h"

/*
	Audio System
*/
#include "SDL.h"

#define AUDIO_NUM_BUFFERS	1
#define AUDIO_NUM_SOURCES	512

bool bAudioInitialized = false;

void Audio_PlayCommand(void);

void Audio_Initialize(void)
{
	if(bAudioInitialized)
		return;

	Cmd_AddCommand("audio_play",Audio_PlayCommand);

	if(SDL_AudioInit(NULL) < 0)
		Sys_Error("Failed to initialize audio!\n%s\n",SDL_GetError());

	bAudioInitialized = true;
}

/*	Play a specified sound via the console.
*/
void Audio_PlayCommand(void)
{
}

void Audio_PlaySound(AudioSound_t *asSample)
{
	sfxcache_t	*sSoundCache;

	if(!asSample)
		return;

	sSoundCache = S_LoadSound(asSample->sSample);
	if(!sSoundCache)
	{
		Con_Warning("Failed to load sound data (%s)!\n",asSample->sSample->name);
		return;
	}

	Audio.iAudioSource++;
}

AudioSound_t *Audio_LoadSound(sfx_t *sSoundEffect)
{
	AudioSound_t	*asNewSound = NULL;
	byte			*bData,
					bStackBuffer[1*1024];
//	sfxcache_t		*sSoundCache;
	char			cNameBuffer[256];

//	sSoundCache = (sfxcache_t*)Cache_Check(&sSoundEffect->cache);
//	if(sSoundCache)
//		return sSoundCache;

	sprintf(cNameBuffer,"%s",Global.cSoundPath);
	strncat(cNameBuffer,sSoundEffect->name,sizeof(sSoundEffect->name));

	bData = COM_LoadStackFile(cNameBuffer,bStackBuffer,sizeof(bStackBuffer));
	if(!bData)
	{
		Con_Warning("Failed to load %s!\n",cNameBuffer);
		return NULL;
	}

	//------

	return asNewSound;
}

/*	Called per-frame to update listener position and more!
*/
void Audio_Process(void)
{
	vec3_t	vPosition,
			vOrientation;

	if(cls.signon == SIGNONS)
	{
		Math_VectorCopy(r_origin,vPosition);
		Math_VectorCopy(r_refdef.viewangles,vOrientation);
	}
	else
	{
		Math_VectorCopy(vec3_origin,vPosition);
		Math_VectorCopy(vec3_origin,vOrientation);
	}

	// [5/8/2013] Update listener position ~hogsy
}

void Audio_Shutdown(void)
{
	if(!bAudioInitialized)
		return;

	SDL_CloseAudio();

	bAudioInitialized = false;
}

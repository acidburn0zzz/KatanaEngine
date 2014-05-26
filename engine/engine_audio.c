/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "engine_audio.h"

/*
	Audio System
*/
#include "AL/al.h"
#include "AL/alc.h"

#define AUDIO_NUM_BUFFERS	1
#define AUDIO_NUM_SOURCES	512

ALCdevice	*aAudioDevice;
ALCcontext	*aAudioContext;

ALuint	aAudioBuffer[AUDIO_NUM_BUFFERS],
		aAudioEnvironment[2],
		aAudioSource[AUDIO_NUM_SOURCES];

bool bAudioInitialized = false;

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

	Con_Printf("Initializing audio...\n");
	// [9/10/2012] Let us know what device OpenAL is trying to use ~hogsy
	Con_DPrintf(" Device: %s\n",alcGetString(NULL,ALC_DEFAULT_DEVICE_SPECIFIER));

	aAudioDevice = alcOpenDevice(NULL);
	if(!aAudioDevice)
		Sys_Error("Failed to open audio device!\nCheck that OpenAL is installed.\n");

	aAudioContext = alcCreateContext(aAudioDevice,iAttributes);
	if(!aAudioContext)
		Sys_Error("Failed to create audio context!\n");

	alcMakeContextCurrent(aAudioContext);

	// [8/10/2012] Generate our set of buffers we'll use ~hogsy
	alGetError();
	alGenBuffers(AUDIO_NUM_BUFFERS,aAudioBuffer);
	alGetError();

	// [8/10/2012] Set the initial listener position ~hogsy
	alListenerfv(AL_POSITION,vec3_origin);
	alListenerfv(AL_ORIENTATION,vec3_origin);

	bAudioInitialized = true;
}

void Audio_PlaySound(AudioSound_t *asSample)
{
#ifdef KATANA_AUDIO_OPENAL
	sfxcache_t	*sSoundCache;

	if(!asSample)
		return;

	sSoundCache = S_LoadSound(asSample->sSample);
	if(!sSoundCache)
	{
		Con_Warning("Failed to load sound data (%s)!\n",asSample->sSample->name);
		return;
	}

	alGenSources((ALuint)Audio.iAudioSource,&aAudioSource[0]);

	alSourcef(aAudioSource[0],AL_PITCH,asSample->fPitch);
	alSourcef(aAudioSource[0],AL_GAIN,asSample->fGain);
//	alSourcefv(aAudioSource,AL_VELOCITY,vVelocity);
//	alSourcefv(aAudioSource,AL_POSITION,vPosition);
//	alSourcei(aAudioSource,AL_LOOPING,bLoop);
	alSourcei(aAudioSource[0],AL_BUFFER,Audio.iAudioBuffer);

	Audio.iAudioSource++;

	alSourcePlay(Audio.iAudioSource);
#endif
}

sfxcache_t *Audio_LoadSound(sfx_t *sSoundEffect)
{
	byte		*bData,
				bStackBuffer[1*1024];
	sfxcache_t	*sSoundCache;
	char		cNameBuffer[256];

	sSoundCache = (sfxcache_t*)Cache_Check(&sSoundEffect->cache);
	if(sSoundCache)
		return sSoundCache;

	strcpy(cNameBuffer,PATH_SOUNDS"/");
	strcat(cNameBuffer,sSoundEffect->name);

	bData = COM_LoadStackFile(cNameBuffer,bStackBuffer,sizeof(bStackBuffer));
	if(!bData)
	{
		Con_Warning("Failed to load %s!\n",cNameBuffer);
		return NULL;
	}

	//------

	return sSoundCache;
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
	alListenerfv(AL_POSITION,vPosition);
	alListenerfv(AL_ORIENTATION,vOrientation);
}

void Audio_Shutdown(void)
{
	int i;

	if(!bAudioInitialized)
		return;

	// [4/9/2014] Stop all active sources ~hogsy
	for(i = 0; i < Audio.iAudioSource; i++)
		alSourceStop(i);

	aAudioContext = alcGetCurrentContext();
	if(!aAudioContext)
		Sys_Error("Failed to get current audio context!\n");

	aAudioDevice = alcGetContextsDevice(aAudioContext);
	if(!aAudioDevice)
		Sys_Error("Failed to get audio device!\n");

	alcMakeContextCurrent(NULL);
	alcDestroyContext(aAudioContext);
	alcCloseDevice(aAudioDevice);

	bAudioInitialized = false;
}

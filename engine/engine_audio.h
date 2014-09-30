#ifndef __ENGINESOUND__
#define __ENGINESOUND__

#include "quakedef.h"

typedef struct
{
	unsigned	int		iSampleID;	// The ID of the sound.

	sfx_t				*sSample;

	vec3_t				vPosition,
						vVelocity;

	float				fPitch,		// The pitch of the sound.
						fGain;		// The gain/volume of the sound.
} AudioSound_t;

typedef struct Audio_e
{
	unsigned	int	iAudioSource,	// Current source.
					iAudioBuffer;	// Current buffer.

	AudioSound_t	asSample[2048];
} Audio_t;

Audio_t	Audio;

void Audio_Initialize(void);
void Audio_Process(void);
void Audio_Shutdown(void);

#endif

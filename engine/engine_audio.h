#ifndef __ENGINESOUND__
#define __ENGINESOUND__

#include "quakedef.h"

typedef struct
{
	unsigned	int		iSampleID;	// The ID of the sound.

	const		char	*ccSample;	// The path of the sound.

	float				fPitch,		// The pitch of the sound.
						fGain;		// The gain/volume of the sound.
} AudioSound_t;

typedef struct Audio_e
{
	unsigned	int	iAudioSource,	// Current source.
					iAudioBuffer;	// Current buffer.
} Audio_t;

Audio_t	Audio;

void Audio_Initialize(void);
void Audio_Process(void);
void Audio_Shutdown(void);

#endif

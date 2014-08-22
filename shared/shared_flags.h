/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SHAREDFLAGS__
#define __SHAREDFLAGS__

#ifdef _MSC_VER
#pragma warning(disable:4100)	// unreferenced formal parameter
#endif

/*	Define for our future crap.
	Most of the stuff plopped
	within these will be removed
	from the final OpenKatana branch.	*/
//#define   PARANOID				// Speed sapping error checking.
//#define	KATANA_VIDEO_NEXT		// New renderer.
#define KATANA_AUDIO_OPENAL			// OpenAL implementation.
#ifndef _WIN32	// [21/1/2013] Only Linux release is currently using this... ~hogsy
#define	KATANA_NETWORK_NEXT			// New network system.
#endif
#define	LIGHTHACK	// Rushed in lighting method thrown up using OpenGL's built-in features.

/*	Build needs to be updated
	each day that work is done
	on the engine.
	Release needs to be updated
	when a build is released publically.	*/
#define ENGINE_VERSION_MAJOR	0
#define ENGINE_VERSION_MINOR	2
#define ENGINE_VERSION_BUILD	744	// 22/8/2014

#define	MODULE_ENGINE	"engine"
#define MODULE_GAME		"game"
#define	MODULE_MENU		"menu"
#define MODULE_EDITOR	"editor"

#define	PATH_ENGINE			"engine"			// Directory to look in for engine specific modules / assets
#define	PATH_MAPS			"maps"				// Default directory for opening, loading and exporting maps
#define	PATH_SOUNDS			"sounds"			// Default directory for opening, loading and exporting sounds
#define	PATH_LOGS			PATH_ENGINE"/logs"
#define	PATH_SCREENSHOTS	"%s/screenshots"	// Directory for screenshots. Pass current game directory.

#define DIR_PARTICLES	"textures/particles/"
#define DIR_FLARES		"textures/flares/"
#define DIR_EFFECTS		"textures/effects/"

typedef unsigned char byte;

// [27/9/2012] TODO: These were and still are temp, remove ~hogsy
enum
{
	SERVER_CLIENTPOSTTHINK,
	SERVER_PLAYERPRETHINK,
	SERVER_CLIENTCONNECT,
	SERVER_CLIENTDISCONNECT,
	SERVER_CLIENTKILL,
	SERVER_STARTFRAME,
	SERVER_SETCHANGEPARMS,
	SERVER_SETNEWPARMS
};

enum
{
	MSG_BROADCAST,	// Unreliable to all
	MSG_ONE,		// Reliable to one (msg_entity)
	MSG_ALL,		// Reliable to all
	MSG_INIT		// Write to the init string
};

#define CMDLINE_LENGTH	256

#define	MAX_ENT_LEAFS	16

#define	MAX_QPATH	128	// Max length of a quake game pathname
#define	MAX_OSPATH	512	// Max length of a filesystem pathname

/*	These have to be the same
	as they are engine-side so
	do not touch!
*/
enum
{
	RESOURCE_MODEL,		// States that the precache should be used for a model.
	RESOURCE_PARTICLE,	// States that the precache should be used for a particle.
	RESOURCE_FLARE,		// States that the precache should be used for a flare.
	RESOURCE_SOUND,		// States that the precache should be used for a sound.
	RESOURCE_FONT,		// States that the precache should be used for a font.
    RESOURCE_TEXTURE	// States that the precache should be used for a texture.
};

#define CONTENT_SLIME				-4		// Horrible slime to slow us up!
#define CONTENT_LAVA				-5		// Lava... IT BURNS!
#define	CONTENT_SKY					-6		// It's the sky!!

/*	Attenuation is used for the
	Sound function and that
	alone.
*/
#define ATTN_NONE	0	// Played everywhere.
#define ATTN_NORM	1	// Large radius.
#define ATTN_IDLE	2	// Small radius.
#define ATTN_STATIC	3	// Medium radius.

/*	Different sound channels.
*/
enum
{
	CHAN_AUTO,								// Sets the sound to a channel automatically (not recommended)
	CHAN_WEAPON,							// Clearly used for weapons
	CHAN_VOICE,								// Usually used for the players grunts and groans (no not those)
	CHAN_ITEM,								// Clearly used for items in the world
	CHAN_BODY								// Usually used for footsteps and such
};

enum
{
	MOVETYPE_NONE,			// Never moves
	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,			// Fly
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_PUSH,			// no clip to world, push and crush
	MOVETYPE_NOCLIP,		// No colliding with world
	MOVETYPE_FLYMISSILE,	// extra size to monsters
	MOVETYPE_BOUNCE,		// Bounces upon hitting the ground
	MOVETYPE_FLYBOUNCE,		// Fly and bounce
	MOVETYPE_PHYSICS		// Sets the entity to use realistic physics
};

#define	MOVE_NORMAL		0
#define	MOVE_NOMONSTERS	1
#define	MOVE_MISSILE	2

enum
{
	SOLID_NOT,		// Entity isn't solid.
	SOLID_TRIGGER,	// Entity will cause a trigger function.
	SOLID_BBOX,		// Entity is solid.
	SOLID_SLIDEBOX,	// Entity is solid and moves.
	SOLID_BSP
};

#define	FL_FLY				1		// Entity can fly.
#define	FL_SWIM				2		// Entity can swim.
#define FL_ANGLEHACK		4		// Used for setting up dumb angles from editor.
#define	FL_INWATER			16		// Entity is in the water.
#define	FL_GODMODE			64
#define	FL_NOTARGET			128
#define	FL_ITEM				256		// Entity is an item.
#define	FL_ONGROUND			512		// Entity is on the ground.
#define	FL_PARTIALGROUND	1024	// Entity is partially on the ground.
#define	FL_WATERJUMP		2048
#define	FL_JUMPRELEASED		4096	// Entity has released jump.

#define	EF_BRIGHTFIELD		1
#define	EF_MUZZLEFLASH		2
#define	EF_BRIGHTLIGHT 		4
#define	EF_DIMLIGHT 		8
#define	EF_GLOW_RED			16
#define EF_LIGHT_GREEN		64
#define	EF_MOTION_ROTATE	128				// Client-side rotation
#define EF_MOTION_FLOAT		256				// Adds a nice floating motion for the entity
#define EF_GLOW_BLUE		512
#define EF_GLOW_WHITE		1024
#define	EF_PARTICLE_BLOOD	2048
#define	EF_PARTICLE_SMOKE	8192			// Smoke trail.
#define EF_FULLBRIGHT		4096
#define	EF_GRENADE			16384			// leave a trail
#define	EF_GIB				32768			// leave a trail
#define	EF_TRACER			65536			// green split trail
#define	EF_TRACER2			262144			// orange split trail + rotate
#define	EF_TRACER3			524288			// purple trail
#define	EF_INVISIBLE		1048576			// Entity is invisible
#define EF_LIGHT_BLUE		2097152			// A constant blue dynamic light
#define EF_LIGHT_RED		4194304			// A constant red dynamic light

#endif

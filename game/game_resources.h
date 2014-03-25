/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __GAMERESOURCES__
#define	__GAMERESOURCES__

#ifdef GAME_OPENKATANA
#define	VEKTAR_MODEL_STATUE		"models/vektar/statue.md2"
#define	VEKTAR_SOUND_FINDING	"vektar/finding_loop.wav"
#define	VEKTAR_SOUND_FANFARE	"vektar/fanfare.wav"
#define	VEKTAR_SOUND_GRABIT		"vektar/grabit_loop.wav"
#define	VEKTAR_SOUND_GOTIT		"vektar/gotit.wav"

#define PRISONER_MODEL_BODY		"models/prisoner.md2"
#define	PRISONER_MODEL_TORSO	"models/prisoner_torso.md2"
#define	PRISONER_MODEL_LEGS		"models/prisoner_torsoless.md2"
#define PRISONER_SOUND_HELP		"monsters/prisoner/prisoner_help.wav"

#define	INMATER_MODEL_BODY		"models/inmater.md2"

#define	LASERGAT_MODEL_BASE		"models/lasergat_base.md2"
#define	LASERGAT_MODEL_BROKEN	"models/lasergat_broken.md2"
#define	LASERGAT_MODEL_HEAD		"models/lasergat_head.md2"

/*	MODEL:	models/weapons/daikatana/
	SOUND:	weapons/daikatana/
*/
#define	DAIKATANA_MODEL_WORLD	"models/w_daikatana.md2"
#define	DAIKATANA_MODEL_VIEW	"models/weapons/v_daikatana.md2"

/*	MODEL:	models/physics/
	SOUND:	physics/
*/
#define	PHYSICS_SOUND_PATH			"physics/"
#define PHYSICS_SOUND_RICOCHET(a)	sprintf(a,"fx/ricochet%i.wav",rand()%10+1)
#define	PHYSICS_SOUND_SPLASH		PHYSICS_SOUND_PATH"watersplash0.wav"
#define	PHYSICS_SOUND_BODY			PHYSICS_SOUND_PATH"body01.wav"
#define PHYSICS_SOUND_WOOD(a)		sprintf(a,"fx/wood%i.wav",rand()%3+1)
#define	PHYSICS_SOUND_WOOD0			"fx/wood1.wav"
#define	PHYSICS_SOUND_WOOD1			"fx/wood2.wav"
#define	PHYSICS_SOUND_WOOD2			"fx/wood3.wav"
#define	PHYSICS_SOUND_GLASS(a)		sprintf(a,"fx/glass%i.wav",rand()%3+1)
#define	PHYSICS_SOUND_GLASS0		"fx/glass1.wav"
#define	PHYSICS_SOUND_GLASS1		"fx/glass2.wav"
#define	PHYSICS_SOUND_GLASS2		"fx/glass3.wav"
#define	PHYSICS_SOUND_METAL(a)		sprintf(a,"fx/metal%i.wav",rand()%3+1)
#define	PHYSICS_SOUND_METAL0		"fx/metal1.wav"
#define	PHYSICS_SOUND_METAL1		"fx/metal2.wav"
#define	PHYSICS_SOUND_METAL2		"fx/metal3.wav"
#define	PHYSICS_SOUND_ROCK(a)		sprintf(a,PHYSICS_SOUND_PATH"rock%i.wav",rand()%3)
#define	PHYSICS_SOUND_ROCK0			PHYSICS_SOUND_PATH"rock0.wav"
#define	PHYSICS_SOUND_ROCK1			PHYSICS_SOUND_PATH"rock1.wav"
#define	PHYSICS_SOUND_ROCK2			PHYSICS_SOUND_PATH"rock2.wav"
#define	PHYSICS_MODEL_PATH			"models/physics/"
#define	PHYSICS_MODEL_GLASS0		"models/gibs/glass_gibs1.md2"
#define	PHYSICS_MODEL_GLASS1		"models/gibs/glass_gibs2.md2"
#define PHYSICS_MODEL_GLASS2		"models/gibs/glass_gibs3.md2"
#define	PHYSICS_MODEL_WOOD0			"models/gibs/wood_gibs1.md2"
#define	PHYSICS_MODEL_WOOD1			"models/gibs/wood_gibs2.md2"
#define	PHYSICS_MODEL_WOOD2			"models/gibs/wood_gibs3.md2"
#define PHYSICS_MODEL_GIB0			"models/gibs/gib0.md2"
#define PHYSICS_MODEL_GIB1			"models/gibs/gib1.md2"
#define	PHYSICS_MODEL_GIB2			"models/gibs/gib2.md2"

#define	WAYPOINT_MODEL_JUMP		"models/debug/waypoint_jump.md2"
#define WAYPOINT_MODEL_BASE		"models/debug/waypoint_base.md2"
#define WAYPOINT_MODEL_CLIMB	"models/debug/waypoint_climb.md2"
#define WAYPOINT_MODEL_ITEM		"models/debug/waypoint_item.md2"
#define WAYPOINT_MODEL_SPAWN	"models/debug/waypoint_spawn.md2"
#define WAYPOINT_MODEL_SWIM		"models/debug/waypoint_swim.md2"
#define WAYPOINT_MODEL_WEAPON	"models/debug/waypoint_weapon.md2"
#elif GAME_EXAMPLE
#endif

#endif

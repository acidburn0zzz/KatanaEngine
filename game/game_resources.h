/*	Copyright (C) 2011-2015 OldTimes Software
*/
#ifndef __GAMERESOURCES__
#define	__GAMERESOURCES__

/*	MODEL:	models/player/
	SOUND:	player/
*/
#define	PLAYER_SOUND_PAIN(a)	sprintf(a,"player/playerpain%i.wav",rand()%3+1)
#define	PLAYER_SOUND_PAIN0		"player/playerpain1.wav"
#define	PLAYER_SOUND_PAIN1		"player/playerpain2.wav"
#define	PLAYER_SOUND_PAIN2		"player/playerpain3.wav"

/*	MODEL:	models/physics/
	SOUND:	physics/
*/
#define	PHYSICS_SOUND_PATH			"physics/"
#define PHYSICS_SOUND_RICOCHET(a)	sprintf(a,PHYSICS_SOUND_PATH"ricochet%i.wav",rand()%10+1)
#define	PHYSICS_SOUND_RICOCHET0		PHYSICS_SOUND_PATH"ricochet1.wav"
#define	PHYSICS_SOUND_RICOCHET1		PHYSICS_SOUND_PATH"ricochet2.wav"
#define	PHYSICS_SOUND_RICOCHET2		PHYSICS_SOUND_PATH"ricochet3.wav"
#define	PHYSICS_SOUND_RICOCHET3		PHYSICS_SOUND_PATH"ricochet4.wav"
#define	PHYSICS_SOUND_RICOCHET4		PHYSICS_SOUND_PATH"ricochet5.wav"
#define	PHYSICS_SOUND_RICOCHET5		PHYSICS_SOUND_PATH"ricochet6.wav"
#define	PHYSICS_SOUND_RICOCHET6		PHYSICS_SOUND_PATH"ricochet7.wav"
#define	PHYSICS_SOUND_RICOCHET7		PHYSICS_SOUND_PATH"ricochet8.wav"
#define	PHYSICS_SOUND_RICOCHET8		PHYSICS_SOUND_PATH"ricochet9.wav"
#define	PHYSICS_SOUND_RICOCHET9		PHYSICS_SOUND_PATH"ricochet10.wav"
#define	PHYSICS_SOUND_SPLASH		PHYSICS_SOUND_PATH"watersplash0.wav"
#define	PHYSICS_SOUND_BODY			PHYSICS_SOUND_PATH"body01.wav"
#define PHYSICS_SOUND_WOOD(a)		sprintf(a,PHYSICS_SOUND_PATH"wood%i.wav",rand()%3)
#define	PHYSICS_SOUND_WOOD0			PHYSICS_SOUND_PATH"wood0.wav"
#define	PHYSICS_SOUND_WOOD1			PHYSICS_SOUND_PATH"wood1.wav"
#define	PHYSICS_SOUND_WOOD2			PHYSICS_SOUND_PATH"wood2.wav"
#define	PHYSICS_SOUND_GLASS(a)		sprintf(a,PHYSICS_SOUND_PATH"glass%i.wav",rand()%3)
#define	PHYSICS_SOUND_GLASS0		PHYSICS_SOUND_PATH"glass0.wav"
#define	PHYSICS_SOUND_GLASS1		PHYSICS_SOUND_PATH"glass1.wav"
#define	PHYSICS_SOUND_GLASS2		PHYSICS_SOUND_PATH"glass2.wav"
#define	PHYSICS_SOUND_METAL(a)		sprintf(a,PHYSICS_SOUND_PATH"metal%i.wav",rand()%3)
#define	PHYSICS_SOUND_METAL0		PHYSICS_SOUND_PATH"metal0.wav"
#define	PHYSICS_SOUND_METAL1		PHYSICS_SOUND_PATH"metal1.wav"
#define	PHYSICS_SOUND_METAL2		PHYSICS_SOUND_PATH"metal2.wav"
#define	PHYSICS_SOUND_ROCK(a)		sprintf(a,PHYSICS_SOUND_PATH"rock%i.wav",rand()%3)
#define	PHYSICS_SOUND_ROCK0			PHYSICS_SOUND_PATH"rock0.wav"
#define	PHYSICS_SOUND_ROCK1			PHYSICS_SOUND_PATH"rock1.wav"
#define	PHYSICS_SOUND_ROCK2			PHYSICS_SOUND_PATH"rock2.wav"
#define	PHYSICS_SOUND_CONCRETESTEP0	PHYSICS_SOUND_PATH"concrete0_footstep.wav"
#define	PHYSICS_SOUND_CONCRETESTEP1	PHYSICS_SOUND_PATH"concrete1_footstep.wav"
#define	PHYSICS_SOUND_CONCRETESTEP2	PHYSICS_SOUND_PATH"concrete2_footstep.wav"
#define	PHYSICS_SOUND_CONCRETESTEP3	PHYSICS_SOUND_PATH"concrete3_footstep.wav"
#define	PHYSICS_MODEL_PATH			"models/physics/"
#define	PHYSICS_MODEL_GLASS(a)		sprintf(a,PHYSICS_MODEL_PATH"glass_gib%i.md2",rand()%3)
#define	PHYSICS_MODEL_GLASS0		PHYSICS_MODEL_PATH"glass_gib0.md2"
#define	PHYSICS_MODEL_GLASS1		PHYSICS_MODEL_PATH"glass_gib1.md2"
#define PHYSICS_MODEL_GLASS2		PHYSICS_MODEL_PATH"glass_gib2.md2"
#define	PHYSICS_MODEL_WOOD(a)		sprintf(a,PHYSICS_MODEL_PATH"wood_gib%i.md2",rand()%3)
#define	PHYSICS_MODEL_WOOD0			PHYSICS_MODEL_PATH"wood_gib0.md2"
#define	PHYSICS_MODEL_WOOD1			PHYSICS_MODEL_PATH"wood_gib1.md2"
#define	PHYSICS_MODEL_WOOD2			PHYSICS_MODEL_PATH"wood_gib2.md2"
#define	PHYSICS_MODEL_GIB(a)		sprintf(a,PHYSICS_MODEL_PATH"gib%i.md2",rand()%4)
#define PHYSICS_MODEL_GIB0			PHYSICS_MODEL_PATH"gib0.md2"
#define PHYSICS_MODEL_GIB1			PHYSICS_MODEL_PATH"gib1.md2"
#define	PHYSICS_MODEL_GIB2			PHYSICS_MODEL_PATH"gib2.md2"
#define	PHYSICS_MODEL_GIB3			PHYSICS_MODEL_PATH"gib3.md2"
#define	PHYSICS_MODEL_ROCK(a)		sprintf(a,PHYSICS_MODEL_PATH"rock_gib%i.md2",rand()%3)
#define	PHYSICS_MODEL_ROCK0			PHYSICS_MODEL_PATH"rock_gib0.md2"
#define	PHYSICS_MODEL_ROCK1			PHYSICS_MODEL_PATH"rock_gib1.md2"
#define	PHYSICS_MODEL_ROCK2			PHYSICS_MODEL_PATH"rock_gib2.md2"
#define	PHYSICS_MODEL_METAL(a)		sprintf(a,PHYSICS_MODEL_PATH"metal_gib%i.md2",rand()%3)
#define	PHYSICS_MODEL_METAL0		PHYSICS_MODEL_PATH"metal_gib0.md2"
#define	PHYSICS_MODEL_METAL1		PHYSICS_MODEL_PATH"metal_gib1.md2"
#define	PHYSICS_MODEL_METAL2		PHYSICS_MODEL_PATH"metal_gib2.md2"

#define	WAYPOINT_MODEL_JUMP		"models/debug/waypoint_jump.md2"
#define WAYPOINT_MODEL_BASE		"models/debug/waypoint_base.md2"
#define WAYPOINT_MODEL_CLIMB	"models/debug/waypoint_climb.md2"
#define WAYPOINT_MODEL_ITEM		"models/debug/waypoint_item.md2"
#define WAYPOINT_MODEL_SPAWN	"models/debug/waypoint_spawn.md2"
#define WAYPOINT_MODEL_SWIM		"models/debug/waypoint_swim.md2"
#define WAYPOINT_MODEL_WEAPON	"models/debug/waypoint_weapon.md2"

#define PARTICLE_SMOKE(a)   sprintf(a,"smoke%i",rand()%4)
#define PARTICLE_SMOKE0     "smoke0"
#define PARTICLE_SMOKE1     "smoke1"
#define PARTICLE_SMOKE2     "smoke2"
#define PARTICLE_SMOKE3     "smoke3"
#define	PARTICLE_BLOOD(a)	sprintf(a,"blood%i",rand()%4)
#define	PARTICLE_BLOOD0		"blood0"
#define	PARTICLE_BLOOD1		"blood1"
#define	PARTICLE_BLOOD2		"blood2"
#define	PARTICLE_BLOOD3		"blood3"

#define BASE_SOUND_TALK0    "misc/talk1.wav"
#define BASE_SOUND_TALK1    "misc/talk2.wav"
#define BASE_SOUND_TALK2    "misc/talk3.wav"

/*
	OpenKatana Resources
*/

#ifdef GAME_OPENKATANA
/*	MODEL:	models/vektar/
	SOUND:	vektar/
*/
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
#endif

/*
	Adamas Resources
*/

#ifdef GAME_ADAMAS
#define	BLAZER_MODEL_VIEW	"models/weapon.bsp"

#define	HURLER_MODEL_BODY	"models/hurler.bsp"
#endif

#endif

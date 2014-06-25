/*  Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SHAREDGAME__
#define __SHAREDGAME__

#ifndef KATANA
#include "platform.h"
#include "shared_math.h"

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;
#endif

enum
{
	DAMAGE_TYPE_NONE,
	DAMAGE_TYPE_EXPLOSIVE
};

typedef struct edict_s edict_t;

typedef struct
{
	edict_t	*eOther;
	int		pad[28];
	int		self;
	int		world;

	int		iKilledMonsters;
	float	time;
	float	frametime;
	float	force_retouch;
	char	*mapname;
	float	deathmatch;
	float	coop;
	float	teamplay;
	float	serverflags;
	float	total_secrets;
	float	total_monsters;
	float	found_secrets;
	float	parm1;
	float	parm2;
	float	parm3;
	float	parm4;
	float	parm5;
	float	parm6;
	float	parm7;
	float	parm8;
	float	parm9;
	float	parm10;
	float	parm11;
	float	parm12;
	float	parm13;
	float	parm14;
	float	parm15;
	float	parm16;
	float	parm17;
	float	parm18;
	float	parm20;
	float	parm21;
	float	parm22;
	float	parm23;
	float	parm24;
	float	parm25;
	float	parm26;
	float	parm27;
	float	parm28;
	float	parm29;

	vec3_t	v_forward,
			v_up,
			v_right;
} GlobalVariables_t;

#define	ENTITY_MAX_INVENTORY	128

typedef struct
{
	// Weapons
	int			iSecondaryAmmo,iPrimaryAmmo;
	int			iWeaponFrame;

	// Editor
	char		*cClassname,	// The classname of the entity.
				*cName;			// The specified name for the entity.
	char		*noise;
	char		*model;
	int			iHealth;

	// [30/1/2013] Changed from float to int ~hogsy
	int			modelindex;
	vec3_t		absmin;
	vec3_t		absmax;
	float		ltime;			// Local time for ents.
	// [20/7/2012] Changed to an integer ~hogsy
	int			movetype;
	vec3_t		origin;
	vec3_t		oldorigin;
	vec3_t		velocity;
	vec3_t		angles;
	vec3_t		avelocity;
	vec3_t		punchangle;

	// [20/10/2013] Changed from a float to an integer ~hogsy
	int			frame;
	float		skin;
	int			effects;
	vec3_t		mins;
	vec3_t		maxs;
	vec3_t		size;
	void		(*TouchFunction)(edict_t *eEntity,edict_t *eOther);
	void		(*use)(edict_t *ent);
	void		(*think)(edict_t *ent);
	void		(*blocked)(edict_t *ent,edict_t *other);
	double		dNextThink;
	edict_t		*groundentity;

	// [21/10/2012] Changed to an integer ~hogsy
	int			iScore;
	int			iActiveWeapon;	// Current active weapon ID
	char		*cViewModel;	// The model that appears in our view, this is traditionally used for weapons.

	int			items;
	int			iInventory[ENTITY_MAX_INVENTORY];

	vec4_t		vLight;

	bool		bTakeDamage;
	// [20/8/2012] Changed from an int to edict ~hogsy
	edict_t		*chain;
	vec3_t		view_ofs;
	vec3_t		button;
	float		impulse;
	float		fixangle;
	vec3_t		v_angle;
	float		idealpitch;
	char		*netname;
	edict_t		*enemy;     // Obsolete
	int			flags;
	float		colormap;
	float		team;
	// [27/1/2013] Changed from float to integer (and renamed to iMaxHealth) ~hogsy
	int			iMaxHealth;
	float		teleport_time;

	int			iArmorType,iArmorValue;

	float		waterlevel;
	int			watertype;
	float		ideal_yaw;
	float		yaw_speed;
	int			aiment;
	// [9/7/2012] Changed from float to an integer ~hogsy
	int			spawnflags;
	char		*targetname;

	float		dmg_take;
	float		dmg_save;
	edict_t		*eDamageInflictor;

	char		*message;
	float		sounds;

	// Physics
	vec3_t		movedir;

	// Misc
	vec3_t		vForward;
} entvars_t;

// [12/1/2013] Model specific variables ~hogsy
typedef struct
{
	float	fScale;
} ModelVariables_t;

typedef struct
{
	/*	Move the following here...
		movetype
		velocity
		avelocity
		solid
	*/

	float	fMass,		// Sets the mass of the entity.
			fGravity,	// Sets the gravity which is enforced on the entity.
			fFriction;	// Sets the amount of friction that effects this entity and others.

	int	iSolid;			// Sets the collision/solid type for the entity.

	edict_t	*eIgnore;	// Tells the entity to ignore collisions with this entity.
} PhysicsVariables_t;

// Monster specific variables
typedef struct
{
	// [15/7/2012] Position we'll be moving to ~hogsy
	vec3_t	vTarget,
			vOldTarget;

	int		iType;

	// Think functions
//	void	(*think_stand)(edict_t *ent);
//	void	(*think_run)(edict_t *ent);
//	void	(*think_attack)(edict_t *ent);
	void	(*think_die)(edict_t *ent,edict_t *other);
	void	(*think_pain)(edict_t *ent,edict_t *other);
//	void	(*think_walk)(edict_t *ent);

	void	(*Think)(edict_t *ent);	// Called per-frame for the specific handling of states for each monster.

	// Current thought state
	int		iState,					// Primary physical state.
			iThink;					// Primary thought state.
	int		iCommandList[64];		// List of sub-commands for the monster to execute.
	float	fEmotion[10],			// Current emotion states.
			fAffect;				// Chance counter which helps determine an action based on the emotion.

	float	fViewDistance;			// Distance in which a monster can detect a target within.

	// Targets
	edict_t	*eEnemy,
			*eOldEnemy;
	edict_t	*eFriend,
			*eOldFriend;
	edict_t	*eTarget,
			*eOldTarget;
} MonsterVariables_t;

/*	Variables used for vehicles.
*/
typedef struct
{
	int		iPassengers,									// Current number of passengers.
			iMaxPassengers;									// Maximum passengers the vehicle can carry.
	int		iSlot[16];
	int		iFuel,											// Current amount of fuel in the vehicle.
			iMaxFuel;										// Maximum amount of fuel that can be in the vehicle.
	int		iType;											// Type of vehicle.

	float	fMaxSpeed;										// Maximum velocity that the vehicle can travel.

	bool	bActive;										// Is the vehicle turned on or not?

	void	(*Enter)(edict_t *eVehicle,edict_t *eOther);	// Function to call when a player enters the vehicle.
	void	(*Exit)(edict_t *eVehicle,edict_t *eOther);		// Function to call when a player leaves the vehicle.
	void	(*Kill)(edict_t *eVehicle,edict_t *eOther);		// Function to call when the vehicle is destroyed.
} VehicleVariables_t;

/*	These are tied in with
	the different flag types
	so please be careful when
	modifying them.
*/
typedef enum
{
	TEAM_NEUTRAL,				// Team neutral, set for deathmatch or other non-team modes
	TEAM_RED,					// For team-based gameplay
	TEAM_BLUE,					// For team-based gameplay
	TEAM_SPECTATOR				// Fags who don't want to play
} PlayerTeam_t;

typedef struct
{
	void	(*Function)(edict_t *eEntity);

	int		iFrame;

	float	fSpeed;

	bool	bIsEnd;
} EntityFrame_t;

// Misc local variables
typedef struct
{
	// mapping features
	char		*sound;
	float		speed;
	int			iDamage,	// Amount of damage to inflict.
				iValue;
	int			volume;		// Volume level for sound.
	float		lip;
	int			distance;
	int			style;
	// [21/2/2014] Changed to a double and renamed ~hogsy
	double		dWait;
	int			count;
	float		red,
				green,
				blue;
	char		*cTarget1,		// First target.
				*cTarget2;		// Second target.

	char		*cSoundStart,
				*cSoundStop,
				*cSoundMoving,
				*cSoundReturn;

#ifdef OPENKATANA
	// Powerups
	double		power_finished,power_time,		// Power Boost.
				speed_finished,speed_time,		// Speed Boost.
				acro_finished,acro_time,		// Acro Boost.
				attackb_finished,attackb_time,	// Attack Boost.
				vita_finished,vita_time;		// Vita Boost.

	// Ammo
	int			iC4Ammo,
				claw_ammo;
	int			trident_ammo;
	int			axe_ammo;
	int			glock_ammo,glock_ammo2;
	int			ionblaster_ammo;
	int			discus_ammo,discus_ammo2;
	int			pulserifle_ammo;
	int			hermes_ammo;
	int			cordite_ammo;
	int			barrier_ammo;
	int			ballista_ammo;
	int			meta_ammo;
	int			midas_ammo;
	int			kineticore_ammo,kineticore_ammo2;
	int			crossbow_ammo;
	int			shockwave_ammo;
	int			sidewinder_ammo;
	int			shotcycler_ammo;
	int			iShotCycle;						// Number of shots to cycle through (shotcycler).
	int			slugger_ammo,slugger_ammo2;
	int			greekfire_ammo;
	int			nightmare_ammo;
	int			wyndrax_ammo;
	int			zeus_ammo;
	int			tazerhook_ammo;
#elif GAME_ADAMAS
	int		iBlazerAmmo;
#endif

	// Animation
	int				iAnimationCurrent,			// Current frame of current sequence.
					iAnimationEnd;				// Last frame of current sequence.
	double			dAnimationTime;				// The speed of the animation.
	EntityFrame_t	*iFrames;					// Active frame group.
	int				iWeaponAnimationCurrent,	// Current frame of current sequence.
					iWeaponAnimationEnd,		// Last frame of current sequence.
					iWeaponIdleFrame;			// Frame to return to for "idling" after sequence.
	float			fWeaponAnimationTime;		// The speed of the animation.
	EntityFrame_t	*iWeaponFrames;				// Active weapon frame group.

	// Misc
	char			*cInfoMessage;				// see server_point > Point_InfoMessage.
	bool			bBleed;						// Do we bleed?

	int				iDamageType;				// The type of damage this entity can recieve.

	double			damage_time,				// Time between each amount of damage.
					dStepTime;					// Time between each step.
	double			dPainFinished,
					dAirFinished,
					dAttackFinished,			// Time before we can attack again.
					dMoveFinished;

	float			jump_flag;
	float			swim_flag;					// Time before next splash sound.
	int				hit;
	int				state;						// Main state.
	float			fSpawnDelay;				// Delay before next spawn.
	vec3_t			pos1;
	vec3_t			pos2;
	char			*deathtype;
	PlayerTeam_t	pTeam;						// Current active team.
	void			(*think1)(edict_t *ent,edict_t *other);
	vec3_t			finaldest;
	char			*killtarget;
	float			delay;			// Delay before doing a task.
	edict_t			*trigger_field;
	int				iFireMode;		// Active fire mode for weapons.
	edict_t			*bomb,
					*activator;
	char			*cOldModel;		// Last model.
	char			cOldStyle;

	// CTF states
	edict_t		*flag;				// Currently owned flag (if any).
	edict_t		*eOwner;			// Owner entity (we usually don't collide with this guy)

	// Door shizz

	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	// state data
	vec3_t		dir;

	// [4/8/2012] Set up by Math_AngleVectors ~hogsy
	vec3_t		vRight,
				vUp;

	int			iDelta[2][2];			// X & Y Delta.

	// Vehicles
	edict_t		*eVehicle;				// Current vehicle.
	int			iVehicleSlot;			// Occupied vehicle slot.

	// Weapons
	int	iBarrelCount;			// For cycling barrel animations.
} ServerVariables_t;

//----------------------------

#define TE_EXPLOSION		3

// [19/5/2013] Fixed this path ~hogsy
#include "shared_client.h"

typedef struct
{
	vec3_t	normal;
	float	dist;
} plane_t;

typedef struct
{
	bool	bAllSolid,		// If true, plane is not valid.
			bStartSolid,	// If true, the initial point was in a solid area.
			bOpen,bWater;

	float	fraction;		// time completed, 1.0 = didn't hit anything
	vec3_t	endpos;			// final position
	plane_t	plane;			// surface normal at impact
	edict_t	*ent;			// entity the surface is on
} trace_t;

/*	If this is changed remember
	to recompile the engine too!    */
typedef struct edict_s
{
	// Shared
	bool				free;
	link_t				area;
	int					num_leafs;
	short				leafnums[MAX_ENT_LEAFS];
	entity_state_t		baseline;
	unsigned char		alpha;
	bool				bSendInterval;
	float				freetime;
	entvars_t			v;
	ModelVariables_t	Model;
	PhysicsVariables_t	Physics;

	// Server
	ServerVariables_t	local;		// [12/4/2013] TODO: Rename to Server! ~hogsy
	MonsterVariables_t	monster;
	VehicleVariables_t	Vehicle;

	// Client (todo: merge with entity_t?)
} edict_t;

// [13/1/2013] Revised ~hogsy
// [25/7/2013] Used intptr to solve gcc warning ~hogsy
// [25/9/2013] Moved here so we can use it in the game-code too :) ~hogsy
#define	FIELD(y)	(intptr_t)&(((edict_t*)0)->y)

typedef struct
{
	// Server
	int			(*Server_PointContents)(vec3_t point);
	void		(*Server_MakeStatic)(edict_t *ent);
	void		(*Server_BroadcastPrint)(char *fmt,...);														// Sends a message to all clients.
	void		(*Server_SinglePrint)(edict_t *eEntity,char *cMessage);											// Sends a message to a specified client.
	void		(*Server_PrecacheResource)(int iType,const char *ccResource);									// Precaches the specified resource.
	void		(*Server_Restart)(void);																		// Restarts the server.
	void		(*Server_ChangeLevel)(const char *ccNewLevel);													// Changes the level.
	void		(*Server_AmbientSound)(vec_t *vPosition,const char *cPath,int iVolume,int iAttenuation);
	trace_t		(*Server_Move)(vec3_t start,vec3_t mins,vec3_t maxs,vec3_t end,int type,edict_t *passedict);
	edict_t		*(*Server_FindRadius)(vec3_t origin,float radius);												// Finds entities within a specific radius.
	edict_t		*(*Server_FindEntity)(edict_t *eStartEntity,char *cName,bool bClassname);						// Finds a specified entity either by classname or by entity name.

	// Client
	int				(*Client_GetEffect)(const char *cPath);					// Get an effect index.
	int				(*Client_GetStat)(ClientStat_t csStat);					// Get a client statistic (health etc.)
	void			(*Client_PrecacheResource)(int iType,char *cResource);	// Precache a resource client-side.
	void			(*Client_SetMenuCanvas)(int iCanvas);					// Set the canvas type that the menu will use.
	void			(*Client_AddMenuState)(int iState);						// Adds a new state to the clients menu.
	void			(*Client_RemoveMenuState)(int iState);					// Removes a state from the clients menu.
	DynamicLight_t	*(*Client_AllocateDlight)(int key);						// Allocate a new dynamic light.
	Particle_t		*(*Client_AllocateParticle)(void);						// Allocate a new particle effect.

	// Global
	vec_t			*(*GetLightSample)(vec3_t vOrigin);						// Gets the current lightmap sample for the specified entity.

	// Pre 9/4/2012 (Update all these)
	void	(*Con_Printf)(char *fmt,...);	// Appears to client in console. Standard message.
	void	(*Con_DPrintf)(char *fmt,...);	// Only appears if launched/running in developer mode.
	void	(*Con_Warning)(char *fmt,...);	// Highlighted message to indicate an issue.

	// [28/7/2012] Added SetMessageEntity ~hogsy
	void	(*SetMessageEntity)(edict_t *ent);
	void	(*CenterPrint)(edict_t *ent,char *msg);	// Sends a message to the specified client and displays the message at the center of the screen.
	void	(*Sys_Error)(char *error,...);
	void	(*SetModel)(edict_t *ent,char *m);		// Sets the model for the specified entity.
	void	(*Particle)(float org[3],float dir[3],float scale,char *texture,int count);
	void	(*Flare)(vec3_t org,float r,float g,float b,float a,float scale,char *texture);
	void	(*Sound)(edict_t *ent,int channel,char *sample,int volume,float attenuation);
	void	(*LinkEntity)(edict_t *ent,bool touch_triggers);
	void	(*UnlinkEntity)(edict_t *ent);
	void	(*FreeEntity)(edict_t *ed);
	void	(*DrawPic)(char *path,float alpha,int x,int y,int w,int h);
	void	(*DrawString)(int x,int y,char *msg);
	void	(*DrawFill)(int x,int y,int w,int h,float r,float g,float b,float alpha);
	void 	(*Cvar_RegisterVariable)(cvar_t *variable,void (*Function)(void));
	void	(*Cvar_SetValue)(char *var_name,float value);
	void	(*LightStyle)(int style,char *val);
	void	(*MakeVectors)(vec3_t angles);
	void	(*Cmd_AddCommand)(char *cmd_name,void (*function)(void));
	void	(*WriteByte)(int mode,int command);
	void	(*WriteCoord)(int mode,float f);
	void	(*WriteAngle)(int mode,float f);
	void	(*WriteEntity)(int mode,edict_t *ent);
	void	(*ShowCursor)(bool bShow);

	int		(*ReadByte)(void);
	float	(*ReadCoord)(void);
	int		(*GetScreenWidth)(void);	// Returns the active screen width.
	int		(*GetScreenHeight)(void);	// Returns the active screen height.

	void	(*GetCursorPosition)(int *X,int *Y);

	float	*(*Aim)(edict_t *ent);

	edict_t	*(*Spawn)(void);
} ModuleImport_t;

typedef struct
{
	//	Game
	char	*Name;																						// Name of the currently active game (used as the name for the window).
	bool	(*Game_Init)(int state,edict_t *ent,double dTime);											// For both server-side and client-side entry
	void	(*ChangeYaw)(edict_t *ent);
	void	(*SetSize)(edict_t *ent,float mina,float minb,float minc,float maxa,float maxb,float maxc);	// Sets the size of an entity.

	void	(*Client_Initialize)(void);
	void	(*Client_RelinkEntities)(entity_t *ent,int i,double dTime);
	void	(*Client_ParseTemporaryEntity)(void);

	void	(*Server_Initialize)(void);																	// Initializes the server.
	void	(*Server_CheckWaterTransition)(edict_t *ent);
	void	(*Server_CheckVelocity)(edict_t *ent);														// Checks the velocity of physically simulated entities.
	void	(*Server_EntityFrame)(edict_t *eEntity);
	void	(*Server_KillClient)(edict_t *eClient);														// Tells the specified client to die.
	void	(*Server_SetSizeVector)(edict_t *eEntity,vec3_t vMin,vec3_t vMax);							// Set the size of an entity by vector.
	void	(*Server_SpawnPlayer)(edict_t *ePlayer);													// Spawns the player (SERVER_PUTCLIENTINSERVER).
	bool	(*Server_SpawnEntity)(edict_t *ent);														// Puts a specific entity into the server

	/*	Shared
	*/
	int		Version;				// Used to get the version of the module.

	void	(*Initialize)(void);	// Called when the module is initialized.
	void	(*Draw)(void);			// Called during video processing.
	void	(*Shutdown)(void);		// Called when the engine begins to uninitialize.
} ModuleExport_t;

#endif

/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SERVERMONSTER__
#define __SERVERMONSTER__

#include "server_main.h"

typedef struct
{
	int	iFirstType,iSecondType,	// The first and second monster types, which determines the encounter we might be dealing with.
		iRelationship;			// The relationship type between the two.
} MonsterRelationship_t;

#define MONSTER_NONE		0	// Used for entities that should not be treated as monsters.
// [28/6/2013] Changed from enum since these are used for spawns ~hogsy
#define MONSTER_PLAYER		1	// Players / clients / bots
#define	MONSTER_VEHICLE		2	// Vehicles
// [15/8/2013] Anything greater than 2 is assumed to be an enemy by the engine ~hogsy
// [28/6/2013] Move into their own header ~hogsy
#ifdef OPENKATANA
#define	MONSTER_PRISONER	3
#define	MONSTER_LASERGAT	4
#define	MONSTER_INMATER		5
#endif

/*	Various states for the AI.
*/
typedef enum
{
	STATE_DEAD		= 1,	// Monster is dead
	STATE_AWAKE,			// Monster is awake
	STATE_ASLEEP,			// Monster is asleep

	STATE_NONE				// Monster has no state
} MonsterState_t;

// [21/9/2012] Changed over to use floats instead, we're counting distance here! ~hogsy
#define	MONSTER_RANGE_MELEE		48.0f
#define MONSTER_RANGE_NEAR		256.0f
#define MONSTER_RANGE_MEDIUM	700.0f
#define MONSTER_RANGE_FAR		2000.0f

/*	Relationship states.
	Neutral is the default relationship state.
*/
enum
{
	RELATIONSHIP_HATE,
	RELATIONSHIP_NEUTRAL,
	RELATIONSHIP_LIKE
};

/*	Various thought processes.
*/
typedef enum
{
	THINK_IDLE = 1,		// Monster is idle
	THINK_FLEEING,		// Monster is fleeing
	THINK_PURSUING,		// Monster is following
	THINK_ATTACKING,	// Monster is attacking
	THINK_WANDERING		// Monster is just walking around
} MonsterThink_t;

/*	Return types for setting states and thought processes.
*/
enum
{
	MONSTER_RETURN_NEWTHINK,
	MONSTER_RETURN_FAIL
};

/*	Various emotions.
	Mostly intended for Ictus.
	[8/2/2013]
	Removed surprise from the list ~hogsy
*/
enum
{
#if 1
	EMOTION_BOREDOM,
	EMOTION_FEAR,
	EMOTION_ANGER,
	EMOTION_JOY,
	EMOTION_INTEREST,
#else
	// [24/8/2012] Revised list ~hogsy
	EMOTION_FEAR,
	EMOTION_JOY,
	EMOTION_ANGER,
	EMOTION_SADNESS,
	EMOTION_DISGUST,
	EMOTION_SURPRISE,
	EMOTION_CONTEMPT,
	EMOTION_INTEREST
#endif

	EMOTION_NONE
};

edict_t	*Monster_GetTarget(edict_t *eMonster);

bool	Monster_WalkMove(edict_t *ent,float yaw,float dist);
bool	Monster_SetEnemy(edict_t *eMonster);
bool	Monster_SetState(edict_t *eMonster,MonsterState_t msState);
bool	Monster_SetThink(edict_t *eMonster,MonsterThink_t mtThink);
bool	Monster_IsVisible(edict_t *ent,edict_t *target);

int	Monster_GetRelationship(edict_t *eMonster,edict_t *eTarget);

float MONSTER_GetRange(edict_t *ent,vec3_t target);

void MONSTER_Damage(edict_t *target,edict_t *inflictor,int iDamage);
void Monster_Jump(edict_t *eMonster,float fVelocity);
void Monster_MoveToGoal(edict_t *ent,vec3_t goal,float distance);
void Monster_Frame(edict_t *eMonster);
void Monster_SetTargets(edict_t *eMonster);
void Monster_Killed(edict_t *eTarget,edict_t *eAttacker);

#endif

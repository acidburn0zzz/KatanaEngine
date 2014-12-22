/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_player.h"

/*
	Code specific towards the player. This includes code for
	spawning, jumping, animating and everything else except
	for physics.
*/

#include "server_waypoint.h"
#include "server_vehicle.h"
#include "server_weapon.h"
#include "server_item.h"

#define	PLAYER_MAX_HEALTH	cvServerMaxHealth.iValue
#define	PLAYER_MIN_HEALTH	-20

// [10/4/2013] Moved entity frames here so that bots can refer to them! ~hogsy
EntityFrame_t PlayerAnimation_Idle[] =
{
	{	NULL,	0,	0.1f			},
	{   NULL,	1,	0.1f			},
	{   NULL,	2,	0.1f			},
	{   NULL,	3,	0.1f			},
	{   NULL,	4,	0.1f			},
	{   NULL,	5,	0.1f			},
	{   NULL,	6,	0.1f			},
	{   NULL,	7,	0.1f			},
	{   NULL,	8,	0.1f			},
	{   NULL,	9,	0.1f,	true    }
};

EntityFrame_t PlayerAnimation_Fire[] =
{
	{   NULL,	10, 0.02f			},
	{   NULL,	11, 0.02f			},
	{   NULL,	12, 0.02f			},
	{   NULL,	13, 0.02f			},
	{   NULL,	14, 0.02f			},
	{   NULL,	15, 0.02f			},
	{   NULL,	16, 0.02f			},
	{   NULL,	17, 0.02f			},
	{   NULL,	18,	0.02f			},
	{	NULL,	19,	0.02f,	true	}
};

EntityFrame_t PlayerAnimation_Walk[] =
{
	{	NULL,	20,	0.02f			},
	{   NULL,	21, 0.02f			},
	{   NULL,	22, 0.02f			},
	{   NULL,	23, 0.02f			},
	{   NULL,	24, 0.02f			},
	{   NULL,	25, 0.02f			},
	{   NULL,	26, 0.02f			},
	{   NULL,	27, 0.02f			},
	{   NULL,	28, 0.02f			},
	{   NULL,	29, 0.02f			},
	{   NULL,	30, 0.02f			},
	{   NULL,	31, 0.02f			},
	{   NULL,	32, 0.02f			},
	{   NULL,	33, 0.02f			},
	{   NULL,	34, 0.02f			},
	{   NULL,	35, 0.02f			},
	{   NULL,	36, 0.02f			},
	{   NULL,	37, 0.02f			},
	{   NULL,	38, 0.02f			},
	{   NULL,	39, 0.02f			},
	{   NULL,	40, 0.02f			},
	{   NULL,	41, 0.02f			},
	{   NULL,	42, 0.02f			},
	{   NULL,	43, 0.02f			},
	{   NULL,	44, 0.02f			},
	{   NULL,	45, 0.02f			},
	{   NULL,	46, 0.02f,	true	}
};

EntityFrame_t PlayerAnimation_Death1[] =
{
	{   NULL, 56, 0.02f    },
	{   NULL, 57, 0.02f    },
	{   NULL, 58, 0.02f    },
	{   NULL, 59, 0.02f    },
	{   NULL, 60, 0.02f    },
	{   NULL, 61, 0.02f    },
	{   NULL, 62, 0.02f    },
	{   NULL, 63, 0.02f    },
	{   NULL, 64, 0.02f    },
	{   NULL, 65, 0.02f    },
	{   NULL, 66, 0.02f    },
	{   NULL, 67, 0.02f    },
	{   NULL, 68, 0.02f    },
	{   NULL, 69, 0.02f    },
	{   NULL, 70, 0.02f    },
	{   NULL, 71, 0.02f    },
	{   NULL, 72, 0.02f    },
	{   NULL, 73, 0.02f    },
	{   NULL, 74, 0.02f    },
	{   NULL, 75, 0.02f    },
	{   NULL, 76, 0.02f    },
	{   NULL, 77, 0.02f    },
	{   NULL, 78, 0.02f, true    }
};

EntityFrame_t PlayerAnimation_Death2[] =
{
	{   NULL, 79, 0.02f    },
	{   NULL, 80, 0.02f    },
	{   NULL, 81, 0.02f    },
	{   NULL, 82, 0.02f    },
	{   NULL, 83, 0.02f    },
	{   NULL, 84, 0.02f    },
	{   NULL, 85, 0.02f    },
	{   NULL, 86, 0.02f    },
	{   NULL, 87, 0.02f    },
	{   NULL, 88, 0.02f    },
	{   NULL, 89, 0.02f    },
	{   NULL, 90, 0.02f    },
	{   NULL, 91, 0.02f    },
	{   NULL, 92, 0.02f    },
	{   NULL, 93, 0.02f    },
	{   NULL, 94, 0.02f    },
	{   NULL, 95, 0.02f    },
	{   NULL, 96, 0.02f    },
	{   NULL, 97, 0.02f    },
	{   NULL, 98, 0.02f    },
	{   NULL, 99, 0.02f, true    }
};

EntityFrame_t PlayerAnimation_Jump[] =
{
	{   NULL, 100, 0.02f    },
	{   NULL, 101, 0.02f    },
	{   NULL, 102, 0.02f    },
	{   NULL, 103, 0.02f    },
	{   NULL, 104, 0.02f    },
	{   NULL, 105, 0.02f    },
	{   NULL, 106, 0.02f    },
	{   NULL, 107, 0.02f    },
	{   NULL, 108, 0.02f    },
	{   NULL, 109, 0.02f    },
	{   NULL, 110, 0.02f    },
	{   NULL, 111, 0.02f    },
	{   NULL, 112, 0.02f    },
	{   NULL, 113, 0.02f    },
	{   NULL, 114, 0.02f    },
	{   NULL, 115, 0.02f    },
	{   NULL, 116, 0.02f    },
	{   NULL, 117, 0.02f    },
	{   NULL, 118, 0.02f    },
	{   NULL, 119, 0.02f    },
	{   NULL, 120, 0.02f    },
	{   NULL, 121, 0.02f    },
	{   NULL, 122, 0.02f    },
	{   NULL, 123, 0.02f    },
	{   NULL, 124, 0.02f    },
	{   NULL, 125, 0.02f    },
	{   NULL, 126, 0.02f    },
	{   NULL, 127, 0.02f    },
	{   NULL, 128, 0.02f    },
	{   NULL, 129, 0.02f, true    }
};

EntityFrame_t PlayerAnimation_RunJump[] =
{
	{   NULL, 130, 0.02f    },
	{   NULL, 131, 0.02f    },
	{   NULL, 132, 0.02f    },
	{   NULL, 133, 0.02f    },
	{   NULL, 134, 0.02f    },
	{   NULL, 135, 0.02f    },
	{   NULL, 136, 0.02f    },
	{   NULL, 137, 0.02f    },
	{   NULL, 138, 0.02f    },
	{   NULL, 139, 0.02f    },
	{   NULL, 140, 0.02f    },
	{   NULL, 141, 0.02f    },
	{   NULL, 142, 0.02f    },
	{   NULL, 143, 0.02f    },
	{   NULL, 144, 0.02f    },
	{   NULL, 145, 0.02f    },
	{   NULL, 146, 0.02f    },
	{   NULL, 147, 0.02f    },
	{   NULL, 148, 0.02f, true    }
};

EntityFrame_t PlayerAnimation_KatanaIdle[] =
{
	{   NULL, 150, 0.02f},
	{   NULL, 151, 0.02f},
	{   NULL, 152, 0.02f},
	{   NULL, 153, 0.02f},
	{   NULL, 154, 0.02f},
	{   NULL, 155, 0.02f},
	{   NULL, 156, 0.02f},
	{   NULL, 157, 0.02f},
	{   NULL, 158, 0.02f},
	{   NULL, 159, 0.02f,  true    }
};

EntityFrame_t PlayerAnimation_KatanaAttack1[] =
{
	{   NULL, 161, 0.02f},
	{   NULL, 162, 0.02f},
	{   NULL, 163, 0.02f},
	{   NULL, 164, 0.02f},
	{   NULL, 165, 0.02f},
	{   NULL, 166, 0.02f},
	{   NULL, 167, 0.02f},
	{   NULL, 168, 0.02f},
	{   NULL, 169, 0.02f},
	{   NULL, 170, 0.02f},
	{   NULL, 171, 0.02f},
	{   NULL, 172, 0.02f},
	{   NULL, 173, 0.02f},
	{   NULL, 174, 0.02f},
	{   NULL, 175, 0.02f},
	{   NULL, 176, 0.02f},
	{   NULL, 177, 0.02f},
	{   NULL, 178, 0.02f},
	{   NULL, 179, 0.02f},
	{   NULL, 180, 0.02f},
	{   NULL, 181, 0.02f},
	{   NULL, 182, 0.02f},
	{   NULL, 183, 0.02f},
	{   NULL, 184, 0.02f},
	{   NULL, 185, 0.02f},
	{   NULL, 186, 0.02f},
	{   NULL, 187, 0.02f},
	{   NULL, 188, 0.02f},
	{   NULL, 189, 0.02f,  true    }
};

EntityFrame_t PlayerAnimation_KatanaAttack2[] =
{
	{   NULL,	191,	0.02f			},
	{   NULL,	192,	0.02f			},
	{   NULL,	193,	0.02f			},
	{   NULL,	194,	0.02f			},
	{   NULL,	195,	0.02f			},
	{   NULL,	196,	0.02f			},
	{   NULL,	197,	0.02f			},
	{   NULL,	198,	0.02f			},
	{   NULL,	199,	0.02f			},
	{   NULL,	200,	0.02f			},
	{   NULL,	201,	0.02f			},
	{   NULL,	202,	0.02f			},
	{   NULL,	203,	0.02f			},
	{   NULL,	204,	0.02f			},
	{   NULL,	205,	0.02f			},
	{   NULL,	206,	0.02f			},
	{   NULL,	207,	0.02f			},
	{   NULL,	208,	0.02f			},
	{   NULL,	209,	0.02f			},
	{   NULL,	210,	0.02f			},
	{   NULL,	211,	0.02f			},
	{   NULL,	212,	0.02f			},
	{   NULL,	213,	0.02f			},
	{   NULL,	214,	0.02f			},
	{   NULL,	215,	0.02f			},
	{	NULL,	216,	0.02f,	true	}
};

EntityFrame_t PlayerAnimation_KatanaDeath1[] =
{
	{   NULL, 218, 0.02f},
	{   NULL, 219, 0.02f},
	{   NULL, 220, 0.02f},
	{   NULL, 221, 0.02f},
	{   NULL, 222, 0.02f},
	{   NULL, 223, 0.02f},
	{   NULL, 224, 0.02f},
	{   NULL, 225, 0.02f},
	{   NULL, 226, 0.02f},
	{   NULL, 227, 0.02f},
	{   NULL, 228, 0.02f},
	{   NULL, 229, 0.02f},
	{   NULL, 230, 0.02f},
	{   NULL, 231, 0.02f},
	{   NULL, 232, 0.02f},
	{   NULL, 233, 0.02f},
	{   NULL, 234, 0.02f},
	{   NULL, 235, 0.02f},
	{   NULL, 236, 0.02f},
	{   NULL, 237, 0.02f},
	{   NULL, 238, 0.02f},
	{   NULL, 239, 0.02f},
	{   NULL, 240, 0.02f},
	{   NULL, 241, 0.02f},
	{   NULL, 242, 0.02f},
	{   NULL, 243, 0.02f},
	{   NULL, 244, 0.02f},
	{   NULL, 245, 0.02f},
	{   NULL, 246, 0.02f},
	{   NULL, 247, 0.02f},
	{   NULL, 248, 0.02f},
	{   NULL, 249, 0.02f,  true    }
};

void Player_CheckFootsteps(edict_t *ePlayer)
{
	float	fForce;
	double	dDelay;
	vec2_t	vStep;

	// [8/6/2013] Also check movetype so we don't do steps while noclipping/flying ~hogsy
	if((ePlayer->v.movetype == MOVETYPE_WALK) && ePlayer->v.flags & FL_ONGROUND)
	{
		if((ePlayer->v.velocity[0] == 0 && ePlayer->v.velocity[1] == 0)	||
			ePlayer->local.dStepTime > Server.dTime)
			return;

		vStep[0] = ePlayer->v.velocity[0];
		if(vStep[0] < 0)
			vStep[0] *= -1.0f;

		vStep[1] = ePlayer->v.velocity[1];
		if(vStep[1] < 0)
			vStep[1] *= -1.0f;

		fForce = vStep[0]+vStep[1];

		// [9/6/2013] Base this on our velocity ~hogsy
		dDelay = Math_Clamp(0.1, (double)(1.0f / (fForce / 100.0f)), 1.0);

		// [9/6/2013] TODO: Check if we're in water or not and change this accordingly :) ~hogsy
		Sound(ePlayer,CHAN_VOICE,va("physics/concrete%i_footstep.wav",rand()%4),35,ATTN_NORM);

		ePlayer->local.dStepTime = Server.dTime+dDelay;
	}
}

void Player_CheckWater(edict_t *ePlayer)
{
	// [2/8/2014] Basic Drowning... ~eukos
	if(ePlayer->v.waterlevel != 3)
		ePlayer->local.dAirFinished = Server.dTime + 12;
	else if(ePlayer->local.dAirFinished < Server.dTime)
	{
		if(ePlayer->local.dPainFinished < Server.dTime)
		{
			MONSTER_Damage(ePlayer,ePlayer,10, 0);
			ePlayer->local.dPainFinished = Server.dTime + 1;
		}
	}
}

void Player_PostThink(edict_t *ePlayer)
{
	// [25/8/2012] Added iFlag to deal with a particular issue ~hogsy
	int		iFlag = CHAN_VOICE;
	char	snd[32];

	// [5/9/2013] If round has not started then don't go through this! ~hogsy
	if((ePlayer->monster.iState == STATE_DEAD) || !Server.bRoundStarted)
		return;
	// [12/4/2013] Check if we're in a vehicle ~hogsy
	else if(ePlayer->local.eVehicle)
	{
		switch(ePlayer->local.eVehicle->Vehicle.iSlot[ePlayer->local.iVehicleSlot])
		{
		case SLOT_DRIVER:
			Math_VectorCopy(ePlayer->local.eVehicle->v.origin,ePlayer->v.origin);
			break;
		case SLOT_PASSENGER:
			break;
		default:
			Engine.Con_Warning("Player (%s) is occupying an unknown vehicle slot (%i)!\n",
				ePlayer->v.netname,ePlayer->local.iVehicleSlot);

			Vehicle_Exit(ePlayer->local.eVehicle,ePlayer);
		}
		return;
	}

	Weapon_CheckInput(ePlayer);

	if(	(ePlayer->local.jump_flag < -300.0f)	&&
		(ePlayer->v.flags & FL_ONGROUND)		&&
		(ePlayer->v.iHealth > 0))
	{
		if(ePlayer->v.watertype == BSP_CONTENTS_WATER)
			sprintf(snd,"player/h2ojump.wav");
		else if(ePlayer->local.jump_flag < -650.0f)
		{
			// [8/6/2012] TODO: Figure out if we were pushed by an enemy... ~hogsy
			// [8/6/2012] TODO: Base damage on velocity ~hogsy
			MONSTER_Damage(ePlayer,ePlayer,10, 0);

			if(rand()%2 == 1)
				sprintf(snd,"player/playerlandhurt.wav");
			else
				sprintf(snd,"player/playerlandhurt2.wav");

			ePlayer->local.deathtype = "falling";

			ePlayer->v.punchangle[0] += (float)(rand()%5+2)*7.0f;	// [13/4/2013] Give him a big punch... ~eukos
		}
		else
		{
			// [25/8/2012] Land sounds DO NOT use CHAN_VOICE otherwise they get horribly cut out! ~hogsy
			iFlag = CHAN_AUTO;

			sprintf(snd,"player/playerland%i.wav",rand()%4+1);

			// [26/4/2013] Rewritten :) ~hogsy
			ePlayer->v.punchangle[0] -= ePlayer->local.jump_flag/100.0f;	// [13/04/2013] Give him a little punch... ~eukos
		}

		Sound(ePlayer,iFlag,snd,255,ATTN_NORM);

		ePlayer->local.jump_flag = 0;
	}

	if(!(ePlayer->v.flags & FL_ONGROUND))
		ePlayer->local.jump_flag = ePlayer->v.velocity[2];
	// [10/11/2013] Fixed a bug where the player would cycle when just sliding down something... ~hogsy
	else if((	(ePlayer->v.velocity[0] < -4.0f || ePlayer->v.velocity[0] > 4.0f)	||
				(ePlayer->v.velocity[1] < -4.0f || ePlayer->v.velocity[1] > 4.0f))	&&
				(!ePlayer->local.dAnimationTime || ePlayer->local.iAnimationEnd == 9))
		Entity_Animate(ePlayer,PlayerAnimation_Walk);
	else if((ePlayer->v.velocity[0] == 0 || ePlayer->v.velocity[1] == 0) && (!ePlayer->local.dAnimationTime || ePlayer->local.iAnimationEnd == 46))
	{
#ifdef GAME_OPENKATANA
		if(ePlayer->v.iActiveWeapon == WEAPON_DAIKATANA)
			Entity_Animate(ePlayer,PlayerAnimation_KatanaIdle);
		else
#endif
			Entity_Animate(ePlayer,PlayerAnimation_Idle);
	}

	Player_CheckFootsteps(ePlayer);
#ifdef GAME_OPENKATANA
	Player_CheckPowerups(ePlayer);
#endif
}

void Player_PreThink(edict_t *ePlayer)
{
	if(Server.bRoundStarted && !Server.bPlayersSpawned)
	{
		// [5/9/2013] Spawn the player! ~hogsy
		Player_Spawn(ePlayer);
		return;
	}
	else if(!Server.bRoundStarted)
		return;

	Weapon_CheckFrames(ePlayer);
	Entity_CheckFrames(ePlayer);
	Player_CheckWater(ePlayer);

	if(ePlayer->monster.iState == STATE_DEAD)
	{
		Player_DeathThink(ePlayer);
		return;
	}

	if(ePlayer->v.waterlevel == 2)
	{
		int		i;
		trace_t	tTrace;
		vec3_t	vStart,vEnd;

		// [23/9/2012] Check if we can jump onto an edge, was originally in a seperate function but merged here instead ~hogsy
		Math_AngleVectors(ePlayer->v.angles,ePlayer->v.vForward,ePlayer->local.vRight,ePlayer->local.vUp);
		Math_VectorCopy(ePlayer->v.origin,vStart);

		vStart[pZ] += 8.0f;

		ePlayer->v.vForward[pZ] = 0;

		Math_VectorNormalize(ePlayer->v.vForward);

		for(i = 0; i < 3; i++)
			vEnd[i]	= vStart[i]+ePlayer->v.vForward[i]*24.0f;

		tTrace = Engine.Server_Move(vStart,vEnd,mv3Origin,mv3Origin,true,ePlayer);
		if(tTrace.fraction < 1.0f)
		{
			vStart[pZ] += ePlayer->v.maxs[pZ]-8.0f;

			for(i = 0; i < 3; i++)
				vEnd[i]	= vStart[i]+ePlayer->v.vForward[i]*24.0f;

			Math_VectorSubtractValue(tTrace.plane.normal,50.0f,ePlayer->v.movedir);

			tTrace = Engine.Server_Move(vStart,vEnd,mv3Origin,mv3Origin,true,ePlayer);
			if(tTrace.fraction == 1.0f)
			{
				ePlayer->v.flags		|= FL_WATERJUMP;
				ePlayer->v.flags		-= (ePlayer->v.flags & FL_JUMPRELEASED);
				ePlayer->v.velocity[pZ]	= 225.0f;
			}
		}
	}

	if(ePlayer->v.button[2])
		Player_Jump(ePlayer);
	else
		// [30/7/2012] Simplified ~hogsy
		ePlayer->v.flags |= FL_JUMPRELEASED;

	if(ePlayer->v.button[1])
	{
		Entity_SetSize(ePlayer,-16.0f,-16.0f,-24.0f,16.0f,16.0f,35.0f);
		ePlayer->v.view_ofs[2]	= 10.0f;
	}
	else
	{
		Entity_SetSize(ePlayer,-16.0f,-16.0f,-24.0f,16.0f,16.0f,40.0f);
		ePlayer->v.view_ofs[2]	= 30.0f;
	}

	if(cvServerWaypointSpawn.value && (Server.dTime >= Server.dWaypointSpawnDelay))
	{
		if(ePlayer->v.movetype != MOVETYPE_WALK)
			return;

		// [2/1/2012] TODO: Delay currently isn't properly setup! Ugh lazy me ~hogsy
		if(ePlayer->v.flags & FL_ONGROUND)
			// [20/12/2012] Create a waypoint at our current position ~hogsy
			Waypoint_Spawn(ePlayer->v.origin,WAYPOINT_DEFAULT);
		else if((ePlayer->v.flags & FL_SWIM) /*&& !(ePlayer->v.flags & FL_ONGROUND)*/)
			Waypoint_Spawn(ePlayer->v.origin,WAYPOINT_SWIM);
		// [10/4/2013] Create waypoints in the air.
		else if(!(ePlayer->v.flags & FL_ONGROUND))
			Waypoint_Spawn(ePlayer->v.origin,WAYPOINT_JUMP);

		Server.dWaypointSpawnDelay = Server.dTime+((double)cvServerWaypointDelay.value);
	}
}

void Player_Die(edict_t *ePlayer,edict_t *other)
{
	char	s[32];

	/*	TODO:
		Slowly fade our screen to red.
		Camera should follow the entity that killed us.
	*/

	Math_VectorClear(ePlayer->v.view_ofs);

	ePlayer->v.modelindex	= iPlayerModelIndex;
	ePlayer->v.view_ofs[2]	= -8.0f;
	ePlayer->v.flags		-= (ePlayer->v.flags & FL_ONGROUND);
	ePlayer->v.movetype		= MOVETYPE_TOSS;
	ePlayer->v.angles[0]	= ePlayer->v.angles[2] = 0;

	ePlayer->monster.iState	= STATE_DEAD;

	ePlayer->Physics.iSolid	= SOLID_NOT;

#ifdef GAME_OPENKATANA
	// [15/10/2013] Detonate all C4 bombs we've laid out! ~hogsy
	C4Vizatergo_SecondaryAttack(ePlayer);
#endif

#if 0
	int i;
	// [25/8/2012] Death cam ~hogsy
	// [25/8/2012] TODO: Monster and client checks ~hogsy
	if(other)
	{
		Engine.SetMessageEntity(ent);

		Engine.WriteByte(MSG_ONE,SVC_SETANGLE);
		for(i = 0; i < 2; i++)
			Engine.WriteAngle(MSG_ONE,other->v.origin[i]);
	}
#endif

	if(ePlayer->v.iHealth < PLAYER_MIN_HEALTH)
	{
		Sound(ePlayer,CHAN_VOICE,"misc/gib1.wav",255,ATTN_NORM);

		// [13/9/2012] Updated paths ~hogsy
		ThrowGib(ePlayer->v.origin,ePlayer->v.velocity,PHYSICS_MODEL_GIB0,(float)ePlayer->v.iHealth*-1,true);
		ThrowGib(ePlayer->v.origin,ePlayer->v.velocity,PHYSICS_MODEL_GIB1,(float)ePlayer->v.iHealth*-1,true);
		ThrowGib(ePlayer->v.origin,ePlayer->v.velocity,PHYSICS_MODEL_GIB2,(float)ePlayer->v.iHealth*-1,true);
		ThrowGib(ePlayer->v.origin, ePlayer->v.velocity, PHYSICS_MODEL_GIB3, (float)ePlayer->v.iHealth*-1, true);

		Engine.Particle(ePlayer->v.origin,ePlayer->v.velocity,10.0f,"blood",20);

		// [2/10/2013] Hide the model ~hogsy
		// [17/11/2013] Fixed ~hogsy
		Entity_SetModel(ePlayer,"");

		return;
	}

	if(ePlayer->v.waterlevel == 3)
		sprintf(s,"player/playerwaterdeath.wav");
	else
		sprintf(s,"player/playerdeath%i.wav",rand()%4+1);

	Sound(ePlayer,CHAN_VOICE,s,255,ATTN_NONE);

#ifdef GAME_OPENKATANA
	if(ePlayer->v.iActiveWeapon == WEAPON_DAIKATANA)
		Entity_Animate(ePlayer,PlayerAnimation_KatanaDeath1);
	else
#endif
	{
		if(rand()%2 == 1)
			Entity_Animate(ePlayer,PlayerAnimation_Death1);
		else
			Entity_Animate(ePlayer,PlayerAnimation_Death2);
	}
}

void Player_Pain(edict_t *ent,edict_t *other)
{
	char cSound[24];

	PLAYER_SOUND_PAIN(cSound);

	Sound(ent,CHAN_VOICE,cSound,255,ATTN_NORM);
}

int	iSpawnSlot;

void Player_Spawn(edict_t *ePlayer)
{
	edict_t *eSpawnPoint;

	ePlayer->monster.iType	= MONSTER_PLAYER;

	ePlayer->v.cClassname	= "player";
	// [30/1/2013] Default health can now be changed by admins ~hogsy
	ePlayer->v.iHealth		= cvServerDefaultHealth.iValue;
	// [5/6/2012] Make max_health changable for admins ~hogsy
	ePlayer->v.iMaxHealth	= PLAYER_MAX_HEALTH;
	ePlayer->v.movetype		= MOVETYPE_WALK;
	ePlayer->v.bTakeDamage	= true;
	ePlayer->v.model		= cvServerPlayerModel.string;
	ePlayer->v.effects		= 0;

	// [30/5/2013] Set physics properties to their defaults! ~hogsy
	ePlayer->Physics.iSolid		= SOLID_SLIDEBOX;
	ePlayer->Physics.fMass		= 1.4f;
	ePlayer->Physics.fGravity	= SERVER_GRAVITY;
	ePlayer->Physics.fFriction	= 4.0f;

	ePlayer->local.fSpawnDelay	= cvServerRespawnDelay.value;	// Set the delay before we spawn ~hogsy
	ePlayer->local.pTeam		= TEAM_NEUTRAL;					// Set the default team ~hogsy
	ePlayer->local.bBleed		= true;							// The player bleeds!

	// [25/8/2012] Clear velocity here (why would we call this twice!?) ~hogsy
	Math_VectorClear(ePlayer->v.velocity);
	Math_VectorClear(ePlayer->v.view_ofs);

	ePlayer->v.fixangle		= true;
	ePlayer->v.view_ofs[2]	= 30.0f; // [10/05/2013] Was 22.0f ~eukos

	ePlayer->monster.iState		= STATE_AWAKE;	// TODO: Is this necessary? ~hogsy
	ePlayer->monster.think_die	= Player_Die;
	ePlayer->monster.think_pain = Player_Pain;

	Server.bPlayersSpawned = true;

	if(bIsMultiplayer)
	{
#ifdef GAME_OPENKATANA
		switch((int)cvServerGameMode.value)
		{
		// TODO: Check what model this player has set in a cvar
		// TODO: Set texture function for team based skins?
		// [25/2/2012] Added mode specific model selection ~hogsy
		case MODE_CAPTURETHEFLAG:
			if(!ePlayer->local.pTeam)
			{
				if(!iSpawnSlot)
				{
					ePlayer->local.pTeam = TEAM_BLUE;
					Engine.Server_BroadcastPrint("%s has been assigned to the blue team!\n",ePlayer->v.netname);
					iSpawnSlot++;
				}
				else
				{
					ePlayer->local.pTeam = TEAM_RED;
					Engine.Server_BroadcastPrint("%s has been assigned to the red team!\n",ePlayer->v.netname);
					iSpawnSlot = 0;
				}

	#if 0
				{
					ent->local.pTeam = TEAM_SPECTATOR;
					Game.Server_BroadcastPrint("%s is spectating.\n",ePlayer->v.netname);
				}
	#endif
			}

			/*	Model names are temporary until we get
				player models sorted. Had a bit of an
				idea that team blue could be a random
				selection of Mikiko, Superfly and Hiro
				while team red could be random selection
				of special robots which Mishima has
				created just for CTF.
			*/
			switch(ePlayer->local.pTeam)
			{
			case TEAM_BLUE:
				ePlayer->v.model = "models/sprfly.mdl";
				break;
			case TEAM_RED:
				ePlayer->v.model = "models/mikiko.mdl";
				break;
			case TEAM_SPECTATOR:
				ePlayer->v.model = "";
				break;
			default:
				ePlayer->v.model = "models/player.md2";
				break;
			}
			break;
		case MODE_DEATHMATCH:
			Deathmatch_Spawn(ePlayer);
			break;
		}
#endif
	}
	else	// [29/7/2013] Singleplayer ~hogsy
	{
		Server_UpdateClientMenu(ePlayer,MENU_STATE_HUD,true);

#ifdef GAME_OPENKATANA
		// [18/5/2013] Initial weapon should be the IonBlaster ? Mainly for testing ~hogsy
		{
			Item_t	*iDaikatana = Item_GetItem(WEAPON_DAIKATANA);

			if(iDaikatana)
			{
				Item_AddInventory(iDaikatana,ePlayer);

				{
					Weapon_t *wStartWeapon = Weapon_GetWeapon(WEAPON_DAIKATANA);
					if(wStartWeapon)
						Weapon_SetActive(wStartWeapon,ePlayer);
				}
			}
		}
#elif GAME_ADAMAS
		{
			Item_t *iBlazer = Item_GetItem(WEAPON_BLAZER);

			if(iBlazer)
			{
				Weapon_t *wStartWeapon;

				Item_AddInventory(iBlazer,ePlayer);

				// Give us some ammo too! ~hogsy
				ePlayer->local.iBulletAmmo = 250;

				wStartWeapon = Weapon_GetWeapon(WEAPON_BLAZER);
				if(wStartWeapon)
					Weapon_SetActive(wStartWeapon,ePlayer);
			}
		}

		// On our first life, notify the player on how to play. ~hogsy
		if(Server.iLives == 2)
			Engine.CenterPrint(ePlayer,"Walk forwards to begin the round!\nDon't get killed.");
#endif
	}

	eSpawnPoint = Entity_SpawnPoint(ePlayer,INFO_PLAYER_START);
	if(eSpawnPoint)
	{
		// [25/8/2012] Just copy our position and angle ~hogsy
		Math_VectorCopy(eSpawnPoint->v.origin,ePlayer->v.origin);
		Math_VectorCopy(eSpawnPoint->v.angles,ePlayer->v.angles);
	}
	else
	{
		Engine.Con_Warning("Failed to find spawn point for player! (%s)\n",ePlayer->v.netname);

		Math_VectorClear(ePlayer->v.angles);
		Math_VectorClear(ePlayer->v.origin);
	}

	// [28/4/2013] Moved here so we check everytime (handy for level designers) ~hogsy
	// [25/8/2012] Check that we're not inside a brush ~hogsy
	if(Engine.Server_PointContents(ePlayer->v.origin) == BSP_CONTENTS_SOLID)
		// [25/8/2012] Let the admin know he's been spawned inside the world ~hogsy
		Engine.Con_Warning("Player spawned inside world! (%s)\n",ePlayer->v.netname);

	// [30/10/2013] This is for a bug that appeared during testing, but sometimes after immediately spawning the players animation would continue! ~hogsy
	Entity_ResetAnimation(ePlayer);

	Entity_SetModel(ePlayer,ePlayer->v.model);
	Entity_SetSize(ePlayer,-16.0f,-16.0f,-24.0f,16.0f,16.0f,40.0f);
	// [25/8/2012] Moved down here ~hogsy
	Entity_SetOrigin(ePlayer,ePlayer->v.origin);
	SetAngle(ePlayer,ePlayer->v.angles);

	iPlayerModelIndex = ePlayer->v.modelindex;
}

void Player_Jump(edict_t *ePlayer)
{
	char	cJumpSound[32];

#if 1
	if(ePlayer->v.movetype == MOVETYPE_FLY)	// LADDER MESSY ~EUKOS
		ePlayer->v.movetype = MOVETYPE_WALK;
#else	// [24/9/2013] Mockup of replacement ~hogsy
	if(ePlayer->v.movetype == MOVETYPE_CLIMB)
		ePlayer->v.movetype	= MOVETYPE_WALK;
#endif

	// [28/4/2013] Don't let us jump while inside a vehicle ~hogsy
	if(ePlayer->v.flags & FL_WATERJUMP || ePlayer->local.eVehicle)
		return;
	else if(ePlayer->v.waterlevel >= 2)
	{
		// [20/8/2012] Simplified ~hogsy
		switch(ePlayer->v.watertype)
		{
		case BSP_CONTENTS_WATER:
			ePlayer->v.velocity[2] = 100.0f;
			break;
		case CONTENT_SLIME:
			ePlayer->v.velocity[2] = 80.0f;
			break;
		default:
			ePlayer->v.velocity[2] = 50.0f;
		}

		if(ePlayer->local.swim_flag < Server.dTime)
		{
			ePlayer->local.swim_flag = (float)(Server.dTime+1.0);

			Sound(ePlayer,CHAN_BODY,"player/playerswim1.wav",255,ATTN_NORM);
		}
		return;
	}
	else if(!(ePlayer->v.flags & FL_ONGROUND) || !(ePlayer->v.flags & FL_JUMPRELEASED))
		return;

	ePlayer->v.flags		-= (ePlayer->v.flags & FL_JUMPRELEASED);
	ePlayer->v.flags		-= FL_ONGROUND;
	ePlayer->v.button[2]	= 0;

#ifdef GAME_OPENKATANA
	if(ePlayer->local.acro_finished > Server.dTime)
	{
		ePlayer->v.velocity[2] += 440.0f;

		sprintf(cJumpSound,"player/acroboost.wav");
	}
	else
#endif
	{
		ePlayer->v.velocity[2] += 250.0f;

		sprintf(cJumpSound,"player/jump%i.wav",rand()%4);
	}

	Sound(ePlayer,CHAN_VOICE,cJumpSound,255,ATTN_NORM);

	ePlayer->v.punchangle[0] += 3.0f;

	if((ePlayer->v.velocity[0] == 0) && (ePlayer->v.velocity[1] == 0))
		Entity_Animate(ePlayer,PlayerAnimation_Jump);
	else
		Entity_Animate(ePlayer,PlayerAnimation_RunJump);
}

#ifdef GAME_OPENKATANA
void Player_CheckPowerups(edict_t *ePlayer)
{
	Item_t	*iPowerBoost,
			*iVitalityBoost,
			*iSpeedBoost,
			*iAttackBoost,
			*iAcroBoost;

	if(ePlayer->v.iHealth <= 0)
		return;

	// [28/7/2013] Use new friendly functions ~hogsy
	iPowerBoost		= Item_GetInventory(ITEM_POWERBOOST,ePlayer);
	iVitalityBoost	= Item_GetInventory(ITEM_VITABOOST,ePlayer);
	iSpeedBoost		= Item_GetInventory(ITEM_SPEEDBOOST,ePlayer);
	iAttackBoost	= Item_GetInventory(ITEM_ATTACKBOOST,ePlayer);
	iAcroBoost		= Item_GetInventory(ITEM_ACROBOOST,ePlayer);

	if(iPowerBoost && ePlayer->local.power_finished)
	{
		if(ePlayer->local.power_time == 1)
			if(ePlayer->local.power_finished < Server.dTime+3.0)
			{
				// [30/7/2012] Updated to use centerprint instead ~hogsy
				Engine.CenterPrint(ePlayer,"Your power boost is running out.\n");

				ePlayer->local.power_time = Server.dTime+1.0;
			}

		if(ePlayer->local.power_finished < Server.dTime)
		{
			Item_RemoveInventory(iPowerBoost,ePlayer);

			ePlayer->local.power_finished	=
			ePlayer->local.power_time		= 0;
		}
	}

	if(iSpeedBoost && ePlayer->local.speed_finished)
	{
		if(ePlayer->local.speed_time == 1)
			if(ePlayer->local.speed_finished < Server.dTime+3.0)
			{
				// [30/7/2012] Updated to use centerprint instead ~hogsy
				Engine.CenterPrint(ePlayer,"Your speed boost is running out.\n");

				ePlayer->local.speed_time = Server.dTime+1.0;
			}

		if(ePlayer->local.speed_finished < Server.dTime)
		{
			Item_RemoveInventory(iSpeedBoost,ePlayer);

			ePlayer->local.speed_finished	=
			ePlayer->local.speed_time		= 0;
		}
		else
		{
			// [14/2/2013] Commented out since this is printed out every frame and kills the framerate! ~hogsy
			//Game.Con_DPrintf("%f %f\n",ent->v.velocity[0],ent->v.velocity[1]);
			// SPEED THE PLAYER UP!!! ~eukos
		}
	}

	if(iAttackBoost && ePlayer->local.attackb_finished)
	{
		if(ePlayer->local.attackb_time == 1.0f)
			if(ePlayer->local.attackb_finished < Server.dTime+3.0)
			{
				// [25/8/2012] Updated to use centerprint instead ~hogsy
				Engine.CenterPrint(ePlayer,"Your attack boost is running out.\n");

				ePlayer->local.attackb_time = Server.dTime+1.0;
			}

		if(ePlayer->local.attackb_finished < Server.dTime)
		{
			Item_RemoveInventory(iAttackBoost,ePlayer);

			ePlayer->local.attackb_finished =
			ePlayer->local.attackb_time		= 0;
		}
	}

	if(iAcroBoost && ePlayer->local.acro_finished)
	{
		if(ePlayer->local.acro_time == 1)
			if(ePlayer->local.acro_finished < Server.dTime+3.0)
			{
				// [25/8/2012] Updated to use centerprint instead ~hogsy
				Engine.CenterPrint(ePlayer,"Your acro boost is running out.\n");

				ePlayer->local.acro_time = Server.dTime+1.0;
			}

		if(ePlayer->local.acro_finished < Server.dTime)
		{
			Item_RemoveInventory(iAcroBoost,ePlayer);

			ePlayer->local.acro_finished	=
			ePlayer->local.acro_time		= 0;
		}
	}

	if(iVitalityBoost && ePlayer->local.vita_finished)
	{
		if(ePlayer->local.vita_time == 1)
			if(ePlayer->local.vita_finished < Server.dTime+3.0)
			{
				// [25/8/2012] Updated to use centerprint instead ~hogsy
				Engine.CenterPrint(ePlayer,"Your vitality boost is running out.\n");

				ePlayer->local.vita_time = Server.dTime+1.0;
			}

		if(ePlayer->local.vita_finished < Server.dTime)
		{
			Item_RemoveInventory(iVitalityBoost,ePlayer);

			ePlayer->local.vita_finished	=
			ePlayer->local.vita_time		= 0;
		}
	}
}
#endif

// [23/3/2013] Countdown is now done for cooperative mode too! ~hogsy
void Player_DeathThink(edict_t *ent)
{
	if(	ent->v.button[0]	||
		ent->v.button[1]	||
		ent->v.button[2])
	{
		// [25/8/2012] Simplified ~hogsy
		Math_VectorClear(ent->v.button);

		if(!ent->local.fSpawnDelay)
		{
#ifdef GAME_ADAMAS
			if(!Server.iLives)
				Engine.Server_Restart();
			else
			{
				Server.iLives--;

				Engine.CenterPrint(ent,va("You have %i lives remaining!\n",Server.iLives));

				Player_Spawn(ent);
			}
#else
			// [25/8/2012] We don't respawn in singleplayer ~hogsy
			// [23/3/2013] Oops! Fixed, we were checking for the wrong case here :) ~hogsy
			if(bIsMultiplayer)
				// [16/10/2013] Swapped out for the more "correct" Player_Spawn rather than via Monster_Respawn ~hogsy
				Player_Spawn(ent);
			else
			{
				Engine.Server_Restart();
				return;
			}
#endif
		}
	}

	// [25/8/2012] If it's multiplayer and not coop, countdown ~hogsy
	// [15/8/2013] Countdown for both singleplayer and multiplayer ~hogsy
	if(ent->local.fSpawnDelay)
		// [25/8/2012] TODO: Force respawn if timer runs out? ~hogsy
		ent->local.fSpawnDelay -= 0.5f;
}

void Player_Use(edict_t *ePlayer)
{
	if(ePlayer->v.iHealth <= 0 || ePlayer->local.dAttackFinished > Server.dTime)
		return;

	// If nothing usable is being aimed at then play sound...
	// TODO: Find a more appropriate sound :)
	Sound(ePlayer,CHAN_VOICE,"player/playerpain3.wav",255,ATTN_NORM);

	ePlayer->local.dAttackFinished = Server.dTime+0.5;
}

/*
	Movement
*/

/*	Called per-frame to handle player movement for each client.
*/
void Player_MoveThink(edict_t *ePlayer)
{
#if 0
	vec3_t	vViewAngle;
	float	fLength,*fPlayerAngles;

	if(ePlayer->v.movetype == MOVETYPE_NONE)
		return;

	fLength = Math_VectorNormalize(ePlayer->v.punchangle);
	fLength -= 10*(float)Engine.Server_GetFrameTime();
	if(fLength < 0)
		fLength = 0;

	Math_VectorScale(ePlayer->v.punchangle,fLength,ePlayer->v.punchangle);

	// If dead, behave differently
	if(!ePlayer->v.iHealth)
		return;

#ifdef IMPLEMENT_ME
	cmd = host_client->cmd;
#endif
	fPlayerAngles = ePlayer->v.angles;

	Math_VectorAdd(ePlayer->v.v_angle,ePlayer->v.punchangle,vViewAngle);
#endif
}

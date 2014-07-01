/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_weapon.h"

/*
	The mighty Daikatana. A magical sword with many hidden properties...
*/

#include "server_player.h"

void Daikatana_Hit(edict_t *ent);

EntityFrame_t DaikatanaAnimation_Deploy [] =
{
	{   NULL, 55, 0.027f		},
	{   NULL, 56, 0.027f		},
	{   NULL, 57, 0.027f		},
	{   NULL, 58, 0.027f		},
	{   NULL, 59, 0.027f		},
	{   NULL, 60, 0.027f		},
	{   NULL, 61, 0.027f, true  }
};

EntityFrame_t DaikatanaAnimation_Attack1 [] =
{
	{	NULL,	2,	0.015f },
	{   NULL,	3, 0.015f  },
	{   NULL,	4, 0.015f  },
	{   NULL,	5, 0.015f  },
	{   NULL,	6, 0.015f  },
	{   NULL,	7, 0.015f  },
	{   Daikatana_Hit,	8, 0.015f    },
	{   NULL,	9, 0.02f  },
	{   NULL,	10, 0.02f },
	{   NULL,	11, 0.02f },
	{   NULL,	12, 0.02f },
	{   NULL,	13, 0.02f },
	{   NULL,	14, 0.02f },
	{   NULL, 15, 0.02f },
	{   NULL, 16, 0.02f },
	{   NULL, 17, 0.02f },
	{   NULL, 18, 0.02f },
	{   NULL, 19, 0.02f },
	{   NULL, 20, 0.02f },
	{   NULL, 21, 0.02f },
	{   NULL, 22, 0.02f },
	{   NULL, 23, 0.02f },
	{   NULL, 24, 0.02f },
	{   NULL, 25,	0.02f,	true	}
};

EntityFrame_t DaikatanaAnimation_Attack2 [] =
{
	{   NULL, 26, 0.015f  },
	{   NULL, 27, 0.015f  },
	{   NULL, 28, 0.015f  },
	{   NULL, 29, 0.015f  },
	{   NULL, 30, 0.015f  },
	{   Daikatana_Hit, 31, 0.015f   },
	{   NULL, 32, 0.015f  },
	{   NULL, 33, 0.015f  },
	{   NULL, 34, 0.015f  },
	{   NULL, 35, 0.015f  },
	{   NULL, 36, 0.015f },
	{   NULL, 37, 0.015f },
	{   NULL, 38, 0.015f },
	{   NULL, 39, 0.015f },
	{   NULL, 40, 0.02f },
	{   NULL, 41, 0.02f },
	{   NULL, 42, 0.02f },
	{   NULL, 43, 0.02f },
	{   NULL, 44, 0.02f },
	{   NULL, 45, 0.02f },
	{   NULL, 46, 0.02f },
	{   NULL, 47, 0.02f },
	{   NULL, 48, 0.02f },
	{   NULL, 49, 0.02f },
	{   NULL, 50, 0.02f },
	{   NULL, 51, 0.02f },
	{   NULL, 52, 0.02f },
	{   NULL, 53, 0.02f },
	{   NULL, 54, 0.02f,	true	}
};

void Daikatana_Deploy(edict_t *ent)
{
	Weapon_Animate(ent,DaikatanaAnimation_Deploy);
}

void Daikatana_Hit(edict_t *ent)
{
	int		i;
	vec3_t	forward,temp,sndvec;
	trace_t	trace;

	Math_AngleVectors(ent->v.v_angle,forward,temp,temp);

	sndvec[0] = ent->v.origin[0]  + (forward[0] * 64.0f); // Changed from 120 to 64 ~eukos
	sndvec[1] = ent->v.origin[1]  + (forward[1] * 64.0f);
	sndvec[2] = (ent->v.origin[2] + ent->v.view_ofs[2]) + (forward[2] * 64.0f);

	trace = Traceline(ent,ent->v.origin,sndvec,0);

	for(i = 0; i < 3; i++)
		sndvec[i] = trace.endpos[i]-(forward[i]*4.0f);

	// [10/3/2013] Simplified ~hogsy
	if(trace.fraction != 1.0f && trace.ent)
	{
		// [5/5/2012] Cleaned up ~hogsy
		// [10/3/2013] Removed particle call since it's done in MONSTER_Damage :) ~hogsy
		if(trace.ent->v.bTakeDamage)
			MONSTER_Damage(trace.ent,ent,35,0);
		else
		{
            char    cSmokeTexture[8];
			vec3_t	vBounce;

			// [6/6/2013] Fix ~hogsy
			Math_VectorClear(vBounce);

			vBounce[2] = 0.5f;

            PARTICLE_SMOKE(cSmokeTexture);

			Engine.Particle(sndvec,vBounce,10,cSmokeTexture,30);
		}
	}
}

void Daikatana_PrimaryAttack(edict_t *ent)
{
	if(rand()%2 == 1)
	{
		Weapon_Animate(ent,DaikatanaAnimation_Attack1);
		Entity_Animate(ent,PlayerAnimation_KatanaAttack1);
	}
	else
	{
		Weapon_Animate(ent,DaikatanaAnimation_Attack2);
		Entity_Animate(ent,PlayerAnimation_KatanaAttack2);
	}

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.5;
	else
		ent->local.dAttackFinished = Server.dTime+1.0;
}

#include "Game.h"

#include "server_weapon.h"

EntityFrame_t AxeAnimation_Deploy [] =
{
	{   NULL, 2, 0.1f    },
	{   NULL, 3, 0.1f    },
	{   NULL, 4, 0.1f    },
	{   NULL, 5, 0.1f    },
	{   NULL, 6, 0.1f		},
	{   NULL, 7, 0.1f, true }
};
EntityFrame_t AxeAnimation_Attack1 [] =
{
	{   NULL, 4, 0.05f    },
	{   NULL, 5, 0.05f    },
	{   NULL, 6, 0.05f    },
	{   NULL, 7, 0.05f    },
	{   NULL, 8, 0.05f    },
	{   NULL, 9, 0.05f    },
	{   NULL, 10, 0.05f    },
	{   NULL, 11, 0.05f    },
	{   NULL, 12, 0.05f    },
	{   NULL, 13, 0.05f    },
	{   NULL, 14, 0.05f    },
	{   NULL, 15, 0.05f    },
	{   NULL, 16, 0.05f, TRUE    }
};
EntityFrame_t AxeAnimation_Attack2 [] =
{
	{   NULL, 17, 0.05f    },
	{   NULL, 18, 0.05f    },
	{   NULL, 19, 0.05f    },
	{   NULL, 20, 0.05f    },
	{   NULL, 21, 0.05f    },
	{   NULL, 22, 0.05f    },
	{   NULL, 23, 0.05f    },
	{   NULL, 24, 0.05f    },
	{   NULL, 25, 0.05f    },
	{   NULL, 26, 0.05f    },
	{   NULL, 27, 0.05f, TRUE    }
};

void Axe_Deploy(edict_t *ent)
{
	WEAPON_Animate(ent,AxeAnimation_Deploy);
}

void AxeHit(edict_t *ent)
{
	vec3_t	forward,temp,sndvec,vel;
	trace_t	trace;

	Math_AngleVectors(ent->v.v_angle,forward,temp,temp);

	// [18/4/2012] A nice soft bounce ~hogsy
	vel[0] = vel[1] = 0;
	vel[2] = 0.5;

	sndvec[0] = ent->v.origin[0]+forward[0]*64;
	sndvec[1] = ent->v.origin[1]+forward[1]*64;
	sndvec[2] = ent->v.origin[2]+forward[2]*64;

	trace = Traceline(ent,ent->v.origin,sndvec,0);

	sndvec[0] = trace.endpos[0]-forward[0]*4;
	sndvec[1] = trace.endpos[1]-forward[1]*4;
	sndvec[2] = trace.endpos[2]-forward[2]*4;

	if(trace.fraction == 1.0f)
		return;
	if(trace.ent->v.bTakeDamage)
	{
		if(trace.ent->local.bBleed)
			Engine.Particle(sndvec,vel,10,"blood",30);
		MONSTER_Damage(trace.ent,ent,20);
	}
	else if(trace.ent)
	{
		Engine.Particle(sndvec,vel,10,"smoke",30);
	}

}

void Axe_PrimaryAttack(edict_t *ent)
{

	Sound(ent,CHAN_WEAPON,"weapons/axe/axeswing.wav",255,ATTN_NORM);

	if(rand()%2 == 1)
		WEAPON_Animate(ent,AxeAnimation_Attack1);
	else
		WEAPON_Animate(ent,AxeAnimation_Attack2);

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.25;
	else
		ent->local.dAttackFinished = Server.dTime+0.5;
}

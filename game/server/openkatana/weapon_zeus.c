#include "server_main.h"

void Zeus_Deploy(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,1,19,0.08f,0,0,0,FALSE);
}

void Zeus_Hit(edict_t *ent)
{
	vec3_t	vSource,vTarg, vel;
	vec_t	*vDirection = Engine.Aim(ent);
	trace_t	trace;

	vSource[0] = ent->v.origin[0];
	vSource[1] = ent->v.origin[1];
	vSource[2] = ent->v.origin[2] + 22.0f;

	vTarg[0] = vSource[0]+(vDirection[0]*250);
	vTarg[1] = vSource[1]+(vDirection[1]*250);
	vTarg[2] = vSource[2]+(vDirection[2]*250);

	vel[0] = vel[1] = vel[2] = 0;

	trace = Traceline(ent,vSource,vTarg,0);

	if(trace.fraction != 1.0f)
		{
			if(trace.ent && trace.ent->v.bTakeDamage)
			{
				if(trace.ent->local.bBleed)
					Engine.Particle(trace.endpos,vel,10,"blood",30);
				MONSTER_Damage(trace.ent,ent,5);
			}
			else
				Engine.Particle(trace.endpos,vel,10,"zspark",30);
		}
}

void Zeus_PrimaryAttack(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,20,35,0.08f,0,0,0,FALSE);

	if(ent->local.attackb_finished > Server.dTime)
		ent->local.dAttackFinished = Server.dTime+0.6;
	else
		ent->local.dAttackFinished = Server.dTime+1.2;
}

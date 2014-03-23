#include "server_main.h"

void Trident_Deploy(edict_t *ent)
{
	//WEAPON_Animate(ent,FALSE,1,10,0.1f,0,0,0,FALSE);
}

void WEAPON_TRIDENT_Precache (void)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/trident.md2");
	Engine.Server_PrecacheResource(RESOURCE_SOUND,"weapons/trident/tridentdraw.wav");
}

void TridentHit(edict_t *ent)
{
	vec3_t	forward,temp,sndvec,vel;
	trace_t	trace;

	Math_AngleVectors(ent->v.v_angle,forward,temp,temp);

	// [18/4/2012] A nice soft bounce ~hogsy
	vel[0] = vel[1] = 0;
	vel[2] = 0.5;

	sndvec[0] = ent->v.origin[0]+forward[0]*78;
	sndvec[1] = ent->v.origin[1]+forward[1]*78;
	sndvec[2] = ent->v.origin[2]+forward[2]*78;

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
		Engine.Particle(sndvec,vel,10,"smoke",30);
}
void Trident_PrimaryAttack(edict_t *ent)
{
	Sound(ent,CHAN_WEAPON,"weapons/axe/axeswing.wav",255,ATTN_NORM);

#if 0
	if(rand()%2 == 1)
		//WEAPON_Animate(ent,FALSE,10,19,0.05f,10,19,0,FALSE);
	else
		//WEAPON_Animate(ent,FALSE,20,29,0.05f,10,19,0,FALSE);
#endif

	if(ent->local.attackb_finished > Server.dTime)	// No attack boost...
		ent->local.dAttackFinished = Server.dTime+0.2;
	else
		ent->local.dAttackFinished = Server.dTime+0.4;
}

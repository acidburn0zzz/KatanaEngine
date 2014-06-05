/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef	__SERVERWEAPON__
#define	__SERVERWEAPON__

#include "server_main.h"

/*	Ammo types.
*/
typedef enum
{
#ifdef GAME_OPENKATANA
	AM_BULLET,
	AM_ROCKET,
/*	Episode one					*/
	AM_IONS,					// ION ammunition.
	AM_SHELL,					// SHOTCYCLER ammunition.
	AM_LASER,					// Shockwave ammunition.
	AM_C4BOMB,					// C4 ammunition.
/*	Episode two					*/
	AM_DISCUS,					// Discus ammunition.
/*	Episode three				*/
/*	Episode four				*/
#endif

	AM_SWITCH,
	AM_MELEE,

	AM_NONE
} WeaponAmmo_t;

#ifdef GAME_OPENKATANA
void IonBlaster_Deploy(edict_t *ent);
void Shockwave_Deploy(edict_t *ent);
void SideWinder_Deploy(edict_t *ent);
void Shotcycler_Deploy(edict_t *ent);
void C4Vizatergo_Deploy(edict_t *ent);
void Daikatana_Deploy(edict_t *ent);
void Axe_Deploy(edict_t *ent);
void Ballista_Deploy(edict_t *ent);
void Barrier_Deploy(edict_t *ent);
void Crossbow_Deploy(edict_t *ent);
void Discus_Deploy(edict_t *ent);
void Glock_Deploy(edict_t *ent);
void GreekFire_Deploy(edict_t *ent);
void Hermes_Deploy(edict_t *ent);
void Kineticore_Deploy(edict_t *ent);
void Midas_Deploy(edict_t *ent);
void PulseRifle_Deploy(edict_t *ent);
void Zeus_Deploy(edict_t *ent);
void IonRifle_Deploy(edict_t *eOwner);

void Axe_PrimaryAttack(edict_t *ent);
void Ballista_PrimaryAttack(edict_t *ent);
void Barrier_PrimaryAttack(edict_t *ent);
void C4Vizatergo_PrimaryAttack(edict_t *eOwner);
void C4Vizatergo_SecondaryAttack(edict_t *eOwner);
void Crossbow_PrimaryAttack(edict_t *ent);
void Daikatana_PrimaryAttack(edict_t *ent);
void Discus_PrimaryAttack(edict_t *ent);
void Glock_PrimaryAttack(edict_t *ent);
void GreekFire_PrimaryAttack(edict_t *ent);
void Hermes_PrimaryAttack(edict_t *ent);
void IonBlaster_PrimaryAttack(edict_t *ent);
void Kineticore_PrimaryAttack(edict_t *ent);
void Midas_PrimaryAttack(edict_t *ent);
void PulseRifle_PrimaryAttack(edict_t *ent);
void PulseRifle_SecondaryAttack(edict_t *ent);
void Shockwave_PrimaryAttack(edict_t *ent);
void Shotcycler_PrimaryAttack(edict_t *ent);
void SideWinder_PrimaryAttack(edict_t *eOwner);
void Zeus_PrimaryAttack(edict_t *ent);
void IonRifle_PrimaryAttack(edict_t *eOwner);
void IonRifle_SecondaryAttack(edict_t *eOwner);
#endif

typedef struct
{
	int				iItem;

	char			*model;

	void			(*Deploy)(edict_t *ent);

	WeaponAmmo_t	iPrimaryType;

	void			(*Primary)(edict_t *ent);

	WeaponAmmo_t	iSecondaryType;

	void			(*Secondary)(edict_t *ent);

	// [25/9/2013] For handing some control over to the weapon system in regards to updating ammo ~hogsy
	int				iPrimaryAmmoOffset,
					iSecondaryAmmoOffset;
} Weapon_t;

Weapon_t *Weapon_GetCurrentWeapon(edict_t *eEntity);
Weapon_t *Weapon_GetWeapon(int iWeaponID);

void Weapon_Precache(void);
void Weapon_SetActive(Weapon_t *wWeapon,edict_t *eEntity);
void Weapon_BulletProjectile(edict_t *ent,float spread,int damage,vec3_t vVector);
void Weapon_CheckFrames(edict_t *eEntity);
void WEAPON_Animate(edict_t *ent,EntityFrame_t *eFrames);
void Weapon_PrimaryAttack(edict_t *eEntity);
void Weapon_CheckInput(edict_t *eEntity);

bool Weapon_CheckPrimaryAmmo(Weapon_t *wWeapon,edict_t *eEntity);
bool Weapon_CheckSecondaryAmmo(Weapon_t *wWeapon,edict_t *eEntity);

#endif

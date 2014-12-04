/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "server_main.h"

/*
	Item handling code, including inventory crap!
*/

#include "server_item.h"

void CTF_FlagTouch(edict_t *eFlag,edict_t *eOther);

Item_t Items[] =
{
#ifdef GAME_OPENKATANA
	// Ammo
	{	AMMO_IONBALLS,		"Ion Cells",			"models/ammo/ammo_ionblaster.md2",	"items/weaponpickup.wav",	true	},
	{	AMMO_C4BOMBS,		"C4 Bombs",				"models/ammo/ammo_c4viz.md2",		"items/weaponpickup.wav",	true	},
	{	AMMO_SLUGS,			"Shotcycler Round",		"models/ammo/ammo_shotcycler.md2",	"items/weaponpickup.wav",	true	},
	{	AMMO_WINDROCKET,	"Sidewinder Rockets",	"models/ammo/ammo_sidewinder.md2",	"items/weaponpickup.wav",	true	},
	{	AMMO_SHOCKWAVE,		"Shockwave Cells",		"models/ammo/ammo_shockwave.md2",	"items/weaponpickup.wav",	true	},

	// Weapons
	{	WEAPON_DAIKATANA,	"Daikatana",	"models/weapons/w_daikatana.md2",	"items/weaponpickup.wav",	true	},
	{	WEAPON_IONBLASTER,	"Ion Blaster",	"models/weapons/w_ionblaster.md2",	"items/weaponpickup.wav",	true	},
	{	WEAPON_C4VIZATERGO,	"C4 Vizatergo",	"models/weapons/w_c4.md2",			"items/weaponpickup.wav",	true	},
	{	WEAPON_SHOTCYCLER,	"Shotcycler-6",	"models/weapons/w_shotcycler.md2",	"items/weaponpickup.wav",	true	},
	{	WEAPON_SIDEWINDER,	"Sidewinder",	"models/weapons/w_sidewinder.md2",	"items/weaponpickup.wav",	true	},
	{	WEAPON_SHOCKWAVE,	"Shockwave",	"models/weapons/w_shockwave.md2",	"items/weaponpickup.wav",	true	},
	{	WEAPON_IONRIFLE,	"Ion Rifle",	"models/weapons/w_ionrifle.md2",	"items/weaponpickup.wav",	true	},

	// Unused weapons
	{	WEAPON_GLOCK,		"Glock",		"models/weapons/w_glock.md2",		"items/weaponpickup.wav",	true	},
	{   WEAPON_LASERS,      "Lasers",       "",                                 "",							false	},

	// Items
	{	ITEM_PLASTEELARMOR,	"PLASTEEL ARMOR",	"models/plasteel.md2",			"items/armorpickup1.wav",						true	},
	{	ITEM_HEALTHKIT,		"LARGE HEALTH KIT",	"models/canister.md2",			"items/healthpickup.wav",						true	},
	{	ITEM_ATTACKBOOST,	"ATTACK BOOST",		"models/attack.md2",			"items/boostpickup1.wav",						true	},
	{	ITEM_POWERBOOST,	"POWER BOOST",		"models/power.md2",				"items/boostpickup1.wav",						true	},
	{	ITEM_VITABOOST,		"VITA BOOST",		"models/vita.md2",				"items/boostpickup1.wav",						true	},
	{	ITEM_SPEEDBOOST,	"SPEED BOOST",		"models/speed.md2",				"items/boostpickup1.wav",						true	},
	{	ITEM_ACROBOOST,		"ACRO BOOST",		"models/acroboost.md2",			"items/boostpickup1.wav",						true	},
	{	ITEM_OXYLUNG,		"OXYLUNG",			"models/oxylung.md2",			"artifacts/oxylung/oxylungpickup.wav",			true	},
	{	ITEM_ENVIROSUIT,	"ENVIROSUIT",		"models/envirosuit.md2",		"artifacts/envirosuit/envirosuitpickup.wav",	true	},
	{	ITEM_FLAG,			"Flag",				"models/ctf/flag_neutral.md2",	"items/flagpickup.wav",							false	},
	{	ITEM_REDFLAG,		"Red Flag",			"models/ctf/flag_red.md2",		"items/redflagpickup.wav",						false	},
	{	ITEM_BLUEFLAG,		"Blue Flag",		"models/ctf/flag_blue.md2",		"items/blueflagpickup.wav",						false	},
#elif GAME_ADAMAS
	{	WEAPON_BLAZER,		"Blazer",			"models/weapon.bsp",		"items/weaponpickup.wav",	false	},
	{	ITEM_LIFE,			"Extra Life",		"models/life.bsp",			"items/weaponpickup.wav",	false	},
#endif

	{	0	}
};

/*	Gets an item for the given ID.
*/
Item_t *Item_GetItem(int iItemID)
{
	Item_t *iItem;

	for(iItem = Items; iItem->iNumber; iItem++)
		if(iItemID == iItem->iNumber)
			return iItem;

	return NULL;
}

/*	Returns true if the given item exists in the entities
	inventory.
*/
bool Item_CheckInventory(Item_t *iItem,edict_t *eEntity)
{
	int i;

	for(i = 0; i < sizeof(Items); i++)
		if(eEntity->v.iInventory[i] == iItem->iNumber)
			return true;

	return false;
}

/*	Return an item that we know is in our inventory.
	Safer alternative to the above.
*/
Item_t *Item_GetInventory(int iItemID,edict_t *eEntity)
{
	Item_t *iItem = Item_GetItem(iItemID);
	if(!iItem)
	{
		Engine.Con_Warning("Unknown item! (%i)\n",iItemID);
		return NULL;
	}

	if(Item_CheckInventory(iItem,eEntity))
		return iItem;

	return NULL;
}

/*	Adds an item to the entities inventory.
*/
void Item_AddInventory(Item_t *iItem,edict_t *eEntity)
{
	int i;

	for(i = 0; i < sizeof(Items); i++)
		if(!eEntity->v.iInventory[i])
		{
			eEntity->v.iInventory[i] = iItem->iNumber;
			// [25/6/2013] Oops! Don't forget to return ~hogsy
			return;
		}
		else if(eEntity->v.iInventory[i] == iItem->iNumber)
			return;
}

/*	Removes an item from the entities inventory.
*/
void Item_RemoveInventory(Item_t *iItem,edict_t *eEntity)
{
	int i,
		iLength	= sizeof(Items);

	// [12/5/2013] Go through, remove and reorder the array ~hogsy
	for(i = 0; i < iLength; i++)
	{
		if(eEntity->v.iInventory[i] == iItem->iNumber)
		{
			int j;
			for(j = i; j < iLength; j++)
				eEntity->v.iInventory[j] = eEntity->v.iInventory[j+1];

			iLength--;
			i--;
		}
	}
}

void Item_Respawn(edict_t *ent)
{
	Entity_SetModel(ent,ent->local.cOldModel);

	ent->Physics.iSolid = SOLID_TRIGGER;

	// [16/7/2012] Changed to ent rather than other... ~hogsy
	Sound(ent,CHAN_ITEM,"items/respawn.wav",255,ATTN_NORM);
}

void Item_Touch(edict_t *eItem,edict_t *eOther)
{
	Item_t *iItem;

	// Don't let monsters pick up items, and don't let dead players pick them up either.
	if(!Entity_IsPlayer(eOther) || eOther->v.iHealth <= 0)
		return;

	// Get the item reference.
	iItem = Item_GetItem(eItem->local.style);
	if(!iItem)
	{
		Engine.Con_Warning("Unknown item type! (%i)\n",eItem->local.style);
		return;
	}

	if (Item_CheckInventory(iItem, eOther))
	{
		Engine.Server_SinglePrint(eOther, "You already have the");
		Engine.Server_SinglePrint(eOther, va("\x02 %s", iItem->cName));
		Engine.Server_SinglePrint(eOther, "!\n");
		return;
	}

	if (iItem->cSound)
		Sound(eOther, CHAN_ITEM, iItem->cSound, 255, ATTN_NORM);
	else
		Engine.Con_Warning("No pickup sound set for %s!", iItem->cName);

	// [20/1/2013] Changed from Con_Printf to Server_SinglePrint ~hogsy
	Engine.Server_SinglePrint(eOther,"You got the");
	Engine.Server_SinglePrint(eOther,va("\x02 %s",iItem->cName));
	Engine.Server_SinglePrint(eOther,"!\n");

	// Check if we're in a multiplayer game, if so we'll respawn items.
	if(bIsMultiplayer)
	{
		eItem->local.cOldModel = eItem->v.model;

		Entity_SetModel(eItem,"models/blip.md2");

		eItem->v.dNextThink	= Server.dTime+60.0;
		eItem->v.think		= Item_Respawn;
	}
	else
		eItem->v.model = 0;

	eItem->Physics.iSolid = SOLID_NOT;

	// [14/7/2012] Cleaned up ~hogsy
	switch(iItem->iNumber)
	{
#ifdef OPENKATANA
	case ITEM_POWERBOOST:
		eOther->local.power_time		= 1.0f;
		eOther->local.power_finished	= Server.dTime+30.0;

		Item_AddInventory(iItem,eOther);
		break;
	case ITEM_ATTACKBOOST:
		eOther->local.attackb_time		= 1.0;
		eOther->local.attackb_finished	= Server.dTime+30.0;

		Item_AddInventory(iItem,eOther);
		break;
	case ITEM_ACROBOOST:
		eOther->local.acro_time		= 1.0;
		eOther->local.acro_finished	= Server.dTime+30.0;

		Item_AddInventory(iItem,eOther);
		break;
	case ITEM_VITABOOST:
		eOther->local.vita_time		= 1.0;
		eOther->local.vita_finished	= Server.dTime+30.0;

		Item_AddInventory(iItem,eOther);
		break;
	case ITEM_SPEEDBOOST:
		eOther->local.speed_time		= 1.0;
		eOther->local.speed_finished	= Server.dTime+30.0;

		Item_AddInventory(iItem,eOther);
		break;
	case ITEM_HEALTHKIT:
		eOther->v.iHealth += 20;
		if(eOther->v.iHealth > eOther->v.iMaxHealth)
			eOther->v.iHealth = eOther->v.iMaxHealth;
		break;
	/* Episode 1 */
	case WEAPON_IONBLASTER:
		eOther->local.ionblaster_ammo += 25;

		Item_AddInventory(iItem,eOther);
		break;
	case WEAPON_C4VIZATERGO:
		eOther->local.iC4Ammo += 15;

		Item_AddInventory(iItem,eOther);
		break;
	case WEAPON_SHOTCYCLER:
		// Take into consideration that this must be a multiple of 6.
		eOther->local.shotcycler_ammo += 60;

		Item_AddInventory(iItem,eOther);
		break;
	case WEAPON_SIDEWINDER:
		eOther->local.sidewinder_ammo += 16;

		Item_AddInventory(iItem,eOther);
		break;
	case WEAPON_SHOCKWAVE:
		eOther->local.shockwave_ammo += 10;

		Item_AddInventory(iItem,eOther);
		break;
	case WEAPON_DAIKATANA:
		Item_AddInventory(iItem,eOther);
		break;
	case AMMO_IONBALLS:
		eOther->local.ionblaster_ammo += 15;
		break;
	case AMMO_C4BOMBS:
		eOther->local.iC4Ammo += 5;
		break;
	case AMMO_SLUGS:
		eOther->local.shotcycler_ammo += 8;
		break;
	case AMMO_WINDROCKET:
		eOther->local.sidewinder_ammo += 6;
		break;
	case AMMO_SHOCKWAVE:
		eOther->local.shockwave_ammo += 1;
		break;
#elif GAME_ADAMAS
	case ITEM_LIFE:
		Server.iLives++;
		break;
#endif
	default:
		Engine.Con_Warning("Unknown item type (%i) for touch function!\n",eItem->local.style);

		Entity_Remove(eItem);
	}

	//WEAPON_SetCurrentAmmo(other);
}

// [14/7/2012] Renamed to Item_Spawn and heavily revised ~hogsy
void Item_Spawn(edict_t *eItem)
{
	Item_t	*iItem = Item_GetItem(eItem->local.style);

	if(!iItem)
	{
		Engine.Con_Warning("Unknown item type! (%i)\n",eItem->local.style);

		ENTITY_REMOVE(eItem);
	}

	// Start with this so these can be overwritten in a specific entcase
	eItem->Physics.iSolid	= SOLID_TRIGGER;
	eItem->v.movetype		= MOVETYPE_FLY;
	eItem->v.TouchFunction	= Item_Touch;
	eItem->v.flags			= FL_ITEM;

	if(eItem->local.style >= 30 && eItem->local.style <= 34)
		eItem->v.effects = EF_MOTION_FLOAT;
	else
		eItem->v.effects = EF_MOTION_ROTATE|EF_MOTION_FLOAT;

	Engine.Server_PrecacheResource(RESOURCE_SOUND,iItem->cSound);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,iItem->cModel);
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/blip.md2");

	Entity_SetModel(eItem,iItem->cModel);
	Entity_SetSize(eItem,-16.0f,-16.0f,-24.0f,16.0f,16.0f,32.0f);
	Entity_SetOrigin(eItem,eItem->v.origin);
	SetAngle(eItem,eItem->v.angles);
}

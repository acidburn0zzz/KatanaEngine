/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_main.h"

/*
	Standard Deathmatch Mode
	All vs All
*/

#include "server_item.h"
#include "server_weapon.h"

void Deathmatch_Spawn(edict_t *ePlayer)
{
	if(!Server.bRoundStarted)
	{
		ePlayer->v.model		= "";
		ePlayer->v.movetype		= MOVETYPE_NOCLIP;
		ePlayer->Physics.iSolid	= SOLID_BBOX;

		// [1/8/2013] Spectator mode! Hide the hud ~hogsy
		Server_UpdateClientMenu(ePlayer,MENU_STATE_HUD,false);

		// [15/12/2013] Make sure we have no model set ~hogsy
		Entity_SetModel(ePlayer,ePlayer->v.model);

		return;
	}

	// [29/7/2013] Clear our inventory ~hogsy
	memset(ePlayer->v.iInventory,0,sizeof(ePlayer->v.iInventory));

	// [29/7/2013] Add the default inventory ~hogsy
	Item_AddInventory(Item_GetItem(WEAPON_DAIKATANA),ePlayer);
	Item_AddInventory(Item_GetItem(WEAPON_IONBLASTER),ePlayer);

	// [30/7/2013] Add ammo before setting it as active! ~hogsy
	ePlayer->local.ionblaster_ammo = 45;

	// [29/7/2013] Set the IonBlaster as our active weapon ~hogsy
	Weapon_SetActive(Weapon_GetWeapon(WEAPON_IONBLASTER),ePlayer);

	Server_UpdateClientMenu(ePlayer,MENU_STATE_HUD,true);
}

void Deathmatch_Frame(void)
{
	if(!Server.bActive || (!bIsMultiplayer && !bIsDeathmatch))
		return;
	else if((Server.iClients >= cvServerGameClients.value) && !Server.bRoundStarted)
	{
		Server.bRoundStarted = true;

		Engine.Server_BroadcastPrint("Fight!\n");
		return;
	}

	// [5/9/2013] TODO: Check scores etc etc etc ~hogsy
	/*
	for( i = 0; i < Server.iClients; i++)
	{
		edict_t *eClient = getentnum(i);
		if(eClient->iScore >= cvServerMaxScore.value)
			you won!
	}
	*/
}

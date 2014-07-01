/*	Copyright (C) 2011-2014 OldTimes Software
*/
#include "server_main.h"

/*
	Rover
	Simple test vehicle for Ictus.
*/

#include "server_vehicle.h"

void Rover_Enter(edict_t *eRover,edict_t *eOther);

void Rover_Spawn(edict_t *eRover)
{
	Engine.Server_PrecacheResource(RESOURCE_MODEL,"models/vehicles/van.bsp");

	// [15/4/2013] Set up passengers ~hogsy
	eRover->Vehicle.iMaxPassengers	= 4;
	eRover->Vehicle.Enter			= Rover_Enter;

	Vehicle_Spawn(eRover);

	eRover->v.netname		= "Rover";
	eRover->v.bTakeDamage	= true;

	Entity_SetModel(eRover,"models/vehicles/van.bsp");

	eRover->Physics.fMass = 2.0f;
}

/*	Called for when the player enters into the Rover.
*/
void Rover_Enter(edict_t *eRover,edict_t *eOther)
{
}
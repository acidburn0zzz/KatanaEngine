#ifndef __SERVERVEHICLE__
#define __SERVERVEHICLE__

#include "server_main.h"

enum
{
	SLOT_OPEN,

	SLOT_DRIVER,
	SLOT_PASSENGER
};

typedef enum
{
	VEHICLE_TYPE_ROVER = 1,

	VEHICLE_TYPE_END
} VehicleType_t;

void Vehicle_Spawn(edict_t *eVehicle);
void Vehicle_Exit(edict_t *eVehicle,edict_t *eOther);

#endif

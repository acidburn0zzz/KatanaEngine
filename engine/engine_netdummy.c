/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "quakedef.h"

/*
    Basic Network Interface
*/

// Dummy interface for old system...

#include "engine_netloop.h"
#include "engine_netdgrm.h"
#include "engine_netserial.h"

net_driver_t net_drivers[MAX_NET_DRIVERS] =
{
	{
		"Loopback",
		false,
		Loop_Init,
		Loop_Listen,
		Loop_SearchForHosts,
		Loop_Connect,
		Loop_CheckNewConnections,
		Loop_GetMessage,
		Loop_SendMessage,
		Loop_SendUnreliableMessage,
		Loop_CanSendMessage,
		Loop_CanSendUnreliableMessage,
		Loop_Close,
		Loop_Shutdown
	}
};

net_landriver_t	net_landrivers[MAX_NET_DRIVERS];

int net_numdrivers      = 1,
    net_numlandrivers   = 0;

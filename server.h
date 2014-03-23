#include "shared_flags.h"

typedef struct
{
	int				maxclients;
	int				maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int				serverflags;		// episode completion information
	bool			bChangingLevel;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================

// [30/5/2013] Updated to follow new conventions ~hogsy
typedef enum
{
	SERVER_STATE_LOADING,
	SERVER_STATE_ACTIVE
} ServerState_t;

typedef struct
{
	bool	active;				// FALSE if only a net client
	bool	paused;
	bool	loadgame;			// handle connections specially

	double		time;

	int			lastcheck;			// used by PF_checkclient
	double		lastchecktime;

	char			name[64];			// map name
	char			modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s 	*worldmodel;
	char			*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	char			*sound_precache[MAX_SOUNDS];	// NULL terminated
	char			*cParticlePrecache[MAX_PARTICLES];
	char			*lightstyles[MAX_LIGHTSTYLES];
	int				num_edicts;
	int				max_edicts;
	edict_t			*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
	ServerState_t	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[MAX_DATAGRAM];

	sizebuf_t	signon;
	byte		signon_buf[MAX_MSGLEN-2]; //johnfitz -- was 8192, now uses MAX_MSGLEN

	unsigned	protocol; //johnfitz
} server_t;

#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		33

typedef struct client_s
{
	bool		active;				// FALSE = client is free
	bool		bSpawned;			// FALSE = don't send datagrams
	bool		dropasap;			// has been told to go to another level
	bool		privileged;			// can execute any host command
	bool		sendsignon;			// only valid before spawned

	double			last_message;		// reliable messages must be sent
										// periodically

	struct qsocket_s *netconnection;	// communications handle

	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd

	sizebuf_t		message;			// can be added to at any time,
										// copied and clear once per frame
	byte			msgbuf[MAX_MSGLEN];
	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// for printing to other people
	int				colors;

	float			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

	// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

	// client known data for deltas
	int				old_frags;
} client_t;

extern	cvar_t	teamplay;
extern	cvar_t	skill;
extern	cvar_t	deathmatch;
extern	cvar_t	coop;
extern	cvar_t	fraglimit;
extern	cvar_t	timelimit;

extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server

extern	client_t	*host_client;

extern	jmp_buf 	host_abortserver;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,float attenuation);
void SV_DropClient(bool crash);
void SV_SendClientMessages (void);
void SV_ClearDatagram (void);

int SV_ModelIndex (char *name);

void SV_SetIdealPitch (void);
void SV_AddUpdates (void);
void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s	*ret);
void SV_ClientPrintf (char *fmt, ...);
void SV_BroadcastPrintf (char *fmt, ...);

void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg);
void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms ();
void SV_SpawnServer (char *server);

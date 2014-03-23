#ifndef __SHAREDMODULE__
#define __SHAREDMODULE__

typedef struct
{
	void	(*Editor_Initialize)(void);

	int		iVersion;
} ModuleEditor_t;

#define MODULE_VERSION	(sizeof(ModuleExport_t)+sizeof(ModuleImport_t))
#define MODULE_VERSION2	(sizeof(edict_t*)+MODULE_VERSION)

#endif
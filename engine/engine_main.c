/*	Copyright (C) 2011-2015 OldTimes Software
*/
#include "engine_main.h"
#include "engine_video.h"

#include "platform_module.h"

EngineExport_t	eExport;
EngineImport_t	Launcher;

#if 0
/*	On failure returns false.
*/
bool Engine_Initialize(int argc,char *argv[])
{
    double dNewTime,
            dCurrentTime,
            dOldTime = 0;   // [22/7/2013] Set to 0 since we use it on the first frame ~hogsy

	for(;;)
    {
        dNewTime        = System_DoubleTime();
        dCurrentTime    = dNewTime-dOldTime;

        Host_Frame(dCurrentTime);

        dOldTime = dNewTime;
    }

    return true;
}
#endif

// [8/3/2014] Oops, this didn't match! Fixed ~hogsy
bool System_Main(int iArgumentCount,char *cArguments[]);

pMODULE_EXPORT EngineExport_t *Engine_Main(EngineImport_t *mImport)
{
	Launcher.iVersion	= mImport->iVersion;

	eExport.Initialize	= System_Main;
	eExport.iVersion = ENGINE_VERSION;

	return &eExport;
}

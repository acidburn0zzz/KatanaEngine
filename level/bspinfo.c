#include "bsp5.h"

static void BspInfo_PrintInfo(void)
{
	int length;
	FILE *file;

	// get file length
	file = SafeOpenRead( filename_bsp );
	length = Q_filelength( file );
	fclose( file );

	// print file length
	printf( "%s: %i bytes\n", filename_bsp, length );
	printf( "\n" );

	LoadBSPFile( filename_bsp );

	// print .bsp info
	PrintBSPFileSizes ();
}

int BspInfo_Main(void)
{
	// init memory
	Q_InitMem ();

	BspInfo_PrintInfo();

	// free allocated memory
	Q_ShutdownMem ();

	return 0;
}

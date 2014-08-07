/*	Copyright (C) 2014 OldTimes Software
*/
#include "python_main.h"

/*
	Entry point!
*/

int main(int argc,char *argv[])
{
	Py_SetProgramName(argv[0]);
	Py_Initialize();

	//Py_InitModule("katana",pMethods);

	{
		PyObject *pScript = PyFile_FromString("test.py","r");

		if(pScript)
			PyRun_SimpleFileEx(PyFile_AsFile(pScript),"test.py",1);
		else
			printf("Failed to load script!\n");
	}

	Py_Finalize();

	for(;;)
	{
	}

	return 0;
}
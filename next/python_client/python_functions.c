/*	Copyright (C) 2014 OldTimes Software
*/
#include "python_main.h"

/*
	Includes a collection of basic functions for testing.
*/

static PyObject *Functions_HelloWorld(PyObject *pSelf,PyObject *pArgs)
{
	printf("Hello World!\n");

	return 0;
}

static PyMethodDef	pMethods[]=
{
	{	"helloworld",	Functions_HelloWorld,	METH_NOARGS,	"Test function."	},

	{	NULL,	NULL,	0,	NULL	}
};
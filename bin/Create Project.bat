ECHO OFF
CLS
SET	VERSION=8
TITLE xSDK %VERSION%
GOTO MENU

:MENU
CLS 
COLOR 47
ECHO.
ECHO	             .M"""bgd `7MM"""Yb. `7MMF' `YMM' 
ECHO	            ,MI    "Y   MM    `Yb. MM   .M'   
ECHO	 `7M'   `MF'`MMb.       MM     `Mb MM .d"     
ECHO	   `VA ,V'    `YMMNq.   MM      MM MMMMM.     
ECHO	     XMX    .     `MM   MM     ,MP MM  VMA    
ECHO	   ,V' VA.  Mb     dM   MM    ,dP' MM   `MM.  
ECHO	 .AM.   .MA.P"Ybmmd"  .JMMmmmdP' .JMML.   MMb.
ECHO.  
ECHO  Copyright (C) 2014 OldTimes Software
ECHO.
ECHO.
ECHO Welcome to the xSDK! Please select an option below:
ECHO 1 - Create Project
ECHO 2 - Delete Project
ECHO 3 - Exit
ECHO.
SET /P M=
IF %M%==1 GOTO MENU_CREATEPROJECT
IF %M%==2 GOTO MENU_DELETEPROJECT
IF %M%==3 (
GOTO EXIT
) ELSE GOTO MENU

:MENU_CREATEPROJECT
ECHO MENU/CREATEPROJECT
ECHO.
PAUSE
GOTO MENU

:MENU_DELETEPROJECT
ECHO MENU/DELETEPROJECT
ECHO.
PAUSE
GOTO MENU

:EXIT
EXIT
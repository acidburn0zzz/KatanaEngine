ECHO OFF
CLS
SET	VERSION=8
TITLE xSDK %VERSION%
SET STM=%ProgramFiles(x86)%\Steam\Steam.exe
IF NOT EXIST "%ProgramFiles(x86)%\Steam\Steam.exe" (
SET STM=%ProgramFiles%\Steam\Steam.exe
IF NOT EXIST "%ProgramFiles%\Steam\Steam.exe" (
SET STM=Not found
)
)
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
ECHO STATUS:
ECHO    * STEAM = %STM%
IF NOT EXIST "%VPROJECT%" (
ECHO    * VPROJECT = Not found
) ELSE ECHO    * VPROJECT = %VPROJECT%
ECHO.
ECHO  Copyright (C) 2014 OldTimes Software
ECHO.
ECHO.
ECHO Welcome to xSDK! Please select an option below:
ECHO 1 - Create Project
ECHO 2 - Delete Project
ECHO 3 - Exit
ECHO.
SET /P M=
IF %M%==1 GOTO SE15
IF %M%==2 GOTO SE2B
IF %M%==3 (
GOTO EXIT
) ELSE GOTO MENU

:SE15
CLS
ECHO MENU/SE15
ECHO.
ECHO You selected Source Engine 15
ECHO Choose an option of what you want to do now below:
ECHO.
ECHO 1 - Compile a model
ECHO 2 - Open the SDK
ECHO 3 - Back to the menu!
ECHO.
SET /P G=
IF %G%==1 GOTO SE15_COMPILEMODEL
IF %G%==2 GOTO SE15_SDK
IF %G%==3 (
GOTO MENU
) ELSE GOTO SE15

:SE2B
CLS
ECHO MENU/SE2B
ECHO.
ECHO You selected Source Engine 2 Beta
ECHO Choose an option of what you want to do now below:
ECHO.
ECHO 1 - Convert BSP
ECHO 2 - Back to the menu!
ECHO.
SET /P GFG=
IF %GFG%==1 GOTO SE2B_CONVERTBSP
IF %GFG%==2 (
GOTO MENU
) ELSE GOTO SE2B

:SE15_COMPILEMODEL
CLS
ECHO MENU/SE15/SE15_COMPILEMODEL
ECHO.
ECHO Please enter your Steam name:
SET /P N=
ECHO. 
ECHO Please enter your qc files directory and the qc file you want
ECHO to compile (example mdlcompile.qc):
SET /P Q=
ECHO. 
IF EXIST "%ProgramFiles(x86)%" (
GOTO SE15_COMPILEMODEL_64
) ELSE GOTO SE15_COMPILEMODEL_32

:SE15_COMPILEMODEL_32
START /B "Model Compile (32)" "%ProgramFiles%\steam\steamapps\%N%\sourcesdk\bin\ep1\bin\studiomdl.exe" "%Q%"
GOTO SE15_COMPILEMODEL_COMPILING

:SE15_COMPILEMODEL_64
START /B "Model Compile (64)" "%ProgramFiles(x86)%\steam\steamapps\%N%\sourcesdk\bin\ep1\bin\studiomdl.exe" "%Q%"
GOTO SE15_COMPILEMODEL_COMPILING

:SE15_COMPILEMODEL_COMPILING
CLS
PAUSE
ECHO.
GOTO MENU

:SE15_SDK
START "SDK" "steam://rungameid/211"
GOTO SE15_SDK_WAIT

:SE15_SDK_WAIT
ECHO.
PAUSE
GOTO MENU

:SE2B_CONVERTBSP
CLS
ECHO MENU/SE2B/SE2B_CONVERTBSP
ECHO.
ECHO Please enter your Steam name:
SET /P C=
ECHO. 
ECHO Enter the name of the bsp file you wish to convert:
SET /P E=
ECHO.
ECHO Enter the name of the output file:
SET /P F=
ECHO.
IF EXIST "%ProgramFiles(x86)%" (
GOTO SE2B_CONVERTBSP_64
) ELSE GOTO SE2B_CONVERTBSP_32

:SE2B_CONVERTBSP_64
ECHO MENU/SE2B/SE2B_CONVERTBSP_64
ECHO.
START /B "BSP Convert (64)" "%ProgramFiles(x86)%\steam\steamapps\%C%\sourcesdk\bin\ep1\bin\bspconv.exe" "%E%" "%F%"
PAUSE
GOTO MENU

:SE2B_CONVERTBSP_32
ECHO MENU/SE2B/SE2B_CONVERTBSP_32
ECHO.
START /B "BSP Convert (32)" "%ProgramFiles%\steam\steamapps\%C%\sourcesdk\bin\ep1\bin\bspconv.exe" "%E%" "%F%"
PAUSE
GOTO MENU

:EXIT
EXIT
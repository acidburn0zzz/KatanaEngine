@"%~dp0\Level.x86.exe" -verbose -subdivide 240 %1
@"%~dp0\Level.x86.exe" -vis -fast %1
@"%~dp0\Level.x86.exe" -light -extra 3 %1
pause

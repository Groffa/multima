@echo off

set GAME_TITLE=%1
set LIBS=user32.lib kernel32.lib gdi32.lib Winmm.lib

if "%GAME_TITLE%"=="" (
    set GAME_TITLE=Multima
)

pushd w:\build\

del /Q w:\build\*.dll
del /Q w:\build\*.exe
del /Q w:\build\*.exp
del /Q w:\build\*.lib
del /Q w:\build\*.obj
del /Q w:\build\*.pdb

cl /DGAME_TITLE=\"%GAME_TITLE%\" /Zi ..\code\game.cpp /LDd
cl /DGAME_TITLE=\"%GAME_TITLE%\" /Zi /Fe%GAME_TITLE% ..\code\win32_platform.cpp %LIBS%
cl /DGAME_TITLE=\"%GAME_TITLE%\" /DMULTIMA_CONSOLE /Zi /Fe%GAME_TITLE%_console.exe ..\code\win32_platform.cpp %LIBS%

popd

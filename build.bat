@echo off

set GAME_TITLE=%1
set LIBS=user32.lib kernel32.lib gdi32.lib Winmm.lib

if "%GAME_TITLE%"=="" (
    set GAME_TITLE=multima
)

pushd w:\build\

del /Q w:\build\*.dll
del /Q w:\build\*.exe
del /Q w:\build\*.exp
del /Q w:\build\*.lib
del /Q w:\build\*.obj
del /Q w:\build\*.pdb

cl /Zi ..\code\win32_artpacker.cpp %LIBS%
if exist win32_artpacker.exe win32_artpacker.exe ..\arts\ ..\code\game_items
if %ERRORLEVEL%==0 (
cl /DGAME_TITLE=\"%GAME_TITLE%\" /O2 ..\code\game.cpp /LD
cl /DGAME_TITLE=\"%GAME_TITLE%\" /O2 /Fe%GAME_TITLE% ..\code\win32_platform.cpp %LIBS%

cl /DGAME_TITLE=\"%GAME_TITLE%\" /DMULTIMA_DEBUG /Zi /Fegame_debug.dll ..\code\game.cpp /LDd
cl /DGAME_TITLE=\"%GAME_TITLE%\" /DMULTIMA_DEBUG /Zi /Fe%GAME_TITLE%_debug.exe ..\code\win32_platform.cpp %LIBS%
) else (
echo Error: artpacker failed.
)

popd

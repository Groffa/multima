@echo off

set MYVIM=d:\vim\vim74
set MYVC=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC

subst w: /D
subst w: d:\programmering\c\multima
call "%MYVC%\vcvarsall.bat" x64
set path=w:\misc\;%MYVIM%;%path%
w:
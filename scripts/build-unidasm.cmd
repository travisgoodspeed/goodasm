@echo off
rem Build unidasm on Windows (GENie-generated gmake)
setlocal enabledelayedexpansion

set G=
if exist mame\build\projects\sdl\mame\gmake-msvc ( 
  set G=mame\build\projects\sdl\mame\gmake-msvc
) else (
  for /f "delims=" %%D in ('dir /b /s /ad mame\build\projects\*\*gmake* 2^>nul') do (
    if not defined G set G=%%D
  )
)

if not exist mame (
  echo mame\ not found -- cloning shallow repository (this may take a while)...
  git clone --depth 1 https://github.com/mamedev/mame.git || echo clone failed
)

if not defined G (
  echo Generated gmake tree not found. Running generator in mame\ (may take a while)...
  pushd mame
  call nmake TOOLS=1 generate 2>nul || (call mingw32-make TOOLS=1 generate 2>nul)
  popd
  for /f "delims=" %%D in ('dir /b /s /ad mame\build\projects\*\*gmake* 2^>nul') do (
    if not defined G set G=%%D
  )
)

if not defined G (
  echo Could not find generated gmake tree under mame\build\projects
  exit /b 1
)

echo Building unidasm from %G%

pushd %G%
rem Try to build with nmake or mingw32-make
call nmake unidasm 2>nul || call mingw32-make unidasm
set RET=%ERRORLEVEL%
popd

if not %RET%==0 (
  echo Build failed with exit %RET%
  exit /b %RET%
)

rem Try to copy built exe
set DEST=%USERPROFILE%\bin
if not exist "%DEST%" mkdir "%DEST%"

if exist mame\build\generated\unidasm\unidasm.exe (
  copy /Y mame\build\generated\unidasm\unidasm.exe "%DEST%\unidasm.exe"
) else if exist mame\unidasm.exe (
  copy /Y mame\unidasm.exe "%DEST%\unidasm.exe"
) else (
  echo Could not find unidasm.exe after build
  exit /b 1
)

echo Installed %DEST%\unidasm.exe
endlocal
exit /b 0

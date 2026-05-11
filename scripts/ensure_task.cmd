@echo off
REM Cross-platform ensure_task for Windows (cmd.exe)
where task >nul 2>nul
if %ERRORLEVEL%==0 (
  echo [ensure_task] Task is already installed.
  exit /b 0
)

where npm >nul 2>nul
if %ERRORLEVEL%==0 (
  echo [ensure_task] Installing Task via npm...
  npm install -g @go-task/cli
  exit /b %ERRORLEVEL%
)

where winget >nul 2>nul
if %ERRORLEVEL%==0 (
  echo [ensure_task] Installing Task via winget...
  winget install -e --id Task.Task
  exit /b %ERRORLEVEL%
)

REM Try choco as a fallback
where choco >nul 2>nul
if %ERRORLEVEL%==0 (
  echo [ensure_task] Installing Task via choco...
  choco install go-task -y
  exit /b %ERRORLEVEL%
)

REM Could not install
>&2 echo [ensure_task] ERROR: Could not find a supported package manager to install Task.
exit /b 1

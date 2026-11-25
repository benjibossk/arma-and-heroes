@echo off
rem Arma & Heroes orchestrator - long-lived loop (Ctrl+C to stop).
rem Restarts the loop after a crash; exits if another loop already runs (orchestrator.pid).
cd /d "%~dp0"
:loop
python orchestrator.py %*
if errorlevel 1 if exist orchestrator.pid goto :eof
echo orchestrator exited (code %errorlevel%), restart in 30 s...
timeout /t 30 /nobreak >nul
goto loop

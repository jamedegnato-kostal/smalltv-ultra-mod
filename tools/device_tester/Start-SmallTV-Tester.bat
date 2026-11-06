@echo off
setlocal
cd /d "%~dp0\..\.."
py -3 tools\device_tester\smalltv_tester.py
if errorlevel 1 (
  echo.
  echo Tester konnte nicht gestartet werden.
  pause
)

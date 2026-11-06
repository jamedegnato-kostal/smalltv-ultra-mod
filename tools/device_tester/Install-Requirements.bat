@echo off
setlocal
py -3 -m pip install --user -r "%~dp0requirements.txt"
if errorlevel 1 (
  echo Installation fehlgeschlagen. Siehe tools\device_tester\README.md.
  pause
)

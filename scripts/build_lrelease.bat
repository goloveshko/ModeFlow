@echo off
rem Convenience wrapper - equivalent to: build.bat --lrelease [...]
call "%~dp0build.bat" --lrelease %*

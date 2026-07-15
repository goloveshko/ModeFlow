@echo off
rem Convenience wrapper - equivalent to: build.bat --debug [...]
call "%~dp0build.bat" --debug --ninja %*

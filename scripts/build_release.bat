@echo off
rem Convenience wrapper - equivalent to: build.bat --release --static --ninja [...]
call "%~dp0build.bat" --release --static --ninja %*

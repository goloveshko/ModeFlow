@echo off
rem Convenience wrapper - equivalent to: build.bat --lupdate [...]
call "%~dp0build.bat" --lupdate %*

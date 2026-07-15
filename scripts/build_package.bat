@echo off
rem Convenience wrapper - equivalent to: build.bat --release --static --ninja --package [...]
call "%~dp0build.bat" --release --static --ninja --package %*

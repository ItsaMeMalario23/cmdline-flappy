@echo off

gcc -o flappy.exe -I ./inc/ ./src/*.c ./src/debug/*.c

flappy

if not errorlevel 0 echo Exit code: %errorlevel%
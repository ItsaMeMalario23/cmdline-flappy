@echo off

gcc -o flappy.exe -I ./inc/ ./src/*

flappy

echo Exit code: %errorlevel%
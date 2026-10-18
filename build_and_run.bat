@echo off
REM Windows (MinGW-w64 g++). Run from the SmartStock folder.
g++ -std=c++17 -O2 -D_WIN32_WINNT=0x0A00 -Iinclude -Isrc src\main.cpp -o smartstock.exe -lws2_32 -lmswsock
if errorlevel 1 (echo Build failed & pause & exit /b 1)
start "" http://localhost:8080
smartstock.exe

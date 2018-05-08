# Kiwi-Software-Rasterizer

This repository contains a software rasterizer implemented in C++. The program is built for visual studio with the .sln files stored in build_win32.

Build: To build the program, open a command promt and navigate to the build directory. Run build.bat and the project should be built.

Program Entry Point: The main entry point for the program is found at win32_kiwi.cpp while the rasterizers main entry point is found in the function GameUpdateAndRender at kiwi.cpp. 

Controls: The program has no controls but you can set which scene you want to render in the #if statements found in GameUpdateAndRender.

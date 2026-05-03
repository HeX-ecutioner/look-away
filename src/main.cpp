/*  main.cpp: LookAway entry point
 *  Uses WinMain so no console window appears when the .exe is launched.
 *  The CMakeLists sets WIN32 subsystem so the linker agrees.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    App app;

    if (!app.init(hInstance))
        return -1;

    app.run();
    app.shutdown();

    return 0;
}
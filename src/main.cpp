#define WIN32_LEAN_AND_MEAN // CMakeLists sets WIN32 subsystem so the linker agrees
#include <windows.h>
#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) // Uses WinMain so no console window appears when the .exe is launched
{
    App app;

    if (!app.init(hInstance))
        return -1;

    app.run();
    app.shutdown();

    return 0;
}
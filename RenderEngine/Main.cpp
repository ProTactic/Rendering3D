#include "Engine.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
   Engine engine;

    if (!engine.Initialize("Rendering Engine", "Engine", 600, 400))
    {
        return 0;
    }

    engine.EngineLoop();

    return 0;
}

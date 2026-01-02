#include "../../dxlatest.h"
#include "../Shared/Window.h"

int32_t APIENTRY WinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] char* lpCmdLine, [[maybe_unused]] int32_t nCmdShow)
{
    Window window(hInstance, "SingleTriangle");

    window.ShowWindow();
    while (window.IsAlive())
    {
        /*if (!window.IsMinimized())
        {
            Update_Internal();

            Render_Internal();
        }*/

        window.MessageLoop();
    }

    return 0;
}
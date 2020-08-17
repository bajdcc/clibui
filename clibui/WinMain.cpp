// CCGameFramework.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ui/window/Window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    auto window = std::make_shared<Window>(nullptr, _T("CC_CLIBUI_CLS"), _T("Clibui Window"), hInstance);
    window->Run();
    window.reset();
    return 0;
}
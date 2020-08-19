#ifndef UI_WINDOWMSGLOOP_H
#define UI_WINDOWMSGLOOP_H

#include <ui/gdi/Gdi.h>

class WindowMsgLoop
{
public:
    WindowMsgLoop();

    void Run();
    BOOL Event();
    BOOL PumpMessage();

    using MSGMAP = std::unordered_map<UINT, CString>;
    LPCTSTR DebugGetMessageName(UINT message);

private:
    int m_nDisablePumpCount{ 0 };
    UINT m_nMsgLast{ 0 };
    MSG m_msg{ 0 };
    CPoint m_ptMousePos;
    MSGMAP m_msgMap;
};

#endif
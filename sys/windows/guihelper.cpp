
#include "guihelper.h"

#include <QWidget>

#include <windows.h>

void Windows_QWidget_SetForegroundWindow(QWidget* w) {
    HWND hForgroundWnd = GetForegroundWindow();
    DWORD dwForeID = ::GetWindowThreadProcessId(hForgroundWnd, NULL);
    DWORD dwCurID = ::GetCurrentThreadId();
    ::AttachThreadInput(dwCurID, dwForeID, TRUE);
    ::SetForegroundWindow((HWND) w->winId());
    ::AttachThreadInput(dwCurID, dwForeID, FALSE);
}

#include "MiniDump.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>

#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>

typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD dwPid,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG __stdcall CreateCrashHandler(EXCEPTION_POINTERS *pException) {
    QDir::setCurrent(QApplication::applicationDirPath());

    HMODULE DllHandle = NULL;
    DllHandle = LoadLibrary(_T("DBGHELP.DLL"));

    if (DllHandle) {
        MINIDUMPWRITEDUMP Dump = (MINIDUMPWRITEDUMP) GetProcAddress(DllHandle, "MiniDumpWriteDump");
        if (Dump) {
            // 创建 Dump 文件
            QDateTime CurDTime = QDateTime::currentDateTime();
            QString current_date = CurDTime.toString("yyyy_MM_dd_hh_mm_ss");
            // dmp文件的命名
            QString dumpText = "Dump_" + current_date + ".dmp";
            EXCEPTION_RECORD *record = pException->ExceptionRecord;
            QString errCode(QString::number(record->ExceptionCode, 16));
            QString errAddr(QString::number((uintptr_t) record->ExceptionAddress, 16));
            QString errFlag(QString::number(record->ExceptionFlags, 16));
            QString errPara(QString::number(record->NumberParameters, 16));
            HANDLE DumpHandle = CreateFile((LPCWSTR) dumpText.utf16(),
                                           GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (DumpHandle != INVALID_HANDLE_VALUE) {
                MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
                dumpInfo.ExceptionPointers = pException;
                dumpInfo.ThreadId = GetCurrentThreadId();
                dumpInfo.ClientPointers = TRUE;
                // 将dump信息写入dmp文件
                Dump(GetCurrentProcess(), GetCurrentProcessId(), DumpHandle, MiniDumpNormal, &dumpInfo,
                     NULL, NULL);
                CloseHandle(DumpHandle);
            } else {
                dumpText = "";
            }
            // 创建消息提示
            QMessageBox::warning(NULL, "Application crashed",
                                 QString("ErrorCode: %1 ErrorAddr:%2 ErrorFlag: %3 ErrorPara: %4\nVersion: %5\nDump file at %6")
                                     .arg(errCode)
                                     .arg(errAddr)
                                     .arg(errFlag)
                                     .arg(errPara)
                                     .arg(NKR_VERSION)
                                     .arg(dumpText),
                                 QMessageBox::Ok);
        }
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void Windows_SetCrashHandler() {
    SetErrorMode(SEM_FAILCRITICALERRORS);
    SetUnhandledExceptionFilter(CreateCrashHandler);
}

#include "CrashHandler.h"

#include <QDir>

#include "Logging.h"

#ifdef Q_OS_WIN
// clang-format off
#include <windows.h>
#include <DbgHelp.h>
#include <strsafe.h>
// clang-format on

namespace {
// Global static buffer for the path (so we don't have to allocate memory on crash)
wchar_t g_dumpFolderPath[MAX_PATH] = {0};

LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* pExceptionPointers) {
    if (g_dumpFolderPath[0] == L'\0') {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Build filename with timestamp using WinAPI to avoid heap allocations during a crash
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);

    wchar_t dumpFilePath[MAX_PATH];
    StringCchPrintfW(dumpFilePath, MAX_PATH, L"%s\\ModeFlow_crash_%04d%02d%02d_%02d%02d%02d.dmp", g_dumpFolderPath,
                     stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, stLocalTime.wHour, stLocalTime.wMinute,
                     stLocalTime.wSecond);

    HANDLE hFile = CreateFileW(dumpFilePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = pExceptionPointers;
        mei.ClientPointers = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mei, nullptr, nullptr);
        CloseHandle(hFile);
    }

    // Return to allow Windows to display the standard termination dialog
    return EXCEPTION_EXECUTE_HANDLER;
}
} // namespace
#endif

namespace ModeFlow::Utils {

void CrashHandler::setup(const QString& dumpDirPath) {
#ifdef Q_OS_WIN
    // Create dump directory if missing
    QDir dir(dumpDirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Pre-save path to global buffer for crash-time access
    QString nativePath = QDir::toNativeSeparators(dir.absolutePath());
    wcsncpy_s(g_dumpFolderPath, MAX_PATH, reinterpret_cast<const wchar_t*>(nativePath.utf16()), _TRUNCATE);

    // Register unhandled exception filter
    SetUnhandledExceptionFilter(UnhandledExceptionHandler);
    qCDebug(lcUtil) << "CrashHandler initialized. Dump folder:" << nativePath;
#endif
}

} // namespace ModeFlow::Utils

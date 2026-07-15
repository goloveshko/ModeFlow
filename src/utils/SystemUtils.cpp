#include "SystemUtils.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QOperatingSystemVersion>
#include <QProcess>
#include <QScreen>
#include <QSettings>
#include <vector>

#include "Logging.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace ModeFlow::Utils {

using namespace Qt::StringLiterals;

bool SystemUtils::isSystemReady() {
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return false;
    }

    if (QGuiApplication::screens().isEmpty()) {
        return false;
    }

    if (!QGuiApplication::primaryScreen() || QGuiApplication::primaryScreen()->name().isEmpty()) {
        return false;
    }

    if (auto* primary = QGuiApplication::primaryScreen()) {
        qCDebug(lcUtil) << "Primary screen:" << primary->name() << ", model:" << primary->model()
                        << ", manufacturer:" << primary->manufacturer() << ", serialNumber:" << primary->serialNumber()
                        << ", size:" << primary->size();
    }

    return true;
}

bool SystemUtils::isDesktopActive() {
    // Attempt to open the input desktop.
    // If this fails, the Logon screen or UAC prompt is likely blocking hardware access.
    HDESK hDesk = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
    if (hDesk) {
        CloseDesktop(hDesk);
        return true;
    }
    return false;
}

QString SystemUtils::getExecutableDir() {
    DWORD bufferSize = MAX_PATH;
    std::vector<wchar_t> buffer(bufferSize);

    while (true) {
        const DWORD copied = GetModuleFileNameW(NULL, buffer.data(), bufferSize);
        if (copied == 0)
            return QString();

        if (copied < bufferSize) {
            return QFileInfo(QString::fromWCharArray(buffer.data(), copied)).absolutePath();
        }

        bufferSize *= 2;
        buffer.resize(bufferSize);
    }
}

bool SystemUtils::isWin11() {
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11;
}

bool SystemUtils::isWin11_22H2() {
    return QOperatingSystemVersion::current() >=
           QOperatingSystemVersion(QOperatingSystemVersion::Windows, 10, 0, 22621);
}

bool SystemUtils::isSystemDarkMode() {
    if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10) {
        return false;
    }
    // Theme settings in Windows 10/11 are stored here
    QSettings settings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
                       QSettings::NativeFormat);

    // SystemUsesLightTheme = 0 means Dark Mode for system (taskbar)
    return !settings.value(u"AppsUseLightTheme"_s, true).toBool();
}

void SystemUtils::configureWindowButtons(QWidget* widget, bool allowMinimize, bool allowMaximize) {
    if (!widget)
        return;

#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());
    LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);

    if (allowMinimize) {
        style |= static_cast<LONG_PTR>(WS_MINIMIZEBOX);
    } else {
        style &= ~static_cast<LONG_PTR>(WS_MINIMIZEBOX);
    }

    if (allowMaximize) {
        style |= static_cast<LONG_PTR>(WS_MAXIMIZEBOX);
    } else {
        style &= ~static_cast<LONG_PTR>(WS_MAXIMIZEBOX);
    }

    ::SetWindowLongPtrW(hwnd, GWL_STYLE, style);
    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
#endif
}

bool SystemUtils::isValidExecutablePath(const QString& path) {
    if (path.isEmpty())
        return false;
    QFileInfo fi(path);
    return fi.exists() && fi.isFile() && !fi.isSymLink() && fi.suffix().toLower() == "exe";
}

bool SystemUtils::terminateProcess(qint64 pid) {
    if (pid <= 0)
        return false;

#ifdef Q_OS_WIN
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (hProcess) {
        BOOL result = TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        return result == TRUE;
    }
#endif

    return false;
}

bool SystemUtils::restartApplication(const QStringList& arguments) {
    qCDebug(lcUtil) << "Spawning detached application instance with args:" << arguments;
    return QProcess::startDetached(QCoreApplication::applicationFilePath(), arguments);
}

} // namespace ModeFlow::Utils

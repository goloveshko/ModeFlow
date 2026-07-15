#pragma once

#include <QByteArray>
#include <QString>
#include <QWidget>

namespace ModeFlow::Utils {

class SystemUtils {
public:
    // Returns true if the Windows Graphics driver and Qt Screen system are initialized
    static bool isSystemReady();

    // Returns true if the interactive user desktop is active (not the logon screen)
    static bool isDesktopActive();

    // Optional: a helper to get the executable's directory without relying on QCoreApplication
    static QString getExecutableDir();

    static bool isWin11();
    static bool isWin11_22H2();
    static bool isSystemDarkMode();
    static void configureWindowButtons(QWidget* widget, bool allowMinimize, bool allowMaximize);

    static bool isValidExecutablePath(const QString& path);

    static bool terminateProcess(qint64 pid);

    static bool restartApplication(const QStringList& arguments);
};
} // namespace ModeFlow::Utils

#pragma once

#include <QString>

namespace ModeFlow::Utils {

class TaskScheduler {
public:
    // Register the program in the scheduler
    // arguments - launch arguments
    // taskName - the name of the task in the list (optional, auto-filled if empty)
    // exePath - the full path to the program (optional, auto-filled if empty)
    static bool createTaskAtLogon(const QString& arguments = "", int delaySeconds = 0, bool runHighest = true,
                                  QString taskName = QString(), QString exePath = QString());

    static bool removeTask(QString taskName = QString());

    static bool isTaskRegistered(QString taskName = QString(), QString exePath = QString());

    static bool isAdmin();
    static bool runAsAdmin(const QString& argument);
};
} // namespace ModeFlow::Utils

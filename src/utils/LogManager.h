#pragma once

#include <QFile>
#include <QMutex>

namespace ModeFlow::Utils {

class LogManager {
    Q_DISABLE_COPY_MOVE(Utils::LogManager)

public:
    static void setup(bool enabled);
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:
    static QFile s_logFile;
    static QMutex s_mutex;
    static bool s_enabled;
};
} // namespace ModeFlow::Utils

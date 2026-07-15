#include "LogManager.h"

#include <QDir>

#include "Constants.h"
#include "Logging.h"
#include "SystemUtils.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

using namespace Qt::StringLiterals;

namespace ModeFlow::Utils {

namespace {
QString simplifyFunction(const char* rawFunction) {
    if (!rawFunction)
        return u"unknown"_s;

    std::string_view func(rawFunction);

    const size_t parenPos = func.find('(');
    if (parenPos != std::string_view::npos) {
        func = func.substr(0, parenPos);
    }

    const size_t lastSpace = func.rfind(' ');
    if (lastSpace != std::string_view::npos) {
        func = func.substr(lastSpace + 1);
    }

    const size_t lastColons = func.rfind("::");
    if (lastColons != std::string_view::npos) {
        const size_t secondLastColons = func.rfind("::", lastColons - 1);
        if (secondLastColons != std::string_view::npos) {
            func = func.substr(secondLastColons + 2);
        }
    }

    return QString::fromUtf8(func.data(), static_cast<int>(func.size()));
}
} // namespace

QFile LogManager::s_logFile;
QMutex LogManager::s_mutex;
bool LogManager::s_enabled = false;

void LogManager::setup(bool enabled) {
    s_enabled = enabled;
    if (!enabled)
        return;

    // Get path using Windows API (Works before QApplication exists)
    QString appDir = SystemUtils::getExecutableDir();

    QString fileName = "/log.txt";
    QString targetPath = appDir + fileName;

    // Test if writable, else use Temp
    QString logPath;
    QFile testFile(targetPath);
    if (testFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        testFile.close();
        logPath = targetPath;
    } else {
        logPath = QDir::tempPath() + fileName;
    }

    // Rotation (if > 5MB)
    if (QFile(logPath).size() > MaxLogFileSizeBytes) {
        QFile::remove(logPath);
    }

    // Open file once and keep it open for the session
    s_logFile.setFileName(logPath);
    if (!s_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qCWarning(lcUtil) << "Failed to open log file:" << logPath;
    }

    qInstallMessageHandler(LogManager::messageHandler);
}

void LogManager::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (!s_enabled || !s_logFile.isOpen())
        return;

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString typeStr;
    switch (type) {
    case QtDebugMsg:
        typeStr = "DEBUG";
        break;
    case QtInfoMsg:
        typeStr = "INFO ";
        break;
    case QtWarningMsg:
        typeStr = "WARN ";
        break;
    case QtCriticalMsg:
        typeStr = "CRIT ";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    }

    QString func = simplifyFunction(context.function);

    QString formattedMsg =
        u"["_s % time % u"] ["_s % typeStr % u"] ["_s % context.category % u"] ["_s % func % u"] "_s % msg % u"\n"_s;

    OutputDebugStringW(reinterpret_cast<const wchar_t*>(formattedMsg.utf16()));

    QMutexLocker locker(&s_mutex);
    if (s_logFile.isOpen()) {
        QTextStream out(&s_logFile);
        out << formattedMsg;
        out.flush();
    }
}
} // namespace ModeFlow::Utils

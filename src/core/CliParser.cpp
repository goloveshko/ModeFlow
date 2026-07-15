#include "CliParser.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

namespace ModeFlow::Core {

CliParser::Result CliParser::parse(const QStringList& arguments) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("ModeFlow System Utility"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption optRegister(TaskArgs::Register.toString(), "Register the autostart task in Task Scheduler.");
    QCommandLineOption optUnregister(TaskArgs::Unregister.toString(), "Remove the autostart task from Task Scheduler.");
    QCommandLineOption optLogon(TaskArgs::Logon.toString(), "Started via Task Scheduler (logon).");
    QCommandLineOption optLog(TaskArgs::Log.toString(), "Enable diagnostic logging.");
    QCommandLineOption optDelay(TaskArgs::Delay.toString(), "Set logon delay in seconds.", "seconds", "0");
    QCommandLineOption optSilent(TaskArgs::SilentRestart.toString(), "Internal silent restart flag.");

    parser.addOption(optRegister);
    parser.addOption(optUnregister);
    parser.addOption(optLogon);
    parser.addOption(optLog);
    parser.addOption(optDelay);
    parser.addOption(optSilent);

    parser.parse(arguments);

    Result result;

    if (parser.isSet(optRegister)) {
        result.mode = RunMode::RegisterTask;
    } else if (parser.isSet(optUnregister)) {
        result.mode = RunMode::UnregisterTask;
    } else {
        result.mode = RunMode::NormalGui;
    }

    result.options.isLogon = parser.isSet(optLogon);
    result.options.isSilentRestart = parser.isSet(optSilent);
    result.options.enableLogging = parser.isSet(optLog);

    bool ok = false;
    result.options.delaySeconds = parser.value(optDelay).toInt(&ok);
    if (!ok) {
        result.options.delaySeconds = 0;
    }

    return result;
}

} // namespace ModeFlow::Core
#pragma once

#include <QCoreApplication>

#include "CliArgs.h"

namespace ModeFlow::Core {

class CliParser {
public:
    enum class RunMode { NormalGui, RegisterTask, UnregisterTask };

    struct Result {
        RunMode mode = RunMode::NormalGui;
        StartupOptions options;
    };

    static Result parse(const QStringList& arguments);
};

} // namespace ModeFlow::Core

#pragma once

#include <QString>

namespace ModeFlow::Utils {

/**
 * @brief Handles unhandled exceptions and generates minidumps for debugging.
 */
class CrashHandler {
public:
    // Initializes the crash interceptor. Call main() at the beginning.
    // dumpDirPath is the folder where the dumps will be saved.
    static void setup(const QString& dumpDirPath);

private:
    // Prevent instantiation, class is fully static
    CrashHandler() = default;
};

} // namespace ModeFlow::Utils
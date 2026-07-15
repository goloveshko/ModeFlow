#pragma once
#include <QLoggingCategory>

// Declare categories for everything
Q_DECLARE_LOGGING_CATEGORY(lcMain)
Q_DECLARE_LOGGING_CATEGORY(lcCore)    // Profile logic, configs
Q_DECLARE_LOGGING_CATEGORY(lcGui)     // Windows, dialogs, styles
Q_DECLARE_LOGGING_CATEGORY(lcService) // WinAPI, Displays, Audio, Hotkeys
Q_DECLARE_LOGGING_CATEGORY(lcUtil)
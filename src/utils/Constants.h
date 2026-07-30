#pragma once

#include <QStringView>

/**
 * @brief Global constants for ModeFlow application.
 *
 * This file contains all magic numbers used in the application.
 * Each constant has a descriptive name and documentation.
 */

namespace ModeFlow::Utils {

// ============================================================================
// Timeouts and Delays (in milliseconds)
// ============================================================================

/** Local socket connection timeout (ms) */
constexpr int LocalSocketTimeoutMs = 500;
static_assert(LocalSocketTimeoutMs >= 100 && LocalSocketTimeoutMs <= 5000,
              "LocalSocketTimeoutMs must be between 100ms and 5s");

/** Delay before checking autostart status (ms) */
constexpr int AutostartStatusCheckDelayMs = 100;
static_assert(AutostartStatusCheckDelayMs >= 100 && AutostartStatusCheckDelayMs <= 10000,
              "AutostartStatusCheckDelayMs must be between 100ms and 10s");

/** System readiness poll interval at startup (ms) */
constexpr int SystemReadyPollIntervalMs = 500;
static_assert(SystemReadyPollIntervalMs >= 10 && SystemReadyPollIntervalMs <= 500,
              "SystemReadyPollIntervalMs must be between 10ms and 500ms");

/** Maximum number of system readiness poll attempts */
constexpr int SystemReadyMaxAttempts = 20;
static_assert(SystemReadyMaxAttempts >= 10 && SystemReadyMaxAttempts <= 10000,
              "SystemReadyMaxAttempts must be between 10 and 10000");

/** Default display switch timeout (ms) */
constexpr int DisplaySwitchTimeoutMs = 10000;
static_assert(DisplaySwitchTimeoutMs >= 1000 && DisplaySwitchTimeoutMs <= 60000,
              "DisplaySwitchTimeoutMs must be between 1s and 60s");

/** Debounce interval for display events (ms) */
constexpr int DisplayDebounceIntervalMs = 500;
static_assert(DisplayDebounceIntervalMs >= 100 && DisplayDebounceIntervalMs <= 5000,
              "DisplayDebounceIntervalMs must be between 100ms and 5s");

/** Tray notification duration (ms) */
constexpr int TrayNotificationDurationMs = 3000;
static_assert(TrayNotificationDurationMs >= 1000 && TrayNotificationDurationMs <= 30000,
              "TrayNotificationDurationMs must be between 1s and 30s");

/** Maximum time to wait for the elevated helper process (ms) */
constexpr int ElevatedHelperTimeoutMs = 30000;
static_assert(ElevatedHelperTimeoutMs >= 1000 && ElevatedHelperTimeoutMs <= 120000,
              "ElevatedHelperTimeoutMs must be between 1s and 120s");

// ============================================================================
// Sizes and Limits
// ============================================================================

/** Maximum log file size before rotation (bytes) */
constexpr int MaxLogFileSizeBytes = 5 * 1024 * 1024; // 5 MB

/** Maximum application launch delay in profile (seconds) */
constexpr int MaxAppLaunchDelaySeconds = 300; // 5 minutes

inline constexpr int ProfileAutosaveDebounceMs = 350;

inline constexpr QStringView DefaultQtStyleKey = u"windows11";

// ============================================================================
// Auto-Update
// ============================================================================

/** Minimum interval between automatic update checks (ms) — 24 hours */
constexpr int UpdateCheckIntervalMs = 24 * 3600 * 1000;

} // namespace ModeFlow::Utils

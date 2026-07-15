#pragma once

namespace ModeFlow::Utils {

/**
 * @brief Lightweight, zero-pollution RAII Guard for Windows COM initialization.
 * Keeps standard <windows.h> out of global headers to prevent macro conflicts.
 */
class ComInitGuard {
public:
    // 2 corresponds to COINIT_APARTMENTTHREADED in Windows API
    explicit ComInitGuard(unsigned long coInit = 2);
    ~ComInitGuard();

    bool isOk() const;

    ComInitGuard(const ComInitGuard&) = delete;
    ComInitGuard& operator=(const ComInitGuard&) = delete;
    ComInitGuard(ComInitGuard&&) = delete;
    ComInitGuard& operator=(ComInitGuard&&) = delete;

private:
    bool m_ok = false;
    long m_hr = 0; // HRESULT is internally represented as a long
};

} // namespace ModeFlow::Utils

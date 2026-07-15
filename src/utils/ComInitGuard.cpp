#include "ComInitGuard.h"

namespace ModeFlow::Utils {

ComInitGuard::ComInitGuard(unsigned long coInit) {
#ifdef Q_OS_WIN
    m_hr = CoInitializeEx(nullptr, static_cast<DWORD>(coInit));
    m_ok = SUCCEEDED(m_hr) || m_hr == RPC_E_CHANGED_MODE;
#else
    m_hr = 0;
    m_ok = true;
#endif
}

ComInitGuard::~ComInitGuard() {
#ifdef Q_OS_WIN
    if (SUCCEEDED(m_hr)) {
        CoUninitialize();
    }
#endif
}

bool ComInitGuard::isOk() const {
    return m_ok;
}

} // namespace ModeFlow::Utils
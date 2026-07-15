#include "DeviceUtils.h"

#include <algorithm>

namespace ModeFlow::Utils {

QList<Core::DeviceEntry> DeviceUtils::sortAndGroupDevices(const QList<Core::DeviceEntry>& source) {
    QList<Core::DeviceEntry> active;
    QList<Core::DeviceEntry> disconnected;

    for (const auto& dev : source) {
        if (dev.isConnected) {
            active.append(dev);
        } else {
            disconnected.append(dev);
        }
    }

    auto sortByName = [](const Core::DeviceEntry& a, const Core::DeviceEntry& b) {
        return a.name.localeAwareCompare(b.name) < 0;
    };
    std::sort(active.begin(), active.end(), sortByName);
    std::sort(disconnected.begin(), disconnected.end(), sortByName);

    QList<Core::DeviceEntry> result = active;
    result.append(disconnected);
    return result;
}

} // namespace ModeFlow::Utils
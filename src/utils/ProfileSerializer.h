#pragma once

#include <QList>

#include "ConfigTypes.h"

namespace ModeFlow::Utils {

class ProfileSerializer {
public:
    static bool exportProfiles(const QList<Core::WorkspaceConfig>& profiles, const QString& filePath);
    static QList<Core::WorkspaceConfig> importProfiles(const QString& filePath, QString& errorOut);
    static bool isValidProfileFile(const QString& filePath);
};

} // namespace ModeFlow::Utils

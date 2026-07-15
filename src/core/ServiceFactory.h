#pragma once

#include <QObject>

#include "AppServices.h"

namespace ModeFlow::Core {

class ServiceFactory {
public:
    static void createCoreServices(AppServices& services);
    static void createHardwareServices(AppServices& services, QObject* parent);
    static void createWorkspaceWindow(AppServices& services);
};

} // namespace ModeFlow::Core

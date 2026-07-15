#pragma once

#include "AppServices.h"

namespace ModeFlow::Core {

class AppController;

class ServiceWiring {
public:
    static void wireErrorConnections(AppServices& s, AppController* controller);
    static void wireServiceConnections(AppServices& s, AppController* controller);
    static void wireWindowConnections(AppServices& s, AppController* controller);
};

} // namespace ModeFlow::Core

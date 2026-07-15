#pragma once

#include <QLocalServer>

namespace ModeFlow::Services {

class SingleInstanceGuard : public QObject {
    Q_OBJECT
public:
    explicit SingleInstanceGuard(QObject* parent = nullptr);
    ~SingleInstanceGuard();

    bool tryToRun();
    void shutdown();

signals:
    void signalRaiseWindow();

private:
    void setupConnections();

    QLocalServer* m_server;

    static QString generateUniqueServerName();
    static QString generateSharedSecret();
};
} // namespace ModeFlow::Services
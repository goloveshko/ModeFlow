#pragma once

#include <QObject>

namespace ModeFlow::Services {

struct DeviceInfo {
    QString id;            ///< Unique device identifier
    QString displayName;   ///< Human-readable device name
    QString interfaceName; ///< Audio interface name
    QString endpointName;  ///< Endpoint name
    bool isInput;          ///< true if input device, false if output
    bool isConnected;      ///< true if device is currently connected
    bool isDefault;        ///< true if device is the system default
};

class AudioDeviceManagerPrivate;

class AudioDeviceManager : public QObject {
    Q_OBJECT
public:
    explicit AudioDeviceManager(QObject* parent = nullptr);
    ~AudioDeviceManager() override;

    QList<DeviceInfo> getAllDevices();
    QList<DeviceInfo> getOutputDevices();
    QList<DeviceInfo> getInputDevices();
    DeviceInfo getDeviceById(const QString& id);

    virtual QString getDefaultOutputDeviceId();
    QString getDefaultInputDeviceId();

    /**
     * @brief Sets the default output device.
     * @param id Device identifier from getOutputDevices().
     * @emits errorOccurred if device not found or access denied.
     */
    virtual void setDefaultOutputDevice(const QString& id);

    /**
     * @brief Sets the default input device.
     * @param id Device identifier from getInputDevices().
     * @emits errorOccurred if device not found or access denied.
     */
    virtual void setDefaultInputDevice(const QString& id);

    void setDefaultCommunicationOutputDevice(const QString& id);

    bool isSystemReady() const;

signals:
    void deviceAdded(const QString& id);
    void deviceRemoved(const QString& id);
    void defaultDeviceChanged(const QString& id);

    void deviceAddedInternal(const QString& id);
    void deviceRemovedInternal(const QString& id);
    void defaultDeviceChangedInternal(const QString& id);

    void errorOccurred(const QString& message);

public slots:
    void onSetDefaultOutputDevice(const QString& id);
    void onSetDefaultInputDevice(const QString& id);

private:
    DeviceInfo mapInfo(const void* infoPtr, const std::string& defaultId);
    QList<DeviceInfo> getDevicesByDirection(int direction);
    QString getDeviceName(const QString& id);

private:
    std::unique_ptr<AudioDeviceManagerPrivate> d;
};

} // namespace ModeFlow::Services
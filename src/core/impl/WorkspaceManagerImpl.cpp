#include "WorkspaceManagerImpl.h"

#include "AudioDeviceManager.h"
#include "ConfigManager.h"
#include "DeviceUtils.h"
#include "DisplayManager.h"
#include "FontAwesome.h"
#include "WorkspaceModel.h"

namespace ModeFlow::Core {

using namespace Qt::StringLiterals;

namespace {
struct ProfileIconRule {
    QString symbol;
    QStringList keywords;
};

const QVector<ProfileIconRule>& profileIconRules() {
    static const QVector<ProfileIconRule> rules = {
        {Gui::FontAwesome::Tv, {u"tv"_s, u"movie"_s, u"video"_s, u"cinema"_s, u"телевиз"_s, u"кино"_s, u"видео"_s}},
        {Gui::FontAwesome::Gamepad, {u"game"_s, u"play"_s, u"игра"_s, u"гейм"_s, u"gaming"_s}},
        {Gui::FontAwesome::Briefcase, {u"work"_s, u"office"_s, u"job"_s, u"рабочее"_s, u"офис"_s}},
        {Gui::FontAwesome::House, {u"home"_s, u"house"_s, u"дом"_s, u"домаш"_s}},
        {Gui::FontAwesome::Music, {u"music"_s, u"audio"_s, u"sound"_s, u"музык"_s, u"аудио"_s, u"звук"_s}},
        {Gui::FontAwesome::Laptop, {u"laptop"_s, u"notebook"_s, u"ноут"_s, u"лаптоп"_s}},
        {Gui::FontAwesome::Monitor, {u"monitor"_s, u"screen"_s, u"display"_s, u"монитор"_s, u"экран"_s, u"дисплей"_s}},
        {Gui::FontAwesome::Desktop, {u"desktop"_s, u"pc"_s, u"рабочий стол"_s, u"пк"_s, u"компьютер"_s}},
    };
    return rules;
}
} // namespace

WorkspaceManagerImpl::WorkspaceManagerImpl(ConfigManager* cm, Services::DisplayManager* dm,
                                           Services::AudioDeviceManager* am)
    : m_config(cm), m_display(dm), m_audio(am) {
    m_model = std::make_unique<WorkspaceModel>(nullptr);
    m_model->setActiveIdGetter([cm]() { return cm->lastActiveProfileId(); });
    m_model->setConfigs(m_config->getWorkspaces());
}

WorkspaceManagerImpl::~WorkspaceManagerImpl() = default;

WorkspaceModel* WorkspaceManagerImpl::model() const {
    return m_model.get();
}
void WorkspaceManagerImpl::addConfig(const WorkspaceConfig& cfg) {
    m_model->addConfig(cfg);
}
void WorkspaceManagerImpl::removeConfig(int row) {
    m_model->removeConfig(row);
}
void WorkspaceManagerImpl::updateConfig(int row, const WorkspaceConfig& cfg) {
    m_model->updateConfig(row, cfg);
}

WorkspaceConfig WorkspaceManagerImpl::captureCurrentHardwareState() const {
    WorkspaceConfig cfg;
    cfg.displayId = m_display->getCurrentDisplayKey();
    cfg.audioId = m_audio->getDefaultOutputDeviceId();
    return cfg;
}

QList<WorkspaceConfig> WorkspaceManagerImpl::getAllWorkspaceConfigs() const {
    return m_model->configs();
}

bool WorkspaceManagerImpl::saveWorkspaces() {
    m_config->setWorkspaces(m_model->configs());
    return m_config->saveConfig();
}

void WorkspaceManagerImpl::setSelectedRow(int row) {
    const auto& configs = m_model->configs();
    if (row >= 0 && row < configs.size())
        m_config->setSelectedProfileId(configs[row].id);
    else
        m_config->setSelectedProfileId(QString());
}

int WorkspaceManagerImpl::selectedRow() const {
    const int selectedRow = m_model->rowOfId(m_config->selectedProfileId());
    return selectedRow != -1 ? selectedRow : m_model->rowOfId(m_config->lastActiveProfileId());
}

int WorkspaceManagerImpl::activeRow() const {
    return m_model->rowOfId(m_config->lastActiveProfileId());
}

QList<DeviceEntry> WorkspaceManagerImpl::getAvailableDisplays() const {
    QList<DeviceEntry> res;
    for (const auto& m : m_display->getPhysicalMonitors())
        res.append({m.key, m.friendlyName, m.isActive, m.isPrimary});
    return Utils::DeviceUtils::sortAndGroupDevices(res);
}

QList<DeviceEntry> WorkspaceManagerImpl::getAvailableAudioOutputs() const {
    QList<DeviceEntry> res;
    for (const auto& d : m_audio->getOutputDevices())
        res.append({d.id, d.displayName, d.isConnected, d.isDefault});
    return Utils::DeviceUtils::sortAndGroupDevices(res);
}

QString WorkspaceManagerImpl::generateDefaultName() {
    const auto configs = getAllWorkspaceConfigs();

    auto isTaken = [&](const QString& name) {
        return std::any_of(configs.begin(), configs.end(),
                           [&](const auto& cfg) { return cfg.name.compare(name, Qt::CaseInsensitive) == 0; });
    };

    QString nameDesktop = QObject::tr("Desktop");
    if (!isTaken(nameDesktop)) {
        return nameDesktop;
    }

    QString nameTV = QObject::tr("TV");
    if (!isTaken(nameTV)) {
        return nameTV;
    }

    QString nameBase = QObject::tr("Workspace");
    if (!isTaken(nameBase)) {
        return nameBase;
    }

    for (int i = 1; i < 100; ++i) {
        QString candidate = u"%1 %2"_s.arg(nameBase, QString::number(i));
        if (!isTaken(candidate)) {
            return candidate;
        }
    }

    return nameBase;
}

void WorkspaceManagerImpl::createDefaultProfile() {
    WorkspaceConfig cfg;
    cfg.name = generateDefaultName();
    cfg.id = QUuid::createUuid().toString();
    cfg.iconSymbol = suggestedProfileIconSymbol(cfg.name);

    addConfig(cfg);
    saveWorkspaces();
}

void WorkspaceManagerImpl::duplicateProfile(int row) {
    const auto configs = getAllWorkspaceConfigs();
    if (row < 0 || row >= configs.size()) {
        return;
    }

    WorkspaceConfig duplicate = configs[row];
    duplicate.id = QUuid::createUuid().toString();
    duplicate.name = QObject::tr("%1 (Copy)").arg(configs[row].name);
    duplicate.hotkey = QKeySequence(); // Reset hotkey to prevent direct system conflicts

    addConfig(duplicate);
    saveWorkspaces();
}

QString WorkspaceManagerImpl::suggestedProfileIconSymbol(const QString& profileName) const {
    const QString normalized = profileName.trimmed().toLower();
    for (const auto& rule : profileIconRules()) {
        for (const auto& keyword : rule.keywords) {
            if (normalized.contains(keyword)) {
                return rule.symbol;
            }
        }
    }
    return Gui::FontAwesome::defaultProfileIconSymbol();
}

} // namespace ModeFlow::Core

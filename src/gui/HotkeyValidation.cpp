#include "HotkeyValidation.h"

#include <QCoreApplication>
#include <QStringBuilder>

#include "DialogManager.h"

using namespace Qt::StringLiterals;

namespace ModeFlow::Gui::HotkeyValidation {

QString describeConflict(const QKeySequence& key, const QKeySequence& nextProfileHotkey,
                         const QList<Core::WorkspaceConfig>& configs, const QString& currentProfileId) {
    if (key.isEmpty())
        return {};
    const QString keyName = key.toString(QKeySequence::NativeText);

    if (!nextProfileHotkey.isEmpty() && key == nextProfileHotkey) {
        return QCoreApplication::translate("HotkeyValidation", "Shortcut %1 is already assigned to 'Next profile'.")
            .arg(keyName);
    }

    for (const auto& cfg : configs) {
        if (cfg.id == currentProfileId || cfg.hotkey.isEmpty()) {
            continue;
        }

        if (key == cfg.hotkey) {
            return QCoreApplication::translate("HotkeyValidation", "Shortcut %1 is already assigned to profile '%2'.")
                .arg(keyName, cfg.name);
        }
    }

    return {};
}

bool applyChange(QPointer<HotkeyEdit> edit, const QKeySequence& newKey, const QKeySequence& oldKey,
                 const QString& conflictText, DialogManager* dialogManager, QWidget* parent, bool interactive) {
    if (conflictText.isEmpty())
        return true;

    if (!interactive) {
        return false;
    }

    if (!edit)
        return false;

    edit->setValidating(true);

    {
        QSignalBlocker blocker(edit);
        const QString displayMessage =
            conflictText % u"\n\n"_sv %
            QCoreApplication::translate("HotkeyValidation",
                                        "This shortcut cannot be used here. Reverting to previous value.");
        if (dialogManager) {
            dialogManager->showWarning(parent, QCoreApplication::translate("HotkeyValidation", "Hotkey Conflict"),
                                       displayMessage);
        }
        edit->setKeySequence(oldKey);
    }

    edit->setValidating(false);

    return false;
}

bool validateProfileHotkey(HotkeyEdit* edit, const QKeySequence& nextProfileHotkey,
                           const QList<Core::WorkspaceConfig>& configs, const QString& currentProfileId,
                           DialogManager* dialogManager, QWidget* parent) {
    const QKeySequence currentKey = edit->keySequence();
    const QKeySequence baseKey = edit->lastAcceptedKey();

    if (currentKey == baseKey)
        return false;

    const QString conflictText = describeConflict(currentKey, nextProfileHotkey, configs, currentProfileId);

    bool accepted = applyChange(edit, currentKey, baseKey, conflictText, dialogManager, parent);

    if (accepted) {
        edit->setLastAcceptedKey(currentKey);
    }

    return accepted;
}

} // namespace ModeFlow::Gui::HotkeyValidation

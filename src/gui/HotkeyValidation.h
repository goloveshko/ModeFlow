#pragma once

#include <QList>
#include <QPointer>

#include "ConfigTypes.h"
#include "HotkeyEdit.h"

class QWidget;

namespace ModeFlow::Gui {
class DialogManager;
}

namespace ModeFlow::Gui::HotkeyValidation {

QString describeConflict(const QKeySequence& key, const QKeySequence& nextProfileHotkey,
                         const QList<Core::WorkspaceConfig>& configs, const QString& currentProfileId = QString());

bool applyChange(QPointer<HotkeyEdit> edit, const QKeySequence& newKey, const QKeySequence& oldKey,
                 const QString& conflictText, DialogManager* dialogManager, QWidget* parent, bool interactive = true);

bool validateProfileHotkey(HotkeyEdit* edit, const QKeySequence& nextProfileHotkey,
                           const QList<Core::WorkspaceConfig>& configs, const QString& currentProfileId,
                           DialogManager* dialogManager, QWidget* parent);

} // namespace ModeFlow::Gui::HotkeyValidation

#include "HotkeyEdit.h"

#include <QFocusEvent>

namespace ModeFlow::Gui {

HotkeyEdit::HotkeyEdit(QWidget* parent) : QKeySequenceEdit(parent) {
    // Clear default frame if custom QSS styling is desired
    setClearButtonEnabled(true);
    connect(this, &QKeySequenceEdit::editingFinished, this, &HotkeyEdit::validateRequested);
}

void HotkeyEdit::setLastAcceptedKey(const QKeySequence& seq) {
    m_baseKey = seq;
    // Block signals to prevent false validation triggers
    QSignalBlocker blocker(this);
    setKeySequence(seq);
}

void HotkeyEdit::focusInEvent(QFocusEvent* event) {
    QKeySequenceEdit::focusInEvent(event);
    if (!m_isValidating) {
        emit captureChanged(true);
    }
}

void HotkeyEdit::focusOutEvent(QFocusEvent* event) {
    // IMPORTANT: call base class first to record changes
    QKeySequenceEdit::focusOutEvent(event);

    if (!m_isValidating) {
        emit validateRequested();
        emit captureChanged(false);
    }
}

void HotkeyEdit::keyPressEvent(QKeyEvent* event) {
    // Custom handling: Escape resets to last valid key
    if (event->key() == Qt::Key_Escape) {
        setKeySequence(m_baseKey);
        clearFocus(); // Exit editing mode
        return;
    }

    // Backspace at start of input can clear hotkey
    if (event->key() == Qt::Key_Backspace && keySequence().isEmpty()) {
        setLastAcceptedKey(QKeySequence());
        clearFocus();
        return;
    }

    QKeySequenceEdit::keyPressEvent(event);
}

} // namespace ModeFlow::Gui
#pragma once

#include <QKeySequenceEdit>

namespace ModeFlow::Core {
class IStyleManager;
}

namespace ModeFlow::Gui {

/**
 * @brief Specialized hotkey input field with automatic capture state management.
 */
class HotkeyEdit : public QKeySequenceEdit {
    Q_OBJECT

public:
    explicit HotkeyEdit(QWidget* parent = nullptr);

    // Sets the "last valid" hotkey to revert to on cancel
    void setLastAcceptedKey(const QKeySequence& seq);
    QKeySequence lastAcceptedKey() const { return m_baseKey; }

    // Flag to prevent recursion during programmatic text/focus changes
    bool isValidating() const { return m_isValidating; }
    void setValidating(bool validating) { m_isValidating = validating; }

signals:
    /**
     * @brief Emitted when the field gains "capture" focus (disable global hotkeys)
     * or loses it (re-enable).
     */
    void captureChanged(bool active);

    /**
     * @brief Called on actual focus loss to trigger external validation.
     */
    void validateRequested();

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QKeySequence m_baseKey;
    bool m_isValidating = false;
};

} // namespace ModeFlow::Gui
#include "LogHighlighter.h"

#include "StyleBridge.h"

namespace ModeFlow::Gui {

LogHighlighter::LogHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    updateColors();
}

void LogHighlighter::updateColors() {
    auto& bridge = StyleBridge::instance();

    m_rules.clear();

    m_timestampFormat.setForeground(bridge.logTimestamp());
    m_categoryFormat.setForeground(bridge.logCategory());
    m_functionFormat.setForeground(bridge.logFunction());

    m_debugFormat.setForeground(bridge.logDebug());
    m_debugFormat.setFontWeight(QFont::Bold);

    m_infoFormat.setForeground(bridge.logInfo());
    m_infoFormat.setFontWeight(QFont::Bold);

    m_warnFormat.setForeground(bridge.logWarning());
    m_warnFormat.setFontWeight(QFont::Bold);

    m_critFormat.setForeground(bridge.logCritical());
    m_critFormat.setFontWeight(QFont::Bold);

    m_fatalFormat.setForeground(bridge.logFatal());
    m_fatalFormat.setFontWeight(QFont::Bold);

    rehighlight();
}

void LogHighlighter::highlightBlock(const QString& text) {
    if (text.isEmpty() || !text.startsWith(u'[')) {
        return;
    }

    // Parse blocks structure manually for speed and safety
    int firstClose = text.indexOf(u']');
    if (firstClose == -1)
        return;

    setFormat(0, firstClose + 1, m_timestampFormat);

    int secondOpen = text.indexOf(u'[', firstClose + 1);
    int secondClose = text.indexOf(u']', firstClose + 1);
    if (secondOpen == -1 || secondClose == -1)
        return;

    QStringView levelStr = QStringView(text).mid(secondOpen + 1, secondClose - secondOpen - 1).trimmed();
    QTextCharFormat levelFmt = m_infoFormat;
    if (levelStr == u"DEBUG")
        levelFmt = m_debugFormat;
    else if (levelStr == u"WARN")
        levelFmt = m_warnFormat;
    else if (levelStr == u"CRIT")
        levelFmt = m_critFormat;
    else if (levelStr == u"FATAL")
        levelFmt = m_fatalFormat;

    setFormat(secondOpen, secondClose - secondOpen + 1, levelFmt);

    int thirdOpen = text.indexOf(u'[', secondClose + 1);
    int thirdClose = text.indexOf(u']', secondClose + 1);
    if (thirdOpen != -1 && thirdClose != -1) {
        setFormat(thirdOpen, thirdClose - thirdOpen + 1, m_categoryFormat);

        int fourthOpen = text.indexOf(u'[', thirdClose + 1);
        int fourthClose = text.indexOf(u']', thirdClose + 1);
        if (fourthOpen != -1 && fourthClose != -1) {
            setFormat(fourthOpen, fourthClose - fourthOpen + 1, m_functionFormat);
        }
    }
}

} // namespace ModeFlow::Gui
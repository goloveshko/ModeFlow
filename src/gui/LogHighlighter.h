#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace ModeFlow::Gui {

class LogHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit LogHighlighter(QTextDocument* parent = nullptr);
    void updateColors();

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightRule> m_rules;
    QTextCharFormat m_timestampFormat;
    QTextCharFormat m_categoryFormat;
    QTextCharFormat m_functionFormat;

    QTextCharFormat m_debugFormat;
    QTextCharFormat m_infoFormat;
    QTextCharFormat m_warnFormat;
    QTextCharFormat m_critFormat;
    QTextCharFormat m_fatalFormat;
};

} // namespace ModeFlow::Gui
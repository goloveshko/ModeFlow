#pragma once

#include <QTimer>

#include "BaseDialog.h"

namespace Ui {
class LogViewerDialog;
}

namespace ModeFlow::Gui {

class LogHighlighter;

class LogViewerDialog : public BaseDialog {
    Q_OBJECT

public:
    explicit LogViewerDialog(Core::IStyleManager* sm, QWidget* parent = nullptr);
    ~LogViewerDialog() override;

protected:
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct LogEntry {
        QString rawLine;
        QString timestamp;
        QString category;
        QString function;
        QString message;
        int level = 1; // 0: DEBUG, 1: INFO, 2: WARN, 3: CRIT, 4: FATAL
    };

    void init();
    void setupConnections();
    void loadLogFile();
    void incrementalLoad();
    void rebuildDisplay();
    void updateStatusBar();
    void scrollToBottom();
    void refreshCategories(const QSet<QString>& categories);

    static LogEntry parseLine(const QString& line);
    static QString levelToString(int level);

    void onTimerTick();
    void onFollowToggled(bool checked);
    void onRefreshClicked();
    void onClearClicked();
    void onCopyClicked();
    void onSaveClicked();
    void onFilterTextChanged(const QString& text);
    void onLevelFilterChanged(int index);
    void onCategoryFilterChanged(int index);
    void onApplyFilters();

    std::unique_ptr<Ui::LogViewerDialog> ui;
    LogHighlighter* m_highlighter = nullptr;
    QString m_logFilePath;
    QList<LogEntry> m_allEntries;
    QList<int> m_filteredIndices;

    QTimer m_refreshTimer;
    QTimer m_filterTimer;
    qint64 m_fileSize = 0;
    bool m_followMode = true;
    bool m_isRefreshing = false;
};

} // namespace ModeFlow::Gui
#pragma once

#include <QObject>

namespace ModeFlow::Services {
/**
 * @brief Service for playing audio feedback sounds.
 *
 * Responsible for user audio notifications:
 * - Confirmation sounds (successful actions)
 * - Error sounds (failed actions)
 */
class AudioFeedbackService : public QObject {
    Q_OBJECT

public:
    explicit AudioFeedbackService(QObject* parent = nullptr);
    ~AudioFeedbackService() override = default;

public slots:
    void playConfirmation();

    void playError();
};
} // namespace ModeFlow::Services

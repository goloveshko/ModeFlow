#include "AudioFeedbackService.h"

#include <QApplication>
#include <QTimer>

namespace ModeFlow::Services {
AudioFeedbackService::AudioFeedbackService(QObject* parent) : QObject(parent) {}

void AudioFeedbackService::playConfirmation() {
    QApplication::beep();
}

void AudioFeedbackService::playError() {
    QApplication::beep();
    QTimer::singleShot(150, this, []() { QApplication::beep(); });
}
} // namespace ModeFlow::Services

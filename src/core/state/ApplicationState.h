#pragma once

#include <QObject>

namespace ssa::core::state {

class ApplicationState : public QObject {
    Q_OBJECT
    Q_PROPERTY(AppState currentState READ currentState NOTIFY stateChanged)

public:
    enum AppState {
        IDLE,
        RECORDING,
        STOPPING,
        PROCESSING,
        READY,
        PLAYING,
        PAUSED,
        EXPORTING,
        ERROR_STATE,
        SCREENSHOT_EDITOR
    };
    Q_ENUM(AppState)

    explicit ApplicationState(QObject* parent = nullptr);

    AppState currentState() const;
    
public slots:
    void transitionTo(AppState newState);

signals:
    void stateChanged(AppState newState);

private:
    AppState m_currentState;
};

} // namespace ssa::core::state

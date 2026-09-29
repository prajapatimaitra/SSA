#include "ApplicationState.h"
#include "core/logging/Logger.h"
#include <fmt/core.h>

namespace ssa::core::state {

ApplicationState::ApplicationState(QObject* parent) 
    : QObject(parent), m_currentState(IDLE) {
}

ApplicationState::AppState ApplicationState::currentState() const {
    return m_currentState;
}

void ApplicationState::transitionTo(AppState newState) {
    if (m_currentState == newState) return;
    
    m_currentState = newState;
    logging::Logger::info("Application state transitioned to " + std::to_string(static_cast<int>(newState)));
    emit stateChanged(m_currentState);
}

} // namespace ssa::core::state

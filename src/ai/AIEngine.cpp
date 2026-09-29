#include "AIEngine.h"
#include "core/logging/Logger.h"

namespace ssa::ai {

AIEngine::AIEngine(QObject* parent) : QObject(parent) {}

AIEngine::~AIEngine() {
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void AIEngine::addModel(std::unique_ptr<IAIModel> model) {
    if (model) {
        m_models.push_back(std::move(model));
    }
}

void AIEngine::runProcessing(const QString& projectPath) {
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    m_workerThread = std::thread(&AIEngine::processThread, this, projectPath);
}

void AIEngine::processThread(QString projectPath) {
    core::logging::Logger::info("AIEngine: Started background processing for " + projectPath.toStdString());
    
    for (auto& model : m_models) {
        if (!model->process(projectPath.toStdString())) {
            core::logging::Logger::error("AIEngine: Model processing failed.");
        }
    }
    
    core::logging::Logger::info("AIEngine: Completed background processing.");
    
    // Emit finished on the main thread via Qt
    QMetaObject::invokeMethod(this, [this, projectPath]() {
        emit processingFinished(projectPath);
    });
}

} // namespace ssa::ai

#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <thread>
#include <vector>
#include "IAIModel.h"

namespace ssa::ai {

class AIEngine : public QObject {
    Q_OBJECT
public:
    explicit AIEngine(QObject* parent = nullptr);
    ~AIEngine();

    void addModel(std::unique_ptr<IAIModel> model);
    void runProcessing(const QString& projectPath);

signals:
    void processingFinished(const QString& projectPath);

private:
    void processThread(QString projectPath);

    std::vector<std::unique_ptr<IAIModel>> m_models;
    std::thread m_workerThread;
};

} // namespace ssa::ai

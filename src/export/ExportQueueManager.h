#pragma once

#include <QObject>
#include <QString>
#include <QQueue>
#include <memory>
#include "export/ExportEngine.h"

namespace ssa::export_engine {

struct ExportJob {
    QString projectPath;
    QString presetName;
    QString resolution;
};

class ExportQueueManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isExporting READ isExporting NOTIFY isExportingChanged)
    Q_PROPERTY(QString currentProjectName READ currentProjectName NOTIFY currentProjectNameChanged)
    Q_PROPERTY(double currentProgress READ currentProgress NOTIFY currentProgressChanged)
    Q_PROPERTY(int queueSize READ queueSize NOTIFY queueSizeChanged)

public:
    explicit ExportQueueManager(QObject* parent = nullptr);
    ~ExportQueueManager() override;

    Q_INVOKABLE void enqueueExport(const QString& projectPath, const QString& presetName, const QString& resolution);
    Q_INVOKABLE void cancelCurrentExport();
    Q_INVOKABLE void clearQueue();

    bool isExporting() const { return m_isExporting; }
    QString currentProjectName() const { return m_currentProjectName; }
    double currentProgress() const { return m_currentProgress; }
    int queueSize() const { return m_queue.size(); }

signals:
    void isExportingChanged(bool isExporting);
    void currentProjectNameChanged(const QString& name);
    void currentProgressChanged(double progress);
    void queueSizeChanged(int size);
    void jobFinished(bool success, const QString& path);
    void queueCompleted();

private slots:
    void onEngineProgress(double progress);
    void onEngineFinished(bool success, const QString& path);

private:
    void processNextJob();

    QQueue<ExportJob> m_queue;
    std::unique_ptr<ExportEngine> m_currentEngine;
    
    bool m_isExporting = false;
    QString m_currentProjectName;
    double m_currentProgress = 0.0;
};

} // namespace ssa::export_engine

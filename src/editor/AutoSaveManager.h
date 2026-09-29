#pragma once

#include <QObject>
#include <QTimer>
#include <QString>

namespace ssa::editor {

class EditorController;

class AutoSaveManager : public QObject {
    Q_OBJECT
public:
    explicit AutoSaveManager(EditorController* controller, QObject* parent = nullptr);
    ~AutoSaveManager();

    void start(const QString& projectPath);
    void stopAndCommit();

private slots:
    void performAutoSave();

private:
    EditorController* m_controller;
    QTimer m_timer;
    QString m_projectPath;
};

} // namespace ssa::editor

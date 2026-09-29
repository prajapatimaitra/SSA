#pragma once

#include <QObject>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QVariantList>
#include <QString>

namespace ssa::editor {

class AudioWaveformExtractor : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList waveform READ waveform NOTIFY waveformReady)
    Q_PROPERTY(bool isExtracting READ isExtracting NOTIFY extractionStateChanged)

public:
    explicit AudioWaveformExtractor(QObject* parent = nullptr);
    ~AudioWaveformExtractor();

    Q_INVOKABLE void startExtraction(const QString& audioFilePath);

    QVariantList waveform() const;
    bool isExtracting() const;

signals:
    void waveformReady();
    void extractionStateChanged();

private slots:
    void onBufferReady();
    void onFinished();
    void onError(QAudioDecoder::Error error);

private:
    void processBuffer(const QAudioBuffer& buffer);

    QAudioDecoder* m_decoder;
    QVariantList m_waveform;
    bool m_isExtracting = false;
    
    // Internal tracking for grouping samples
    qint64 m_samplesProcessed;
    double m_currentPeak;
    
    static constexpr int SAMPLES_PER_CHUNK = 2000; // Adjust resolution
};

} // namespace ssa::editor

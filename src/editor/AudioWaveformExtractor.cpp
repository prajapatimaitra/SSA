#include "AudioWaveformExtractor.h"
#include "core/logging/Logger.h"
#include <QAudioFormat>
#include <QFile>
#include <QUrl>
#include <cmath>
#include <iostream>

namespace ssa::editor {

AudioWaveformExtractor::AudioWaveformExtractor(QObject* parent)
    : QObject(parent), m_samplesProcessed(0), m_currentPeak(0)
{
    m_decoder = new QAudioDecoder(this);
    
    // Qt 6 specific signals
    connect(m_decoder, &QAudioDecoder::bufferReady, this, &AudioWaveformExtractor::onBufferReady);
    connect(m_decoder, &QAudioDecoder::finished, this, &AudioWaveformExtractor::onFinished);
    connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), this, [this](QAudioDecoder::Error error) {
        this->onError(error);
    });
}

AudioWaveformExtractor::~AudioWaveformExtractor() {
    if (m_decoder) {
        m_decoder->stop();
    }
}

void AudioWaveformExtractor::startExtraction(const QString& audioFilePath) {
    if (m_isExtracting) {
        m_decoder->stop();
    }
    
    m_waveform.clear();
    m_samplesProcessed = 0;
    m_currentPeak = 0;
    
    m_isExtracting = true;
    emit extractionStateChanged();
    
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    
    m_decoder->setAudioFormat(format);
    
    QUrl url;
    if (audioFilePath.startsWith("file://")) {
        url = QUrl(audioFilePath);
    } else {
        url = QUrl::fromLocalFile(audioFilePath);
    }
    
    m_decoder->setSource(url);
    m_decoder->start();
    
    core::logging::Logger::info("Started audio waveform extraction for: " + audioFilePath.toStdString());
}

QVariantList AudioWaveformExtractor::waveform() const {
    return m_waveform;
}

bool AudioWaveformExtractor::isExtracting() const {
    return m_isExtracting;
}

void AudioWaveformExtractor::onBufferReady() {
    QAudioBuffer buffer = m_decoder->read();
    if (!buffer.isValid()) return;
    
    processBuffer(buffer);
}

void AudioWaveformExtractor::onFinished() {
    // Push the final chunk if we have partial data
    if (m_currentPeak > 0) {
        m_waveform.append(m_currentPeak);
    }
    
    m_isExtracting = false;
    emit extractionStateChanged();
    emit waveformReady();
    
    core::logging::Logger::info("Finished audio waveform extraction. Generated " + std::to_string(m_waveform.size()) + " data points.");
}

void AudioWaveformExtractor::onError(QAudioDecoder::Error error) {
    core::logging::Logger::error("Audio decoding error: " + std::to_string(static_cast<int>(error)));
    m_isExtracting = false;
    emit extractionStateChanged();
    // Still emit waveformReady so the UI can draw whatever we managed to extract (or empty)
    emit waveformReady();
}

void AudioWaveformExtractor::processBuffer(const QAudioBuffer& buffer) {
    const qint16 *data = buffer.constData<qint16>();
    int count = buffer.sampleCount();
    
    for (int i = 0; i < count; ++i) {
        // Normalize the sample to 0.0 - 1.0
        double sample = std::abs(data[i]) / 32768.0;
        
        if (sample > m_currentPeak) {
            m_currentPeak = sample;
        }
        
        m_samplesProcessed++;
        
        if (m_samplesProcessed >= SAMPLES_PER_CHUNK) {
            m_waveform.append(m_currentPeak);
            m_currentPeak = 0;
            m_samplesProcessed = 0;
        }
    }
}

} // namespace ssa::editor

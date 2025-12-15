#include "TtsEngine.h"
#include <QDebug>

TtsEngine& TtsEngine::instance() {
    static TtsEngine instance;
    return instance;
}

TtsEngine::TtsEngine() {
#ifdef HAVE_QT_TTS
    m_speech = new QTextToSpeech(this);
    
    QList<QVoice> voices = m_speech->availableVoices();
    for (const QVoice &voice : voices) {
        if (voice.name().contains("English", Qt::CaseInsensitive) || 
            voice.name().contains("US", Qt::CaseInsensitive) ||
            voice.name().contains("UK", Qt::CaseInsensitive)) {
            m_speech->setVoice(voice);
            break;
        }
    }
#endif
}

void TtsEngine::speak(const QString& text) {
#ifdef HAVE_QT_TTS
    if (m_speech->state() == QTextToSpeech::Speaking) {
        m_speech->stop();
    }
    m_speech->say(text);
#else
    QString cmd = QString("powershell -Command \"Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('%1');\"").arg(text);
    QProcess::startDetached(cmd);
#endif
}

void TtsEngine::stop() {
#ifdef HAVE_QT_TTS
    m_speech->stop();
#endif
}

#pragma once
#include <QObject>
#ifdef HAVE_QT_TTS
#include <QTextToSpeech>
#endif
#include <QProcess>

class TtsEngine : public QObject {
    Q_OBJECT

public:
    static TtsEngine& instance();
    void speak(const QString& text);
    void stop();

private:
    TtsEngine();
#ifdef HAVE_QT_TTS
    QTextToSpeech *m_speech;
#endif
};

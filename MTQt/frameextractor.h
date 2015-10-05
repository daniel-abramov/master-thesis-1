#pragma once

#include "glwidget.h"
#include "libav.h"

#include <QString>

#include <queue>

class QTimerEvent;

class FrameExtractor : public QObject {
    Q_OBJECT

    struct FrameCallbackHandler {
        bool stopped;
        std::queue<const libav::AVFrame*> videoFrames;

        bool operator()(const libav::AVFrame& videoFrame, int /*index*/) {
            videoFrames.push(&videoFrame);
            return true;
        }

        bool operator()(const libav::AVSamples& /*samples*/ , int /*index*/) { return true; } // ignore audio samples
    };

public:
    FrameExtractor(GLWidget& frameReceiver, const char* filename)
        : m_frameReceiver(frameReceiver)
        , m_inputFile(filename)
        , m_fileStream(m_inputFile)
        , m_timerId(startTimer(1000 / 30))
    {
    }

private:
    virtual void timerEvent(QTimerEvent* timerEvent) {
        if (timerEvent->timerId() == m_timerId) {
            if (ProceedDecoding()) {
                m_frameReceiver.FeedFrame(m_callback.videoFrames.front());
                m_callback.videoFrames.pop();
            }
        }
    }

    bool ProceedDecoding() {
        bool decoded = true;
        while (m_callback.videoFrames.empty() && decoded)
            decoded = m_fileStream.Decode(m_callback, true);

        if (! decoded || m_callback.stopped) {
            killTimer(m_timerId);
            m_timerId = 0;
        }
        return decoded;
    }

private:
    GLWidget&             m_frameReceiver;
    libav::AVInputFile    m_inputFile;
    libav::AVStream       m_fileStream;
    FrameCallbackHandler  m_callback;
    int                   m_timerId;
};

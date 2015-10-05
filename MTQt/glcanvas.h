#pragma once

#include "libav.h"

#include <QGLWidget>
#include <QImage>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

#include <QDebug>

#include <memory>

class QGLCanvas : public QGLWidget
{
public:
    QGLCanvas(QWidget* parent = nullptr) : QGLWidget(parent), m_frame(nullptr)
    { }

    void FeedFrame(const libav::AVFrame* frame)
    {
        m_frame = frame;
        update();
    }

private:
    virtual void paintEvent(QPaintEvent* paintEvent) override
    {
        if (! m_frame)
            return;

        // ::PixelFormat pixelFormat = AV_PIX_FMT_RGB32;
        ::PixelFormat pixelFormat = PIX_FMT_RGB24;
        libav::AVImageFormat imageFormat(m_frame->GetWidth(), m_frame->GetHeight(), pixelFormat);

        m_tempFrame.reset(new libav::AVTempFrame(imageFormat, *m_frame));
        m_image = QImage(m_tempFrame->GetPlane(0), m_tempFrame->GetWidth(), m_tempFrame->GetHeight(),
                         QImage::Format_RGB888);

        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.drawImage(rect(), m_image);
    }

private:
    const libav::AVFrame* m_frame;
    std::unique_ptr<libav::AVTempFrame> m_tempFrame;
    QImage m_image;
};

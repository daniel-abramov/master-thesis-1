#pragma once

#include "libav.h"

#include <QGLWidget>
#include <QKeyEvent>


class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(bool vectorscope, QWidget* parent = nullptr);

    virtual QSize sizeHint() const override { return QSize(255, 255); }
    virtual QSize minimumSizeHint() const override { return sizeHint(); }

    void FeedFrame(const libav::AVFrame* frame);

private:
    enum DrawMode {
        MODE_DOTS,
        MODE_LINES,
        MODE_LINES_INCREASING_BRIGHTNESS,
        MODE_LINES_ACCUMULATE,
        MODE_LINES_ALPHA
    };

    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    DrawMode m_mode;
    const libav::AVFrame* m_frame;
    bool m_vectorscope;
};

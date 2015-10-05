#pragma once

#include "libav.h"

#include <QWidget>

#include <memory>

class FrameExtractor;
class QGLCanvas;
class GLWidget;
class QKeyEvent;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

    void FeedFrame(const libav::AVFrame* frame);

private:
    void keyPressEvent(QKeyEvent* keyEvent);

private:
    GLWidget* m_glWidget;
    QGLCanvas* m_glCanvas;
    std::unique_ptr<FrameExtractor> m_frameExtractor;
};

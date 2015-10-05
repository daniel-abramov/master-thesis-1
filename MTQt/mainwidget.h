#pragma once

#include "frameextractor.h"

#include <QWidget>

#include <memory>


class GLWidget;
class QKeyEvent;


class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private:
    void keyPressEvent(QKeyEvent* keyEvent);

private:
    GLWidget* m_glWidget;
    std::unique_ptr<FrameExtractor> m_frameExtractor;
};


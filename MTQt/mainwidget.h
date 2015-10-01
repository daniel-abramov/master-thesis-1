#pragma once

#include "frameextractor.h"

#include <QWidget>

class GLWidget;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private:
    GLWidget* m_glWidget;
    FrameExtractor m_frameExtractor;
};


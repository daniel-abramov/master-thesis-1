#include "glwidget.h"
#include "mainwidget.h"

#include <QVBoxLayout>

#include <algorithm>
#include <fstream>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
{
    GLWidget* glWidget = new GLWidget(this);
    glWidget->setFocusPolicy(Qt::StrongFocus);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(glWidget);
    setLayout(layout);
    resize(640, 480);
}

MainWidget::~MainWidget()
{
}

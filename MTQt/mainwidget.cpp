#include "glwidget.h"
#include "mainwidget.h"

#include <QFileDialog>
#include <QVBoxLayout>

#include <algorithm>
#include <fstream>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , m_glWidget(new GLWidget(this))
    , m_frameExtractor(new FrameExtractor(*m_glWidget, "/Users/daniel/Movies/20150909_111119.mp4"))
{
    m_glWidget->setFocusPolicy(Qt::StrongFocus);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(m_glWidget);

    setLayout(layout);
    resize(640, 480);
}

MainWidget::~MainWidget()
{
}

void MainWidget::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == Qt::Key_O) {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video File"), "/Users/daniel",
                                                        tr("Videos (*.mpg *.mp4 *.mkv *.m4v *.flv *.avi *.mov)"));
        if (! fileName.isEmpty())
            m_frameExtractor.reset(new FrameExtractor(*m_glWidget, fileName.toStdString().c_str()));
    }
    return QWidget::keyPressEvent(keyEvent);
}

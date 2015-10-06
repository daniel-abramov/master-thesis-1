#include "mainwidget.h"
#include "frameextractor.h"
#include "glcanvas.h"
#include "glwidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

#include <algorithm>
#include <fstream>

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , m_waveform(new GLWidget(false, this))
    , m_vectorscope(new GLWidget(true, this))
    , m_canvas(new QGLCanvas(this))
    , m_frameExtractor(new FrameExtractor(*this, "/Users/daniel/Movies/20150909_111119.mp4"))
{
    m_waveform->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_vectorscope->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_waveform->setFocusPolicy(Qt::ClickFocus);
    m_vectorscope->setFocusPolicy(Qt::ClickFocus);
    m_canvas->setFocusPolicy(Qt::ClickFocus);

    /*
    QHBoxLayout* horizontalVectorscope = QHBoxLayout();
    horizontalVectorscope->addWidget(m_vectorscope);
    horizontalVectorscope->setAlignment(
    */

    QVBoxLayout* vertical = new QVBoxLayout();
    vertical->addWidget(m_waveform);
    vertical->addWidget(m_vectorscope);
    vertical->setAlignment(Qt::AlignCenter);

    QHBoxLayout* horizontal = new QHBoxLayout();
    horizontal->addLayout(vertical);
    horizontal->addWidget(m_canvas);

    setLayout(horizontal);
    resize(1280, 720);
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
            m_frameExtractor.reset(new FrameExtractor(*this, fileName.toStdString().c_str()));
    }
    return QWidget::keyPressEvent(keyEvent);
}

void MainWidget::FeedFrame(const libav::AVFrame* frame)
{
    m_canvas->FeedFrame(frame);
    m_waveform->FeedFrame(frame);
    m_vectorscope->FeedFrame(frame);
}

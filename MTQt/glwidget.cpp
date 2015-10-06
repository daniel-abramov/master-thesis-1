#include "glwidget.h"

#include <OpenGL/glu.h>

#include <QDebug>
#include <QImage>

#include <algorithm>

static inline uint8_t GetY(int x, int y, const libav::AVFrame *f)
{
    return f->GetPlane(0)[ (y * f->GetLineSize(0)) + x ];
}

GLWidget::GLWidget(bool vectorscope, QWidget* parent)
    : QGLWidget(parent)
    , m_mode(MODE_DOTS)
    , m_frame(nullptr)
    , m_vectorscope(vectorscope)
{
}

void GLWidget::FeedFrame(const libav::AVFrame* frame)
{
    m_frame = frame;
    update();
}

void GLWidget::initializeGL()
{
    qglClearColor(Qt::black);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0,0.0,0.0,0.0);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (m_frame == nullptr) {
        return;
    }

    static const double kLeft = -1.0;
    static const double kRight = 1.0;
    static const double kBottom = -1.0;
    static const double kTop = 1.0;

    const double xScale = (kRight - kLeft) / (m_vectorscope ? 255.0 : m_frame->GetWidth());
    const double yScale = (kTop - kBottom) / 255.0;

    const double colorStep = (m_mode == MODE_LINES_INCREASING_BRIGHTNESS)
        ? 1.0 / m_frame->GetHeight()
        : 1.0 / 5.0; // levels
    double greenValue = 0.0;
    double z = -0.2;

    if (m_vectorscope) {
        const uint8_t* u = m_frame->GetPlane(1);
        const uint8_t* v = m_frame->GetPlane(2);
        for(size_t t = 0; t < m_frame->GetLineSize(1); ++t) {
            glBegin(GL_POINTS);
            double plotX = kLeft + u[t] * xScale;
            double plotY = kBottom + v[t] * yScale;
            glColor3f(0.0, 1.0, 0.0);
            glVertex3f(plotX, plotY, 0.0);
            glEnd();
        }
        return;
    }

    for (size_t y = 0; y < m_frame->GetHeight(); ++y, greenValue += colorStep, z += 0.01) {
        glBegin( m_mode == MODE_DOTS ? GL_POINTS : GL_LINES );
        glVertex3f(kLeft, 0.0, 0.0);
        for (size_t x = 0; x < m_frame->GetWidth(); ++x) {
            double plotX = kLeft + x * xScale;
            double plotY = kBottom + GetY(x, y, m_frame) * yScale;

            switch (m_mode) {
                case MODE_DOTS:
                case MODE_LINES:
                    glColor3f(0.0, 1.0, 0.0);
                    break;
                case MODE_LINES_INCREASING_BRIGHTNESS:
                    glColor3f(0.0, greenValue, 0.0);
                    break;
                case MODE_LINES_ALPHA:
                    glColor4f(0.0, 1.0, 0.0, z);
                    break;
                default:
                    Q_ASSERT(0);
            }

            glVertex3f(plotX, plotY, m_mode == MODE_LINES_ALPHA ? z : 0.0);
        }
        glEnd();
    }
}

void GLWidget::resizeGL(int width, int height)
{
    if (width == 0 || height == 0)
        return;
    glViewport(0, 0, width, height);
}

void GLWidget::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == Qt::Key_0) {
        m_mode = MODE_DOTS;
    } else if (keyEvent->key() == Qt::Key_1) {
        m_mode = MODE_LINES;
    } else if (keyEvent->key() == Qt::Key_2) {
        m_mode = MODE_LINES_INCREASING_BRIGHTNESS;
    } else if (keyEvent->key() == Qt::Key_3) {
        m_mode = MODE_LINES_ALPHA;
    } else if (keyEvent->key() == Qt::Key_W) {
        m_vectorscope = false;
    } else if (keyEvent->key() == Qt::Key_V) {
        m_vectorscope = true;
    }

    QGLWidget::keyPressEvent(keyEvent);
    update();
}

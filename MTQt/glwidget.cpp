#include "glwidget.h"

#include <OpenGL/glu.h>

#include <QDebug>
#include <QImage>

#include <algorithm>

GLWidget::GLWidget(const QString& filename, QWidget* parent)
    : QGLWidget(parent)
    , m_imageFilename(filename)
{
    if (m_imageFilename.isEmpty())
        return;

    QImage image(filename);

    std::vector<std::vector<double>> luminance(image.width());
    for (int x = 0; x < image.width(); ++x) {
        luminance[x].resize(image.height());
        for (int y = 0; y < image.height(); ++y) {
            QRgb pixel = image.pixel(x, y);
            uint32_t r = ((pixel & 0x00FF0000) >> 4*4);
            uint32_t g = ((pixel & 0x0000FF00) >> 2*4);
            uint32_t b = ((pixel & 0x000000FF));

            double yuv_y = (0.299*r + 0.587*g + 0.114*b);
            if (yuv_y > 255.0 || yuv_y < 0.0)
                qDebug() << "WTF?!";

            luminance[x][y] = yuv_y;
        }
        // std::sort(luminance[x].begin(), luminance[x].end());
    }

    m_luminance.swap(luminance);
}

void GLWidget::initializeGL()
{
    qglClearColor(Qt::black);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.0, 1.0f, 0.0);

    static const double kLeft = -1.0;
    static const double kRight = 1.0;
    static const double kBottom = -1.0;
    static const double kTop = 1.0;

    const double xScale = (kRight - kLeft) / m_luminance.size();
    const double yScale = (kTop - kBottom) / 255.0;

    glBegin( GL_LINES /*GL_POINTS*/ );

    for (size_t y = 0; y < m_luminance[0].size(); ++y) {
        for (size_t x = 0; x < m_luminance.size(); ++x) {
            glVertex3f(kLeft + x * xScale, kBottom + m_luminance[x][y] * yScale, 0.0);
        }
    }

    glEnd();

    /*
    glBegin(GL_QUADS);
    glColor3f(0.0, 1.0f, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.0, 0.0, 1.0f);
    glVertex3f(0.0, -1.0, 0.0f);
    glVertex3f(2.0, -1.0, 0.0f);
    glVertex3f(1.5, 0.0, 0.0f);
    glEnd();

    glBegin(GL_POINTS);
    //glPointSize(3.0f);
    glColor3f(1.0f, 0.0f, 0.0);
    glVertex3f(0.0f, 0.0f, 0.0);
    glVertex3f(0.1f, 0.0f, 0.0);
    glVertex3f(0.9f, 0.0f, 0.0);
    glVertex3f(0.0f, 0.9f, 0.0);
    glVertex3f(0.5f, 0.5f, 0.0);

    //glColor3f(0.0, 0.0, 1.0f);
    for (int i = 0; i < 5; ++i) {
        glVertex3f(1.0f + 0.1*i, 0.0f, 0.0);
    }

    //glVertex3f(1.0f, 1.0f, 0.0);
    //glVertex3f(2.0f, 2.0f, 0.0);
    glEnd();
    */
}

void GLWidget::resizeGL(int width, int height)
{
    qDebug() << "resizeGL" << width << height;

    if (width == 0 || height == 0)
        return;

    glViewport(0, 0, width, height); // ???

    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();

    // /GLfloat m[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };
    // glLoadMatrixf(m);

    //gluLookAt(0, 0, -1, 0, 0, 0, 0, 1, 0);
    //glTranslatef(1.0, 0.0, 0.0);
    //glMatrixMode(GL_PERSPECTIVE);
    //gluPerspective(120.0, width/height, 0.1, 10.0);
    //gluLookAt(1, 1, -1.0, 0, 0, 0, 0, 1, 0);
    //glFrustum(0, 1, 0, 1, 0.1, 2);
    //glOrtho(0, 1, 0, 1, 0.1, 1.5);
}

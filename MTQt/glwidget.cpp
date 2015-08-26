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
    }

    m_luminance.swap(luminance);
}

void GLWidget::initializeGL()
{
    qglClearColor(Qt::black);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glClearColor(0.0,0.0,0.0,0.0);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    static const double kLeft = -1.0;
    static const double kRight = 1.0;
    static const double kBottom = -1.0;
    static const double kTop = 1.0;

    const double xScale = (kRight - kLeft) / m_luminance.size();
    const double yScale = (kTop - kBottom) / 255.0;

    const double colorStep = 1.0 / 5; // m_luminance[0].size();

    std::vector<std::map<double, double>> colors(m_luminance.size());

    double greenValue = 0.0;

    double z = 0.0;
    for (size_t y = 0; y < m_luminance[0].size(); ++y, greenValue += colorStep, z += 0.001) {
        glBegin(GL_LINES);
        // glColor3f(0.0, greenValue, 0.0);
        glVertex3f(kLeft, 0.0, 0.0);
        for (size_t x = 0; x < m_luminance.size(); ++x) {
            double plotX = kLeft + x * xScale;
            double plotY = kBottom + m_luminance[x][y] * yScale;
            // colors[x][plotY] += colorStep;
            // glColor3f(0.0, colors[x][plotY], 0.0);
            glColor4f(0.0, 1.0, z, 0.5);
            glVertex3f(plotX, plotY, 0.0);
        }
        glEnd();
    }
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

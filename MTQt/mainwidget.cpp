#include "mainwidget.h"
#include "glwidget.h"

#include <QVBoxLayout>
#include <QPaintEvent>
#include <QResizeEvent>

#include <QPainter>
#include <QImage>
#include <QMessageBox>
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QIODevice>

#include <fstream>

MainWidget::MainWidget(const QString& filename, QWidget *parent)
    : QWidget(parent)
{
    GLWidget* glWidget = new GLWidget(filename, this);
    glWidget->setFocusPolicy(Qt::StrongFocus);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(glWidget);
    setLayout(layout);
    resize(640, 480);


    /*
    v.resize(image.width());
    for (int x = 0; x < image.width(); ++x) {
        v[x].resize(255);
        for (int y = 0; y < image.height(); ++y) {
            QRgb pixel = image.pixel(x, y);
            uint32_t r = ((pixel & 0x00FF0000) >> 4*4);
            uint32_t g = ((pixel & 0x0000FF00) >> 2*4);
            uint32_t b = ((pixel & 0x000000FF));
            // out << QString("r=%1, g=%2, b=%3\n").arg(r).arg(g).arg(b);
            unsigned yuv_y = (0.299*r + 0.587*g + 0.114*b);

            if (yuv_y != 0)
                --yuv_y;
            if (yuv_y >= 255)
                yuv_y = 254;

            v[x][yuv_y] = true;
        }
    }
    */
}

MainWidget::~MainWidget()
{

}

void MainWidget::customEvent(QEvent* event)
{
    Q_UNUSED(event);
}

void MainWidget::paintEvent(QPaintEvent* event)
{
    /*
    QPainter p(this);

    double xfactor = width() / (double)v.size();
    double yfactor = height() / 255.0;

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this, "", "SHIT");
        return;
    }

    QTextStream out(&f);

    for (auto i = 0; i < v.size(); ++i) {
        for (auto j = 0; j < v[i].size(); ++i) {
            if (v[i][j]) {
                int x = i*xfactor;
                int y = j*yfactor;
                if (x > width() || y > height() || x < 0 || y < 0)
                    continue;
                out << QString("%1, %2\n").arg(x).arg(y);
                p.drawPoint(x, y);
            }
        }
    }

    f.close();
    */
}

void MainWidget::resizeEvent(QResizeEvent* event)
{
    return QWidget::resizeEvent(event);
    //update();
}

#pragma once

#include <QGLWidget>
#include <QKeyEvent>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(const QString& filename, QWidget* parent = nullptr);
    virtual QSize minimumSizeHint() const override { return QSize(320, 240); }

private:
    enum DrawMode {
        MODE_DOTS,
        MODE_LINES,
        MODE_LINES_INCREASING_BRIGHTNESS,
        MODE_LINES_ACCUMULATE,
        MODE_LINES_ALPHA
    };

    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;

    QString m_imageFilename;
    std::vector<std::vector<double>> m_luminance;
    DrawMode m_mode;
};

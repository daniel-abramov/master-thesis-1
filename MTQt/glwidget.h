#pragma once

#include <QGLWidget>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(const QString& filename, QWidget* parent = nullptr);
    virtual QSize minimumSizeHint() const override { return QSize(320, 240); }

private:
    virtual void initializeGL() override;
    virtual void paintGL() override;
    virtual void resizeGL(int width, int height) override;

    QString m_imageFilename;
    std::vector<std::vector<double>> m_luminance;
};

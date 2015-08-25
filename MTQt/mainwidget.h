#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>

#include <vector>

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(const QString& filename = QString(), QWidget *parent = 0);
    ~MainWidget();

private:
    virtual void customEvent(QEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
};

#endif // MAINWIDGET_H

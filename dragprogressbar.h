#ifndef DRAGPROGRESSBAR_H
#define DRAGPROGRESSBAR_H

#include <QProgressBar>
#include <QMouseEvent>
#include <QWidget>
class DragProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    DragProgressBar(QWidget* parent=nullptr);
protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
private:
    bool isPressed;
    QPoint lastPoint;
    int xOffset;
};

#endif // DRAGPROGRESSBAR_H

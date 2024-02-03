#include "dragprogressbar.h"

DragProgressBar::DragProgressBar(QWidget* parent):QProgressBar(parent)
{

}

void DragProgressBar::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPoint widgetPoint = e->pos();
    if(this->rect().contains(widgetPoint))
    {
        int x = widgetPoint.x();
        this->setValue(this->value()+this->maximum()*x/this->width());
    }
    return QProgressBar::mouseDoubleClickEvent(e);
}

void DragProgressBar::mousePressEvent(QMouseEvent* e)
{
    QPoint progressPoint = this->mapFromGlobal(e->globalPosition().toPoint());
    if(this->rect().contains(progressPoint))
    {
        if(e->button()==Qt::LeftButton)
        {
            isPressed = true;
            lastPoint = progressPoint;
            qDebug()<<"初始点击位置:"<<progressPoint.x();
        }
    }
    return QProgressBar::mousePressEvent(e);
};

void DragProgressBar::mouseMoveEvent(QMouseEvent* e)
{
    QPoint movePoint = e->pos()-lastPoint;
    if(isPressed && e->buttons() == Qt::LeftButton)
    {
        QPoint progressPoint = this->mapFromGlobal(e->globalPosition().toPoint());
        if(this->rect().contains(progressPoint))
        {
            int x1 = progressPoint.x();
            int x2 = lastPoint.x();
            xOffset = (-x2+x1)*this->maximum()/this->width();
            this->setValue(this->value()+xOffset);
            lastPoint = progressPoint;
            qDebug()<<"差值:"<<-x2+x1;
            qDebug()<<"当前值:"<<this->value();
        }
    }
    return QProgressBar::mouseMoveEvent(e);
};

void DragProgressBar::mouseReleaseEvent(QMouseEvent* e)
{
    isPressed = false;
    return QProgressBar::mouseReleaseEvent(e);
};

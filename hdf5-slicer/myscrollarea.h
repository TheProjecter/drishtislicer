#ifndef MYSCROLLAREA_H
#define MYSCROLLAREA_H
#include <QtGui>

class MyScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    MyScrollArea(QWidget* parent=0);
    ~MyScrollArea();

    int hscroll();
    int vscroll();
    int width();
    int height();

 signals :
    void updateImage();

protected:
    void resizeEvent(QResizeEvent *);
    void scrollContentsBy(int dx, int dy);
};

#endif

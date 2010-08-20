#include "myscrollarea.h"

MyScrollArea::MyScrollArea(QWidget *parent) : QScrollArea(parent) { }

MyScrollArea::~MyScrollArea() { }

void
MyScrollArea::resizeEvent(QResizeEvent *event)
{
  QScrollArea::resizeEvent(event);
  emit updateImage();
}

void
MyScrollArea::scrollContentsBy(int dx, int dy)
{
  QScrollArea::scrollContentsBy(dx, dy);
  emit updateImage();
}

int MyScrollArea::hscroll() { return horizontalScrollBar()->value(); }
int MyScrollArea::vscroll() { return verticalScrollBar()->value(); }
int MyScrollArea::width() { return size().width(); }
int MyScrollArea::height() { return size().height(); }

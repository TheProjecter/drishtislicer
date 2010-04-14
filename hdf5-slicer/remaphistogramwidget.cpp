#include "remaphistogramwidget.h"
#include <math.h>

RemapHistogramWidget::RemapHistogramWidget(QWidget *parent) :
  QWidget(parent)
{
  m_parent = parent;

  setFocusPolicy(Qt::StrongFocus);

  m_activeTick = -1;
  m_width = 100;
  m_shift = 0;
  m_scale = 1;
  m_base = 50;
  m_scaleHistogram = 1;

  m_histMax = 0;
  m_rawMin = m_rawMax = 0;
  m_histogram.clear();
  m_histogramScaled.clear();

  m_rawMap.clear();
  m_pvlMap.clear();

  m_moveLine = false;

  m_Line = new RemapHistogramLine();
  m_Line->setLine(m_base, m_base+m_width);

  connect(m_Line, SIGNAL(addTick(int)),
	  this, SLOT(addTick(int)));
  connect(m_Line, SIGNAL(removeTick()),
	  this, SLOT(removeTick()));
  connect(m_Line, SIGNAL(updateScene(int)),
	  this, SLOT(updateScene(int)));
}

void RemapHistogramWidget::enterEvent(QEvent* event) { setFocus(); }
void RemapHistogramWidget::leaveEvent(QEvent* event) { clearFocus(); }

QList<float> RemapHistogramWidget::rawMap() {return m_rawMap;}
QList<uchar> RemapHistogramWidget::pvlMap() {return m_pvlMap;}

void
RemapHistogramWidget::setGradientStops(QGradientStops stops)
{  
  m_gradientStops = stops;
  for(uint i=0; i<m_gradientStops.size(); i++)
    {
      qreal f = m_gradientStops[i].first;
      QColor c = m_gradientStops[i].second;
      c.setAlpha(255);
      m_gradientStops[i] = QGradientStop(f, c);
    }
 
  update();
}

void
RemapHistogramWidget::setRawMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
}

void
RemapHistogramWidget::setHistogram(QList<uint> hist)
{
  m_histogram = hist;
  
  m_histMax = 0;
  for(uint i=0; i<m_histogram.size(); i++)
    m_histMax = qMax(m_histMax, m_histogram[i]);

  rescaleHistogram();

  defineMapping();
  update();
}

void
RemapHistogramWidget::updateHistogram(QList<uint> hist)
{
  m_histogram = hist;
  
  m_histMax = 0;
  for(uint i=0; i<m_histogram.size(); i++)
    m_histMax = qMax(m_histMax, m_histogram[i]);

  rescaleHistogram();
  update();
}

void
RemapHistogramWidget::rescaleHistogram()
{
  m_histogramScaled.clear();
  if (m_histMax > 0)
    {
      for(uint i=0; i<m_histogram.size(); i++)
	{
	  float hht = (float)m_histogram[i]/(float)m_histMax;
	  for(uint sh=0; sh<m_scaleHistogram; sh++)
	    hht = sqrt(hht);
	  
	  hht *= 200;
	  m_histogramScaled.append(hht);
	}
    }
  else
    for(uint i=0; i<m_histogram.size(); i++)
      m_histogramScaled.append(m_histogram[i]);
}

void
RemapHistogramWidget::addTick(int tkn)
{
  m_activeTick = tkn;
  defineMapping();
  update();
}
void
RemapHistogramWidget::removeTick()
{
  m_activeTick = -1;
  defineMapping();
  update();
}
void
RemapHistogramWidget::updateScene(int tkn)
{
  m_activeTick = tkn;

  if (tkn == -1)
    defineMapping();

  update();
}


QList<QPoint>
RemapHistogramWidget::tickMarkers()
{
  QList<QPoint> markerPos;
  
  QList<float> tk = m_Line->ticks();
  for(uint i=0; i<tk.size(); i++)
    {
      int xp = m_lineOrigin + tk[i]*m_lineWidth;
      int yp = m_lineHeight;
      markerPos.append(QPoint(xp, yp));
    }

  return markerPos;
}

QList<QPoint>
RemapHistogramWidget::tickMarkersOriginal()
{
  QList<QPoint> markerPos;
  
  QList<float> tk = m_Line->ticksOriginal();
  for(uint i=0; i<tk.size(); i++)
    {
      int xp = m_lineOrigin + tk[i]*m_lineWidth;
      int yp = m_lineHeight + 10; // shift marker slightly below the line
      markerPos.append(QPoint(xp, yp));
    }

  return markerPos;
}

void
RemapHistogramWidget::drawTickMarkers(QPainter *p,
				 QList<QPoint> tk,
				 QColor penColor,
				 QColor fillColor1,
				 QColor fillColor2)
{
  p->setPen(QPen(penColor));
  for(uint i=0; i<tk.size(); i++)
    {
      int xp = tk[i].x();
      int yp = tk[i].y();

      QRadialGradient rg(xp, yp, 7);
      if (i != m_activeTick)
	{
	  rg.setColorAt(0, Qt::white);
	  rg.setColorAt(1, fillColor1);
	}
      else
	{
	  rg.setColorAt(0, fillColor2);
	  rg.setColorAt(1, Qt::darkGreen);
	}

      p->setBrush(QBrush(rg));

      if (i != m_activeTick)
	p->drawEllipse(xp-5, yp-5, 11, 11);
      else
	p->drawEllipse(xp-7, yp-7, 15, 15);
    }
}

void
RemapHistogramWidget::drawConnectionLines(QPainter *p,
				     QList<QPoint> e1,
				     QList<QPoint> e2)
{
  p->setPen(QPen(Qt::darkGray));

  p->setBrush(QColor(150, 150, 150, 150));
  p->drawRect(e1[0].x(),
	      m_lineHeight-210,
	      e2[0].x()-e1[0].x(),
	      205);
  int ie = e1.size()-1;
  p->drawRect(e2[ie].x(),
	      m_lineHeight-210,
	      e1[ie].x()-e2[ie].x(),
	      205);

  p->setPen(QPen(Qt::darkGray));

  for(uint i=0; i<e1.size(); i++)
    {
      QPoint a = e1[i];
      QPoint b = e2[i];

      int x, y, wd, ht;
      if (a.x() < b.x())
	{
	  x = a.x(); wd = b.x()-a.x();
	}
      else
	{
	  x = b.x(); wd = a.x()-b.x();
	}
      if (a.y() < b.y())
	{
	  y = a.y(); ht = b.y()-a.y();
	}
      else
	{
	  y = b.y(); ht = a.y()-b.y();
	}

      QPolygon poly1;
      poly1 << QPoint(b.x(), y);
      poly1 << QPoint(b.x()-3, y+ht+5);
      poly1 << QPoint(b.x()+3, y+ht+5);
      p->drawConvexPolygon(poly1);

      QPolygon poly2;
      poly2 << QPoint(a.x(), y+ht+5);
      poly2 << QPoint(a.x()-3, y+5);
      poly2 << QPoint(a.x()+3, y+5);
      p->drawConvexPolygon(poly2);

      p->drawArc(x, y, wd, ht+20,
		 0, -180*16);
    }
}

void
RemapHistogramWidget::drawRawValues(QPainter *p)
{
  QFont pfont = QFont("Helvetica", 10);
  QPen linepen(Qt::darkGray);

  p->setBrush(QColor(0,0,0,100));
  p->setFont(pfont);

  QList<float> tk = m_Line->ticks();
  QList<float> otk = m_Line->ticksOriginal();
  for(uint i=0; i<tk.size(); i++)
    {
      float v;
      uchar mv;
      if (i>0 && i<tk.size()-1)
	  v = m_rawMin + otk[i]*(m_rawMax-m_rawMin);
      else
	v = m_rawMin + tk[i]*(m_rawMax-m_rawMin);
      
      mv = m_pvlMap[i];

      int xp = m_lineOrigin + tk[i]*m_lineWidth;
      int yp = m_lineHeight-200;

      //-------------
      QPainterPath pp;
      pp.addText(QPointF(0,0), 
		 pfont,
		 QString("%1").arg(v));
      QRectF br = pp.boundingRect();
      float by = br.height();      
      p->translate(xp-by/2, yp-7);
      p->rotate(90);
      br.adjust(-3,-5,3,4);
      p->setPen(Qt::darkRed);
      p->drawRect(br);
      p->setPen(Qt::white);
      p->drawText(QPointF(0,0), 
		  QString("%1").arg(v));      
      p->resetTransform();
      //-------------

      //-------------
      int myp = m_lineHeight+30;
      QPainterPath mpp;
      mpp.addText(QPointF(0,0), 
		  pfont,
		  QString("%1").arg(mv));
      QRectF mbr = mpp.boundingRect();
      float mby = mbr.height();      
      p->translate(xp-mby/2, myp-7);
      p->rotate(90);
      mbr.adjust(-3,-5,3,4);
      p->setPen(Qt::darkRed);
      p->drawRect(mbr);
      p->setPen(Qt::white);
      p->drawText(QPointF(0,0), 
		  QString("%1").arg(mv));      
      p->resetTransform();
      //-------------


      yp = yp+br.width()-10;
      p->setPen(linepen);
      p->drawLine(xp, yp, xp, m_lineHeight-5);
    }
}

void
RemapHistogramWidget::drawMainLine(QPainter *p)
{
  p->setPen(Qt::black);
  p->drawLine(m_lineOrigin,
	      m_lineHeight-200,
	      m_lineOrigin,
	      m_lineHeight);
  p->drawLine(m_lineOrigin+m_lineWidth,
	      m_lineHeight-200,
	      m_lineOrigin+m_lineWidth,
	      m_lineHeight);

  QLinearGradient lg(0, m_lineHeight-5,
		     0, m_lineHeight+5);
  lg.setColorAt(0, Qt::black);
  lg.setColorAt(0.1, Qt::darkGray);
  lg.setColorAt(0.5, Qt::white);
  lg.setColorAt(1, Qt::black);
  QPen pen(QBrush(lg), 10, Qt::SolidLine, Qt::RoundCap);
  p->setPen(pen);
  p->drawLine(m_lineOrigin,
	      m_lineHeight,
	      m_lineOrigin+m_lineWidth,
	      m_lineHeight);
}

void
RemapHistogramWidget::paintEvent(QPaintEvent *event)
{
  QPainter p(this);

  p.setRenderHints(QPainter::Antialiasing |
		   QPainter::TextAntialiasing |
		   QPainter::HighQualityAntialiasing |
		   QPainter::SmoothPixmapTransform);

  QList<QPoint> rawMarkers = tickMarkers();
  QList<QPoint> rawMarkersOriginal = tickMarkersOriginal();
  
  drawMainLine(&p);

  drawConnectionLines(&p,
		      rawMarkersOriginal,
		      rawMarkers);

  drawTickMarkers(&p,
		  rawMarkers,
		  Qt::black,
		  Qt::darkRed,
		  Qt::green);


  drawColorMap(&p);

  drawHistogram(&p);

  drawRawValues(&p);

  if (hasFocus())
    {
      p.setPen(QPen(QColor(250, 100, 0), 2));
      p.setBrush(Qt::transparent);
      p.drawRect(rect());
    }
}

void
RemapHistogramWidget::drawColorMap(QPainter *p)
{
//  if (m_histogramScaled.size() == 0)
//    return;

  QList<float> tk = m_Line->ticks();

  QLinearGradient lg(m_lineOrigin+m_lineWidth*tk[0],
		     0,
		     m_lineOrigin+m_lineWidth*tk[tk.size()-1],
		     0);
  lg.setStops(m_gradientStops);
  QPen pen(QBrush(lg), 10); 
  p->setPen(pen);
  p->drawLine(m_lineOrigin+m_lineWidth*tk[0]+5,
	      m_lineHeight-10,
	      m_lineOrigin+m_lineWidth*tk[tk.size()-1]-5,
	      m_lineHeight-10);  
}

void
RemapHistogramWidget::drawHistogram(QPainter *p)
{
  if (m_histogramScaled.size() == 0)
    {
      emit getHistogram();
      return;
    }

  QList<float> tk = m_Line->ticks();
  QList<float> otk = m_Line->ticksOriginal();

  int hsize = m_histogramScaled.size()-1;

  {
    QPolygon poly;
    //for first part
    float kstart = otk[0];
    float kend = tk[0];
    int istart = kstart*hsize;
    int iend = kend*hsize;

    for(uint i=istart; i<=iend; i++)
      {
	float frc =(float)(i-istart)/(float)(iend-istart);
	frc = kend*frc;

	int x, y;
	x = m_lineOrigin + m_lineWidth*frc;
	y = m_lineHeight - 5 - m_histogramScaled[i];
	poly << QPoint(x,y);
      }
    p->setPen(Qt::darkRed);
    p->drawPolyline(poly);
  }

  {
    QPolygon poly;
    //for last part
    int otke = otk.size()-1;
    float kend = otk[otke];
    float kstart = tk[otke];
    float kw = kend-kstart;
    int istart = kstart*hsize;
    int iend = kend*hsize;

    for(uint i=istart; i<=iend; i++)
      {
	float frc =(float)(i-istart)/(float)(iend-istart);
	frc = kstart + kw*frc;

	int x, y;
	x = m_lineOrigin + m_lineWidth*frc;
	y = m_lineHeight - 5 - m_histogramScaled[i];	
	poly << QPoint(x,y);
      }

    p->setPen(Qt::darkRed);
    p->drawPolyline(poly);
  }

  QPolygon poly;
  for(uint t=0; t<otk.size()-1; t++)
    {
      float otkstart = otk[t];
      float otkend = otk[t+1];

      float tkstart = tk[t];
      if (t == 0) otkstart = tkstart;
      float tkend = tk[t+1];
      if (t == otk.size()-2) otkend = tkend;
      float tkw = tkend-tkstart;

      int istart = otkstart*hsize;
      int iend = otkend*hsize;

      for(uint i=istart; i<=iend; i++)
	{
	  float frc =(float)(i-istart)/(float)(iend-istart);
	  frc = tkstart + tkw*frc;

	  int x, y;
	  x = m_lineOrigin + m_lineWidth*frc;
	  y = m_lineHeight - 5 - m_histogramScaled[i];
	  poly << QPoint(x,y);
	}
    }
  p->setPen(Qt::darkGray);
  p->drawPolyline(poly);
}

void
RemapHistogramWidget::defineMapping()
{
  QList<float> tk = m_Line->ticks();
  QList<float> otk = m_Line->ticksOriginal();

  float tklen = tk[tk.size()-1]-tk[0];

  m_rawMap.clear();
  m_pvlMap.clear();

  for(uint i=0; i<otk.size(); i++)
    {
      float rv, pv;
      if (i > 0 && i < otk.size()-1)
	{
	  rv = m_rawMin + otk[i]*(m_rawMax-m_rawMin);
	  pv = 255*(tk[i]-tk[0])/tklen;
	}
      else
	{
	  rv = m_rawMin + tk[i]*(m_rawMax-m_rawMin);
	  pv = 255*otk[i];
	}

      m_rawMap.append(rv);
      m_pvlMap.append(pv);
    }

  emit newMapping();
}

void
RemapHistogramWidget::resizeEvent(QResizeEvent *event)
{
  m_width = rect().width()-2*m_base;

  resizeLine();
}

void
RemapHistogramWidget::resizeLine()
{
  int rawStart = m_base + m_shift;
  int rawWidth = m_scale*m_width;
  m_Line->setLine(rawStart, rawWidth);

  QPoint rawline = m_Line->line();  
  m_lineOrigin = rawline.x();
  m_lineWidth = rawline.y();
  m_lineHeight = rect().height()-50;
}

void
RemapHistogramWidget::processCommands(QString cmd)
{
  QStringList list = cmd.split(" ", QString::SkipEmptyParts);

  if (list.size() == 2)
    {
      float rmin = list[0].toFloat();
      float rmax = list[1].toFloat();
      emit newMinMax(rmin, rmax);      
    }
}

void
RemapHistogramWidget::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Space)
    {
      bool ok;
      QString cmd;
      QString str;
      str = "Enter new Min and Max values\n";
      str += QString("Current values are %1 and %2").\
	      arg(m_rawMin).arg(m_rawMax);
      cmd = QInputDialog::getText(0,
				  "User Min Max Values",
				  str,
				  QLineEdit::Normal,
				  "",
				  &ok);
      if (ok && !cmd.isEmpty())
	processCommands(cmd);
    }
  else if (event->key() == Qt::Key_Up)
    {
	m_scaleHistogram++;	
	m_scaleHistogram = qBound(0, m_scaleHistogram, 5);	
	rescaleHistogram();
	update();
	return;
    }
  else if (event->key() == Qt::Key_Down)
    {
	m_scaleHistogram--;	
	m_scaleHistogram = qBound(0, m_scaleHistogram, 5);	
	rescaleHistogram();
	update();
	return;
    }

  return;
}

void
RemapHistogramWidget::mousePressEvent(QMouseEvent *event)
{
  QPoint pp = mapFromParent(event->pos());
  float ypos = pp.y();
  float xpos = pp.x();

  m_moveLine = false;

  m_button = event->button();

  if (fabs(ypos-m_lineHeight) <= 15)
    {
      m_Line->mousePress(xpos, event->button());
      update();
      return;
    }

  m_prevX = xpos;
  m_prevY = ypos;
  m_moveLine = true;
}

void
RemapHistogramWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!hasFocus())
    setFocus();

  QPoint pp = mapFromParent(event->pos());
  float ypos = pp.y();
  float xpos = pp.x();

  if (m_button == Qt::RightButton)
    {
      float d = (m_prevY-ypos)*0.3;
      m_scaleHistogram += d;
      m_scaleHistogram = qBound(0, m_scaleHistogram, 5);
      rescaleHistogram();
      update();
    }
  else if (m_button == Qt::MidButton)
    {						
      float d = (m_prevY-ypos)*0.01;
      if (fabs(d) < 1)
	{
	  if (d > 0)
	    m_scale/=(1-d);
	  else
	    {
	      m_scale*=(1+d);
	      while (m_base+m_shift+m_scale*m_width < m_width*0.9)
		m_shift +=100;
	    }

	  m_scale = qBound(1.0f, m_scale, 10.0f);      
	  
	  m_prevY = ypos;
	  m_prevX = xpos;

	  resizeLine();
	  update();
	}
    }
  else if (m_moveLine)
    {
      int move = xpos - m_prevX;
      if (move < 0)
	{
	  int pshift = m_shift;
	  m_shift += move;
	  if (m_base+m_shift+m_scale*m_width < m_width*0.9)
	    m_shift = pshift;
	}
      else
	{
	  m_shift += move;
	  if (m_base+m_shift > m_base)
	    m_shift = 0;
	}

      resizeLine();

      m_prevY = ypos;
      m_prevX = xpos;
      update();
    }
  else if (m_Line->grabsMouse() ||
      fabs(ypos-m_lineHeight) <= 15)
    {
      m_Line->mouseMove(xpos);

      m_prevY = ypos;
      m_prevX = xpos;
      update();
    }
  
}

void
RemapHistogramWidget::mouseReleaseEvent(QMouseEvent *event)
{
  m_moveLine = false;
  m_Line->mouseRelease();
}

void
RemapHistogramWidget::wheelEvent(QWheelEvent *event)
{
  float numSteps = event->delta()*0.01;
  if (numSteps < 0)
    {
      m_scale *= 1.0/(-numSteps); 
      m_scale = qMax(1.0f, m_scale);      

      m_shift -= ((numSteps+1)*m_lineWidth*0.25);

      if (m_shift > 0) m_shift = 0;
//      while (m_base+m_shift+m_scale*m_width < m_width*0.9)
//	m_shift +=100;
    }
  else
    {
      m_scale *= numSteps;
      m_shift -= ((numSteps-1)*m_lineWidth*0.25);
    }

  resizeLine();
  update();
}

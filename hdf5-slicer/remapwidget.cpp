#include "global.h"
#include "remapwidget.h"
#include <QFile>

RemapWidget::RemapWidget(QWidget *parent) :
  QWidget(parent)
{
  m_ok = false;
  ui.setupUi(this);

  QVBoxLayout *layout1 = new QVBoxLayout();
  layout1->setContentsMargins(0,0,0,0);
  ui.histogramFrame->setLayout(layout1);

  QVBoxLayout *layout2 = new QVBoxLayout();
  layout2->setContentsMargins(0,0,0,0);
  ui.imageFrame->setLayout(layout2);

  QVBoxLayout *layout3 = new QVBoxLayout();
  layout3->setContentsMargins(0,0,0,0);
  ui.colorFrame->setLayout(layout3);

  QVBoxLayout *layout4 = new QVBoxLayout();
  layout4->setContentsMargins(0,0,0,0);
  ui.sliderFrame->setLayout(layout4);

  m_scrollArea = new MyScrollArea;
  m_scrollArea->setBackgroundRole(QPalette::Dark);
  ui.imageFrame->layout()->addWidget(m_scrollArea);


  m_histogramWidget = 0;
  m_imageWidget = 0;
  m_gradientWidget = 0;
  m_remapVolume = 0;
  m_slider = 0;

  connect(m_scrollArea, SIGNAL(updateImage()),
	  this, SLOT(updateImage()));


  hideWidgets();
}

void
RemapWidget::hideWidgets()
{
  ui.checkBox->hide();
  ui.butX->hide();
  ui.butY->hide();
  ui.butZ->hide();
}

void
RemapWidget::showWidgets()
{
  ui.checkBox->show();
  ui.checkBox->setCheckState(Qt::Checked);
  ui.butX->show();
  ui.butY->show();
  ui.butZ->show();
  ui.butZ->setChecked(true);

  if (m_remapVolume->voxelType() == _Rgb ||
      m_remapVolume->voxelType() == _Rgba)
    {
      ui.checkBox->setCheckState(Qt::Unchecked);
      ui.checkBox->hide();
      m_histogramWidget->hide();
      m_gradientWidget->hide();
    }
}

bool
RemapWidget::setFile(QString flnm)
{  
  m_ok = false;
  Global::statusBar()->clearMessage();

  hideWidgets();

  if (m_histogramWidget)
    delete m_histogramWidget;

  if (m_imageWidget)
    delete m_imageWidget;

  if (m_remapVolume)
    delete m_remapVolume;
  
  if (m_gradientWidget)
    delete m_gradientWidget;

  if (m_slider)
    delete m_slider;

  m_histogramWidget = 0;
  m_imageWidget = 0;
  m_gradientWidget = 0;
  m_remapVolume = 0;
  m_slider = 0;

  m_volumeFile = flnm;

  m_remapVolume = new RemapBvf();

  if (! m_remapVolume->setFile(m_volumeFile))
    return false;

  m_ok = true;

  int d, w, h;
  m_remapVolume->gridSize(d, w, h);

  m_slider = new MySlider();
  m_slider->set(0, d-1, 0, d-1, 0);
  
  m_gradientWidget = new GradientEditorWidget();
  m_gradientWidget->setDrawBox(false);
  m_gradientWidget->setMinimumSize(200, 20);
  m_gradientWidget->setGeneralLock(GradientEditor::LockToTop);
  
  m_histogramWidget = new RemapHistogramWidget();
  m_histogramWidget->setMinimumSize(100, 300);
  m_histogramWidget->setSizePolicy(QSizePolicy::Expanding,
				   QSizePolicy::Fixed);

  ui.histogramFrame->layout()->addWidget(m_histogramWidget);
  ui.colorFrame->layout()->addWidget(m_gradientWidget);
  ui.sliderFrame->layout()->addWidget(m_slider);

  m_imageWidget = new RemapImage();
  m_imageWidget->setGridSize(d, w, h);
  m_scrollArea->setWidget(m_imageWidget);

  m_currSlice = 0;
  //---------------------------------------------------
  // ---- set initial raw min and max values
  float rawMin, rawMax;
  rawMin = m_remapVolume->rawMin();
  rawMax = m_remapVolume->rawMax();
  m_histogramWidget->setRawMinMax(rawMin, rawMax);
  m_histogramWidget->setHistogram(m_remapVolume->histogram());
  //---------------------------------------------------


  connect(m_histogramWidget, SIGNAL(getHistogram()),
	  this, SLOT(getHistogram()));  

  connect(m_histogramWidget, SIGNAL(newMapping()),
	  this, SLOT(newMapping()));  

  connect(m_histogramWidget, SIGNAL(newMinMax(float, float)),
	  this, SLOT(newMinMax(float, float)));

  connect(m_imageWidget, SIGNAL(getSlice(int)),
	  this, SLOT(getSlice(int)));  
  connect(m_imageWidget, SIGNAL(getSliceLowres(int)),
	  this, SLOT(getSliceLowres(int)));  

  connect(m_imageWidget, SIGNAL(zoomChanged()),
	  this, SLOT(updateImage()));

  connect(m_imageWidget, SIGNAL(getRawValue(int, int, int)),
	  this, SLOT(getRawValue(int, int, int)));

  connect(m_imageWidget, SIGNAL(newMinMax(float, float)),
	  this, SLOT(newMinMax(float, float)));
  
  connect(m_imageWidget, SIGNAL(saveTrimmedImages(int, int, int,
						  int, int, int)),
	  this, SLOT(saveTrimmedImages(int, int, int,
				       int, int, int)));
  
  connect(m_gradientWidget, SIGNAL(gradientChanged(QGradientStops)),
	  m_imageWidget, SLOT(setGradientStops(QGradientStops)));

  connect(m_gradientWidget, SIGNAL(gradientChanged(QGradientStops)),
	  m_histogramWidget, SLOT(setGradientStops(QGradientStops)));

  connect(m_remapVolume, SIGNAL(updateHistogram(QList<uint>)),
	  m_histogramWidget, SLOT(updateHistogram(QList<uint>)));

  connect(m_remapVolume, SIGNAL(setHiresImage(QImage, int, int, int)),
	  m_imageWidget, SLOT(setHiresImage(QImage, int, int, int)));

  connect(m_slider, SIGNAL(valueChanged(int)),
	  m_imageWidget, SLOT(sliceChanged(int)));

  connect(m_slider, SIGNAL(valueChangedLowres(int)),
	  m_imageWidget, SLOT(sliceChangedLowres(int)));

  connect(m_slider, SIGNAL(userRangeChanged(int, int)),
	  m_imageWidget, SLOT(userRangeChanged(int, int)));


  QGradientStops stops;
  stops << QGradientStop(0, Qt::black)
	<< QGradientStop(1, Qt::white);
  m_gradientWidget->setColorGradient(stops);
  m_imageWidget->setGradientStops(stops);
  m_histogramWidget->setGradientStops(stops);


  ((RemapBvf*)m_remapVolume)->gridSize(m_depth, m_width, m_height);
  ((RemapBvf*)m_remapVolume)->lowresGridSize(m_sslevel, m_ssd, m_ssw, m_ssh);

  on_butZ_clicked();

  showWidgets();
  
  return true;
}

void
RemapWidget::on_butZ_clicked()
{
  m_imageWidget->setSliceType(RemapImage::DSlice);

  int d, w, h, u0, u1;
  m_remapVolume->gridSize(d, w, h);
  m_imageWidget->depthUserRange(u0, u1);
  int v = m_imageWidget->depthSliceNumber();
  m_slider->set(0, d-1, u0, u1, v);
}
void
RemapWidget::on_butY_clicked()
{
  m_imageWidget->setSliceType(RemapImage::WSlice);

  int d, w, h, u0, u1;
  m_remapVolume->gridSize(d, w, h);
  m_imageWidget->widthUserRange(u0, u1);
  int v = m_imageWidget->widthSliceNumber();
  m_slider->set(0, w-1, u0, u1, v);
}
void
RemapWidget::on_butX_clicked()
{
  m_imageWidget->setSliceType(RemapImage::HSlice);

  int d, w, h, u0, u1;
  m_remapVolume->gridSize(d, w, h);
  m_imageWidget->heightUserRange(u0, u1);
  int v = m_imageWidget->heightSliceNumber();
  m_slider->set(0, h-1, u0, u1, v);
}

void
RemapWidget::newMinMax(float rmin, float rmax)
{
  m_remapVolume->setMinMax(rmin, rmax);
  setRawMinMax();
  m_histogramWidget->setHistogram(m_remapVolume->histogram());
}

void
RemapWidget::getRawValue(int d, int w, int h)
{
  m_imageWidget->setRawValue(m_remapVolume->rawValue(d, w, h));
}

void
RemapWidget::setRawMinMax()
{
  float rawMin, rawMax;
  rawMin = m_remapVolume->rawMin();
  rawMax = m_remapVolume->rawMax();

  m_histogramWidget->setRawMinMax(rawMin, rawMax);

  updateImage();
}

void
RemapWidget::getSlice(int slc)
{
  m_currSlice = slc;
  updateImage();
  m_slider->setValue(slc);
}

void
RemapWidget::getSliceLowres(int slc)
{
  m_currSlice = slc;

  if (ui.butZ->isChecked())
    m_imageWidget->setLowresImage(m_remapVolume->getDepthSliceLowresImage(m_currSlice),
				  m_height, m_width,
				  m_remapVolume->lowresLoD());
  else if (ui.butY->isChecked())
    m_imageWidget->setLowresImage(m_remapVolume->getWidthSliceLowresImage(m_currSlice),
				  m_height, m_depth,
				  m_remapVolume->lowresLoD());
  else if (ui.butX->isChecked())
    m_imageWidget->setLowresImage(m_remapVolume->getHeightSliceLowresImage(m_currSlice),
				  m_width, m_depth,
				  m_remapVolume->lowresLoD());

  m_slider->setValue(slc);
}

void
RemapWidget::newMapping()
{
  QList<float> rawMap;
  QList<uchar> pvlMap;

  rawMap = m_histogramWidget->rawMap();
  pvlMap = m_histogramWidget->pvlMap();

  m_remapVolume->setMap(rawMap, pvlMap);

  updateImage();
}

void
RemapWidget::getHistogram()
{
  // will be called only once
  m_histogramWidget->setHistogram(m_remapVolume->histogram());
}

void
RemapWidget::saveImage()
{
  if (m_imageWidget)
    m_imageWidget->saveImage();
  else
    QMessageBox::information(0, "Error", "Save what ???  Load a volume first !!"); 
}

void
RemapWidget::saveImages()
{
  if (m_imageWidget)
    m_imageWidget->emitSaveTrimmedImages();
  else
    QMessageBox::information(0, "Error", "Save what ???  Load a volume first !!"); 
}

void
RemapWidget::saveTrimmedImages(int dmin, int dmax,
			       int wmin, int wmax,
			       int hmin, int hmax)
{
  QString imgflnm;
  imgflnm = QFileDialog::getSaveFileName(0,
			 "Save images with basename as",
		         Global::previousDirectory(),
			 "Image Files (*.png *.tif *.bmp *.jpg)");

  if (imgflnm.isEmpty())
    return;

  QFileInfo f(imgflnm);	
  QChar fillChar = '0';
  QImage timage;

  QProgressDialog progress("Saving images",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);
  int dsz = dmax-dmin+1;
  int wsz = wmax-wmin+1;
  int hsz = hmax-hmin+1;
  QVector<QRgb> colorMap = m_imageWidget->colorMap();
  for(int d=dmin; d<=dmax; d++)
    {
      timage = m_remapVolume->getDepthSliceImage(d);
      if (timage.format() == QImage::Format_Indexed8)
	timage.setColorTable(colorMap);  
      timage = timage.copy(hmin, wmin, hsz, wsz);

      QString flname = f.absolutePath() + QDir::separator() +
	               f.baseName();
      flname += QString("%1").arg((int)d, 5, 10, fillChar);
      flname += ".";
      flname += f.completeSuffix();

      timage.save(flname);
      progress.setValue((int)(100*(float)(d-dmin)/(float)dsz));
    }
  progress.setValue(100);
}

void
RemapWidget::updateImage()
{
  if (! m_remapVolume ||
      ! m_ok)
    return;

  float zoom = qMax(m_imageWidget->zoom(), 0.03f);
  int scrollWidth = m_scrollArea->width()/zoom;
  int scrollHeight = m_scrollArea->height()/zoom;
  int scrollh = m_scrollArea->hscroll()/zoom;
  int scrollv = m_scrollArea->vscroll()/zoom;

  m_imageWidget->setOffset(m_scrollArea->hscroll(),
			   m_scrollArea->vscroll());

  int maxlevel = 0;
  if (zoom <= 0.03125) maxlevel = 5;
  else if (zoom <= 0.0625) maxlevel = 4;
  else if (zoom <= 0.125) maxlevel = 3;
  else if (zoom <= 0.25) maxlevel = 2;
  else if (zoom <= 0.5) maxlevel = 1;
  
  m_remapVolume->setMaxLoD(maxlevel);

  if (ui.butZ->isChecked())
    {
      int hstart = scrollh;
      int hend = qMin(scrollWidth + hstart, m_height);

      int wstart = scrollv;
      int wend = qMin(scrollHeight + wstart, m_width);

      m_remapVolume->startDepthSliceImage(m_currSlice, wstart, wend, hstart, hend);
    }
  else if (ui.butY->isChecked())
    {
      int hstart = scrollh;
      int hend = qMin(scrollWidth + hstart, m_height);

      int dstart = scrollv;
      int dend = qMin(scrollHeight + dstart, m_depth);

      m_remapVolume->startWidthSliceImage(m_currSlice, dstart, dend, hstart, hend);
    }
  else if (ui.butX->isChecked())
    {
      int wstart = scrollh;
      int wend = qMin(scrollWidth + wstart, m_width);

      int dstart = scrollv;
      int dend = qMin(scrollHeight + dstart, m_depth);

      m_remapVolume->startHeightSliceImage(m_currSlice, dstart, dend, wstart, wend);
    }
}

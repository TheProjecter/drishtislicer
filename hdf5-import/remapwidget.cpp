#include "global.h"
#include "common.h"
#include "remapwidget.h"
#include "raw2pvl.h"
#include "savepvldialog.h"
#include "fileslistdialog.h"
#include <QFile>

RemapWidget::RemapWidget(QWidget *parent) :
  QWidget(parent)
{
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

  m_scrollArea = new QScrollArea;
  m_scrollArea->setBackgroundRole(QPalette::Dark);
  ui.imageFrame->layout()->addWidget(m_scrollArea);


  m_histogramWidget = 0;
  m_imageWidget = 0;
  m_gradientWidget = 0;
  m_slider = 0;


  hideWidgets();

  m_timeseriesFiles.clear();
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
  m_histogramWidget->show();
  m_gradientWidget->show();

  if (m_volData.voxelType() == _Rgb ||
      m_volData.voxelType() == _Rgba)
    {
      ui.checkBox->hide();
      ui.checkBox->setCheckState(Qt::Unchecked);
      m_gradientWidget->hide();
      m_histogramWidget->hide();
    }
  else
    {
      ui.checkBox->show();
      ui.checkBox->setCheckState(Qt::Checked);
      m_histogramWidget->show();
      m_gradientWidget->show();
    }

  ui.butX->show();
  ui.butY->show();
  ui.butZ->show();
  ui.butZ->setChecked(true);
}

bool
RemapWidget::setFile(QList<QString> flnm,
		     QString plugin)
{  
  m_timeseriesFiles.clear();
  Global::statusBar()->clearMessage();

  hideWidgets();

  if (m_histogramWidget)
    delete m_histogramWidget;

  if (m_imageWidget)
    delete m_imageWidget;

  if (m_gradientWidget)
    delete m_gradientWidget;

  if (m_slider)
    delete m_slider;

  m_histogramWidget = 0;
  m_imageWidget = 0;
  m_gradientWidget = 0;
  m_slider = 0;

  m_volumeFile = flnm;

  if (! m_volData.setFile(m_volumeFile, plugin))
    return false;

  m_histogramWidget = new RemapHistogramWidget();
  m_histogramWidget->setMinimumSize(100, 300);
  m_histogramWidget->setSizePolicy(QSizePolicy::Expanding,
				   QSizePolicy::Fixed);

  m_gradientWidget = new GradientEditorWidget();
  m_gradientWidget->setDrawBox(false);
  m_gradientWidget->setMinimumSize(200, 20);
  m_gradientWidget->setGeneralLock(GradientEditor::LockToTop);
  
  int d, w, h;
  m_volData.gridSize(d, w, h);

  m_slider = new MySlider();
  m_slider->set(0, d-1, 0, d-1, 0);
  
  ui.histogramFrame->layout()->addWidget(m_histogramWidget);
  ui.colorFrame->layout()->addWidget(m_gradientWidget);
  ui.sliderFrame->layout()->addWidget(m_slider);

  m_imageWidget = new RemapImage();
  m_imageWidget->setGridSize(d, w, h);
  m_scrollArea->setWidget(m_imageWidget);

  m_currSlice = 0;

  connect(m_histogramWidget, SIGNAL(getHistogram()),
	  this, SLOT(getHistogram()));  

  connect(m_histogramWidget, SIGNAL(newMapping()),
	  this, SLOT(newMapping()));  

  connect(m_histogramWidget, SIGNAL(newMinMax(float, float)),
	  this, SLOT(newMinMax(float, float)));

  connect(m_imageWidget, SIGNAL(getSlice(int)),
	  this, SLOT(getSlice(int)));  

  connect(m_imageWidget, SIGNAL(getRawValue(int, int, int)),
	  this, SLOT(getRawValue(int, int, int)));

  connect(m_imageWidget, SIGNAL(newMinMax(float, float)),
	  this, SLOT(newMinMax(float, float)));

  connect(m_imageWidget, SIGNAL(saveTrimmed(int, int, int,
					    int, int, int)),
	  this, SLOT(saveTrimmed(int, int, int,
				 int, int, int)));
  
  connect(m_imageWidget, SIGNAL(saveTrimmedImages(int, int, int,
						  int, int, int)),
	  this, SLOT(saveTrimmedImages(int, int, int,
				       int, int, int)));
  
  connect(m_imageWidget, SIGNAL(extractRawVolume()),
	  this, SLOT(extractRawVolume()));
  
  connect(m_gradientWidget, SIGNAL(gradientChanged(QGradientStops)),
	  m_imageWidget, SLOT(setGradientStops(QGradientStops)));

  connect(m_gradientWidget, SIGNAL(gradientChanged(QGradientStops)),
	  m_histogramWidget, SLOT(setGradientStops(QGradientStops)));


  connect(m_slider, SIGNAL(valueChanged(int)),
	  m_imageWidget, SLOT(sliceChanged(int)));

  connect(m_slider, SIGNAL(userRangeChanged(int, int)),
	  m_imageWidget, SLOT(userRangeChanged(int, int)));


  QGradientStops stops;
  stops << QGradientStop(0, Qt::black)
	<< QGradientStop(1, Qt::white);
  m_gradientWidget->setColorGradient(stops);
  m_imageWidget->setGradientStops(stops);
  m_histogramWidget->setGradientStops(stops);

  setRawMinMax();
  
  showWidgets();

  return true;
}

void
RemapWidget::on_butZ_clicked()
{
  m_imageWidget->setSliceType(RemapImage::DSlice);

  int d, w, h, u0, u1;
  m_volData.gridSize(d, w, h);
  m_imageWidget->depthUserRange(u0, u1);
  m_slider->set(0, d-1, u0, u1, 0);
}
void
RemapWidget::on_butY_clicked()
{
  m_imageWidget->setSliceType(RemapImage::WSlice);

  int d, w, h, u0, u1;
  m_volData.gridSize(d, w, h);
  m_imageWidget->widthUserRange(u0, u1);
  m_slider->set(0, w-1, u0, u1, 0);
}
void
RemapWidget::on_butX_clicked()
{
  m_imageWidget->setSliceType(RemapImage::HSlice);

  int d, w, h, u0, u1;
  m_volData.gridSize(d, w, h);
  m_imageWidget->heightUserRange(u0, u1);
  m_slider->set(0, h-1, u0, u1, 0);
}

void
RemapWidget::newMinMax(float rmin, float rmax)
{
  m_volData.setMinMax(rmin, rmax);
  setRawMinMax();
  m_histogramWidget->setHistogram(m_volData.histogram());
}

void
RemapWidget::getRawValue(int d, int w, int h)
{
  m_imageWidget->setRawValue(m_volData.rawValue(d, w, h));
}

void
RemapWidget::setRawMinMax()
{
  float rawMin, rawMax;
  rawMin = m_volData.rawMin();
  rawMax = m_volData.rawMax();

  m_histogramWidget->setRawMinMax(rawMin, rawMax);

  if (ui.butZ->isChecked())
    m_imageWidget->setSliceType(RemapImage::DSlice);
  else if (ui.butY->isChecked())
    m_imageWidget->setSliceType(RemapImage::WSlice);
  else if (ui.butX->isChecked())
    m_imageWidget->setSliceType(RemapImage::HSlice);
}

void
RemapWidget::getSlice(int slc)
{
  m_currSlice = slc;

  if (ui.butZ->isChecked())
    m_imageWidget->setImage(m_volData.getDepthSliceImage(m_currSlice));
  else if (ui.butY->isChecked())
    m_imageWidget->setImage(m_volData.getWidthSliceImage(m_currSlice));
  else if (ui.butX->isChecked())
    m_imageWidget->setImage(m_volData.getHeightSliceImage(m_currSlice));

  m_slider->setValue(slc);
}

void
RemapWidget::newMapping()
{
  QList<float> rawMap;
  QList<uchar> pvlMap;

  rawMap = m_histogramWidget->rawMap();
  pvlMap = m_histogramWidget->pvlMap();

  m_volData.setMap(rawMap, pvlMap);
  
  if (ui.butZ->isChecked())
    m_imageWidget->setImage(m_volData.getDepthSliceImage(m_currSlice));
  else if (ui.butY->isChecked())
    m_imageWidget->setImage(m_volData.getWidthSliceImage(m_currSlice));
  else if (ui.butX->isChecked())
    m_imageWidget->setImage(m_volData.getHeightSliceImage(m_currSlice));
}

void
RemapWidget::getHistogram()
{
  // will be called only once
  m_histogramWidget->setHistogram(m_volData.histogram());
}

void
RemapWidget::loadLimits()
{
  if (m_imageWidget)
    m_imageWidget->loadLimits();
  else
    QMessageBox::information(0, "Error", "Load what ???  Load a volume first !!"); 
}

void
RemapWidget::saveLimits()
{
  if (m_imageWidget)
    m_imageWidget->saveLimits();
  else
    QMessageBox::information(0, "Error", "Save what ???  Load a volume first !!"); 
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
RemapWidget::saveAs()
{
  if (m_imageWidget)
    m_imageWidget->emitSaveTrimmed();
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
RemapWidget::extractRawVolume()
{
  QMessageBox::information(0, "Error", "Not implemented");
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
      timage = m_volData.getDepthSliceImage(d);
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
RemapWidget::saveTrimmed(int dmin, int dmax,
			 int wmin, int wmax,
			 int hmin, int hmax)
{
  Raw2Pvl::savePvl(&m_volData,
		   dmin, dmax, wmin, wmax, hmin, hmax,
		   m_timeseriesFiles);
  return;
}

void
RemapWidget::handleTimeSeries(QString voltype,
			      QString plugin)
{
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
					QString("Load %1").arg(voltype),
					Global::previousDirectory(),
					"*");
  
  if (flnms.size() == 0)
    return;
  
  if (flnms.count() > 1)
    {
      FilesListDialog fld(flnms);
      fld.exec();
      if (fld.result() == QDialog::Rejected)
	return;
    }
  QString mesg;
  mesg = "All operations on the first volume will be used\n";
  mesg += "as a template for others in the timeseries.\n";
  mesg += "Subsampling, smoothing, remaping applied to\n";
  mesg += "the first volume will be applied to others.";
  QMessageBox::information(0, "Time series", mesg);

  QFileInfo f(flnms[0]);
  Global::setPreviousDirectory(f.absolutePath());

  setFile(flnms, plugin);

  m_timeseriesFiles = flnms;
}

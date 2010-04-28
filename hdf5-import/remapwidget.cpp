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
  if (!Global::rgbVolume())
    {
      ui.checkBox->show();
      ui.checkBox->setCheckState(Qt::Checked);
    }
  else
    {
      ui.checkBox->setCheckState(Qt::Unchecked);
      m_gradientWidget->hide();
    }

  ui.butX->show();
  ui.butY->show();
  ui.butZ->show();
  ui.butZ->setChecked(true);

}

bool
RemapWidget::loadPlugin()
{
  QString plugindir = qApp->applicationDirPath() + QDir::separator() + "plugin";
  QString pluginflnm = QFileDialog::getOpenFileName(0,
						    "Load Plugin",
						    plugindir,
						    "dll files (*.dll)");
  QPluginLoader pluginLoader(pluginflnm);
  QObject *plugin = pluginLoader.instance();

  if (plugin)
    {
      m_volInterface = qobject_cast<VolInterface *>(plugin);
      if (m_volInterface)
	return true;
    }

  QMessageBox::information(0, "Error",
			   QString("Cannot load plugin for %1").arg(m_volumeType));

  return false;
}

bool
RemapWidget::setFile(QList<QString> flnm,
		     int voltype)
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

  m_volumeType = voltype;
  m_volumeFile = flnm;


  if (!loadPlugin())
    return false;

  m_volInterface->init();

  if (! m_volInterface->setFile(m_volumeFile))
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
  m_volInterface->gridSize(d, w, h);

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

  if (m_volInterface->voxelType() == _UChar ||
      m_volInterface->voxelType() == _Rgb ||
      m_volInterface->voxelType() == _Rgba)
    {
      ui.checkBox->setCheckState(Qt::Unchecked);
      ui.checkBox->hide();
      m_histogramWidget->hide();
    }

  return true;
}

void
RemapWidget::on_butZ_clicked()
{
  m_imageWidget->setSliceType(RemapImage::DSlice);

  int d, w, h, u0, u1;
  m_volInterface->gridSize(d, w, h);
  m_imageWidget->depthUserRange(u0, u1);
  m_slider->set(0, d-1, u0, u1, 0);
}
void
RemapWidget::on_butY_clicked()
{
  m_imageWidget->setSliceType(RemapImage::WSlice);

  int d, w, h, u0, u1;
  m_volInterface->gridSize(d, w, h);
  m_imageWidget->widthUserRange(u0, u1);
  m_slider->set(0, w-1, u0, u1, 0);
}
void
RemapWidget::on_butX_clicked()
{
  m_imageWidget->setSliceType(RemapImage::HSlice);

  int d, w, h, u0, u1;
  m_volInterface->gridSize(d, w, h);
  m_imageWidget->heightUserRange(u0, u1);
  m_slider->set(0, h-1, u0, u1, 0);
}

void
RemapWidget::newMinMax(float rmin, float rmax)
{
  m_volInterface->setMinMax(rmin, rmax);
  setRawMinMax();
  m_histogramWidget->setHistogram(m_volInterface->histogram());
}

void
RemapWidget::getRawValue(int d, int w, int h)
{
  m_imageWidget->setRawValue(m_volInterface->rawValue(d, w, h));
}

void
RemapWidget::setRawMinMax()
{
  float rawMin, rawMax;
  rawMin = m_volInterface->rawMin();
  rawMax = m_volInterface->rawMax();

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
    m_imageWidget->setImage(m_volInterface->getDepthSliceImage(m_currSlice));
  else if (ui.butY->isChecked())
    m_imageWidget->setImage(m_volInterface->getWidthSliceImage(m_currSlice));
  else if (ui.butX->isChecked())
    m_imageWidget->setImage(m_volInterface->getHeightSliceImage(m_currSlice));

  m_slider->setValue(slc);
}

void
RemapWidget::newMapping()
{
  QList<float> rawMap;
  QList<uchar> pvlMap;

  rawMap = m_histogramWidget->rawMap();
  pvlMap = m_histogramWidget->pvlMap();

  m_volInterface->setMap(rawMap, pvlMap);
  
  if (ui.butZ->isChecked())
    m_imageWidget->setImage(m_volInterface->getDepthSliceImage(m_currSlice));
  else if (ui.butY->isChecked())
    m_imageWidget->setImage(m_volInterface->getWidthSliceImage(m_currSlice));
  else if (ui.butX->isChecked())
    m_imageWidget->setImage(m_volInterface->getHeightSliceImage(m_currSlice));
}

void
RemapWidget::getHistogram()
{
  // will be called only once
  m_histogramWidget->setHistogram(m_volInterface->histogram());
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
//  QString rawFile;
//
//  if (m_volumeType == TOMVolume)
//    rawFile = QFileDialog::getSaveFileName(0,
//					   "Extract volume",
//					   Global::previousDirectory(),
//					   "All Files (*.*)");
//  else
//    {
//      QMessageBox::information(0, "Extract volume",
//			       "Currently available only for TOM files");
//      return;
//    }
//
//  if (rawFile.isEmpty())
//    return;
//
//  ((RemapTomVolume*)m_volInterface)->extractRawVolume(rawFile);
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
      timage = m_volInterface->getDepthSliceImage(d);
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
  //-----------------------------------------------------
  // -- take care of RGB/A volumes
  if (Global::rgbVolume())
    {
      QString trimRawFile;
      trimRawFile = QFileDialog::getSaveFileName(0,
			   "Save trimmed RGB volume to netCDF file",
		         Global::previousDirectory(),
			         "NetCDF Files (*.pvl.nc)");

      if (trimRawFile.isEmpty())
	return;

      m_volInterface->saveTrimmed(trimRawFile,
				  dmin, dmax,
				  wmin, wmax,
				  hmin, hmax);

      QMessageBox::information(0, "Save RGB Volume", "Done");
      return;
    }
  //-----------------------------------------------------

//  //-----------------------------------------------------
//  //-- check whether user wants to save in the new format
//  bool ok = false;
//  QStringList slevels;
//  slevels << "Yes - proceed";
//  slevels << "No";  
//  QString option = QInputDialog::getItem(0,
//					 "Save Processed Volume",
//					 "Proceed with new format",
//					 slevels,
//					 0,
//					 false,
//					 &ok);
//  if (!ok)
//    return;
//
//  QStringList op = option.split(' ');
//  if (op[0] == "Yes")
    {
      int volumeType = 0;

      if (m_volumeType == TOMVolume) volumeType = 1;
      if (m_volumeType == AnalyzeVolume) volumeType = 2;

      Raw2Pvl::savePvl(m_volInterface,
		       volumeType,
		       dmin, dmax, wmin, wmax, hmin, hmax,
		       m_timeseriesFiles);
      return;
    }
  //-----------------------------------------------------


//  //-----------------------------------------------------
//  //--  save RAW file and proceed
//  QString trimRawFile;
//  QString mappedFile;
//  QString pvlFile;
//
//  int d, w, h;
//  int dsz=dmax-dmin+1;
//  int wsz=wmax-wmin+1;
//  int hsz=hmax-hmin+1;
//  m_volInterface->gridSize(d, w, h);
//  if (d == dsz &&
//      w == wsz &&
//      h == hsz &&
//      (m_volumeType == RAWVolume ||
//       m_volumeType == TOMVolume ||
//       m_volumeType == AnalyzeVolume) )
//    {
//      if (m_volumeType == AnalyzeVolume)
//	trimRawFile = ((RemapAnalyze*)m_volInterface)->imgFile();
//      else
//	trimRawFile = m_volumeFile[0];
//    }
//  else
//    {
//      trimRawFile = QFileDialog::getSaveFileName(0,
//			   "Save trimmed raw volume",
//			 Global::previousDirectory(),
//			         "RAW Files (*.raw)");
//
//      if (trimRawFile.isEmpty())
//	return;
//
//      m_volInterface->saveTrimmed(trimRawFile,
//				    dmin, dmax,
//				    wmin, wmax,
//				    hmin, hmax);
//    }
//
//  QFileInfo ftrf(trimRawFile);
//  Global::setPreviousDirectory(ftrf.absolutePath());
//
//  if (reduceGridSize(trimRawFile,
//		     dsz, wsz, hsz))
//    return;
//  
//  // now ask for preprocessed volume file name
//  pvlFile = QFileDialog::getSaveFileName(0,
//					 "Save processed volume",
//					 Global::previousDirectory(),
//					 "NetCDF Files (*.pvl.nc)");
//  
//  if (pvlFile.isEmpty())
//    {
//      mappedFile.clear();
//      pvlFile.clear();
//    }
//  else
//    {
//      if (pvlFile.endsWith(".pvl.nc.pvl.nc"))
//	pvlFile.chop(7);
//
//      if(!pvlFile.endsWith(".pvl.nc"))
//	pvlFile += ".pvl.nc";
//
//      QFileInfo f(pvlFile);
//      mappedFile = QFileInfo(f.absolutePath(), "remapped.raw").absoluteFilePath();
//    }
//  
//
//  if (mappedFile.isEmpty())
//    return;
//
//
//  SavePvlDialog savePvlDialog;
//  if (m_volumeType == TOMVolume)
//    {
//      float vx, vy, vz;
//      m_volInterface->voxelSize(vx, vy, vz);
//      QString desc = m_volInterface->description();
//      savePvlDialog.setVoxelUnit(Raw2Pvl::_Micron);
//      savePvlDialog.setVoxelSize(vx, vy, vz);
//      savePvlDialog.setDescription(desc);
//    }
//  else if (m_volumeType == AnalyzeVolume)
//    {
//      float vx, vy, vz;
//      m_volInterface->voxelSize(vx, vy, vz);
//      savePvlDialog.setVoxelSize(vx, vy, vz);
//    }
//
//
//  savePvlDialog.exec();
//
//  int volumeFilter = savePvlDialog.volumeFilter();
//  int voxelUnit = savePvlDialog.voxelUnit();
//  QString description = savePvlDialog.description();
//  float vx, vy, vz;
//  savePvlDialog.voxelSize(vx, vy, vz);
//
//  // remap the raw file
//  applyMapping(trimRawFile, mappedFile,
//	       dsz, wsz, hsz,
//	       volumeFilter);
//
//  // create .pvl.nc file
//  createPVL(trimRawFile, mappedFile, pvlFile,
//	    dsz, wsz, hsz,
//	    voxelUnit,
//	    vx, vy, vz,
//	    description);
//
//  // remove the mapped file
//  unlink((char*)mappedFile.toStdString().c_str());
}


bool
RemapWidget::reduceGridSize(QString trimRawFile,
			    int dsz, int wsz, int hsz)
{
  bool ok;
  
  QStringList slevels;
  slevels << "No reduction";
  slevels << QString("2 [%1 %2 %3]").arg(dsz/2).arg(wsz/2).arg(hsz/2);
  slevels << QString("3 [%1 %2 %3]").arg(dsz/3).arg(wsz/3).arg(hsz/3);
  slevels << QString("4 [%1 %2 %3]").arg(dsz/4).arg(wsz/4).arg(hsz/4);
  QString option = QInputDialog::getItem(0,
					 "Volume Size",
					 "Volume grid size reduction level",
					 slevels,
					 0,
					 false,
					 &ok);
  if (!ok)
    return false;
   
  int level = 1;
  QStringList op = option.split(' ');
  if (op[0] == "2")
    level = 2;
  else if (op[0] == "3")
    level = 3;
  else if (op[0] == "4")
    level = 4;
  
  if (level == 1)
    return false;

  if (level > 1)
    {
      QStringList flevels;
      flevels << "No Filter" << "Mean" << "Median";
      QString option = QInputDialog::getItem(0,
					     "Select Subsampling Filter",
					     "Subsampling Filter",
					     flevels,
					     0,
					     false,
					     &ok);
      if (!ok)
	return false;
      
      int filter = Raw2Pvl::_NoFilter;
      if (option == "Mean")
	filter = Raw2Pvl::_MeanFilter; 
      else if (option == "Median")
	filter = Raw2Pvl::_MedianFilter;
     
      QString newTrimRawFile = QFileDialog::getSaveFileName(0,
			    "Save subsampled raw volume",
                            Global::previousDirectory(),
	       		    "RAW Files (*.raw)");
      
      int voxelType = m_volInterface->voxelType();
      int headerBytes = m_volInterface->headerBytes();

      Raw2Pvl::subsampleVolume(trimRawFile,
			       newTrimRawFile,
			       voxelType,
			       headerBytes,
			       dsz, wsz, hsz,
			       level, filter);
       
      QString mesg;
      mesg += QString("Loading %1\n").arg(newTrimRawFile);
      mesg += "Please reapply mapping as the values will have changed due to filtering";
      QMessageBox::information(0, "Loading Subsampled File",
				mesg);
      
      QStringList flnms;
      flnms << newTrimRawFile;
      setFile(flnms, RAWVolume);
      
      return true;
    }

  return false;
}


void RemapWidget::applyMapping(QString rawFilename,
			       QString mappedFilename,
			       int d, int w, int h,
			       int volumeFilter)
{
//  int voxelType = m_volInterface->voxelType();
//  int headerBytes = m_volInterface->headerBytes();
//  QList<float> rawMap = m_volInterface->rawMap();
//  QList<uchar> pvlMap = m_volInterface->pvlMap();
//  QString sourceFilename = rawFilename;
//  
//  if (volumeFilter > 0)
//    {
//      QFileInfo f(mappedFilename);
//      sourceFilename = QFileInfo(f.absolutePath(), "smooth.raw").absoluteFilePath();
//
//      Raw2Pvl::applyVolumeFilter(rawFilename,
//				 sourceFilename,
//				 voxelType,
//				 headerBytes,
//				 d, w, h,
//				 volumeFilter);
//    }
//
//  Raw2Pvl::applyMapping(sourceFilename,
//			mappedFilename,
//			voxelType,
//			headerBytes,
//			d, w, h,
//			rawMap,
//			pvlMap);
//
//  if (volumeFilter > 0)
//    {
//      QString saveFilterFile = QFileDialog::getSaveFileName(0,
//			      "Save filtered volume ?",
//			       Global::previousDirectory(),
//			       "RAW Files (*.raw)");
//
//      if (! saveFilterFile.isEmpty())
//	QFile::copy(sourceFilename, saveFilterFile);
//
//      unlink((char*)sourceFilename.toStdString().c_str());
//
//    }
}

void
RemapWidget::createPVL(QString rawFilename,
		       QString mappedFilename,
		       QString pvlFilename,
		       int depth, int width, int height,
		       int voxelUnit,
		       float vx, float vy, float  vz,
		       QString description)
{ 
//  QList<float> rawMap = m_volInterface->rawMap();
//  QList<uchar> pvlMap = m_volInterface->pvlMap();
//  uchar voxelType = m_volInterface->voxelType();  
//  int headerBytes = m_volInterface->headerBytes();
//  //headerBytes = 13;
//
//  //------------------ Saving information into netCDF file ----------------
//  NcFile pvlFile((char*)pvlFilename.toAscii().data(),
//		 NcFile::Replace);
//
//  if (!pvlFile.is_valid())
//    {
//      QMessageBox::information(0, "Error",
//			       QString("Cannot write to pvl.nc file"));
//    }
//  else
//    {
//      QString vstr;
//
//      // save relative path for the rawfile
//      QFileInfo fileInfo(pvlFilename);
//      QDir direc = fileInfo.absoluteDir();
//      vstr = direc.relativeFilePath(rawFilename);
//      pvlFile.add_att("rawfile", (char *)vstr.toAscii().data());
//
//      if (voxelType == Raw2Pvl::_UChar)      vstr = "unsigned char";
//      else if (voxelType == Raw2Pvl::_Char)  vstr = "char";
//      else if (voxelType == Raw2Pvl::_UShort)vstr = "unsigned short";
//      else if (voxelType == Raw2Pvl::_Short) vstr = "short";
//      else if (voxelType == Raw2Pvl::_Int)   vstr = "int";
//      else if (voxelType == Raw2Pvl::_Float) vstr = "float";
//      pvlFile.add_att("voxeltype", (char *)vstr.toAscii().data());
//
//      int gsz[3];
//      gsz[0] = depth;
//      gsz[1] = width;
//      gsz[2] = height;
//      pvlFile.add_att("gridsize", 3, (const int*)gsz);
//
//      int seek = headerBytes;
//      pvlFile.add_att("skipheaderbytes", 1, (const int *)&seek);
//      
//      if (voxelUnit == Raw2Pvl::_Nounit)         vstr = "no units";
//      else if (voxelUnit == Raw2Pvl::_Angstrom)  vstr = "angstrom";
//      else if (voxelUnit == Raw2Pvl::_Nanometer) vstr = "nanometer";
//      else if (voxelUnit == Raw2Pvl::_Micron)    vstr = "micron";
//      else if (voxelUnit == Raw2Pvl::_Millimeter)vstr = "millimeter";
//      else if (voxelUnit == Raw2Pvl::_Centimeter)vstr = "centimeter";
//      else if (voxelUnit == Raw2Pvl::_Meter)     vstr = "meter";
//      else if (voxelUnit == Raw2Pvl::_Kilometer) vstr = "kilometer";
//      else if (voxelUnit == Raw2Pvl::_Parsec)    vstr = "parsec";
//      else if (voxelUnit == Raw2Pvl::_Kiloparsec)vstr = "kiloparsec";
//      pvlFile.add_att("voxelunit", (char *)vstr.toAscii().data());
//
//      float vsz[3];
//      vsz[0] = vx;
//      vsz[1] = vy;
//      vsz[2] = vz;
//      pvlFile.add_att("voxelsize", 3, (const float*)vsz);
//
//      float *rawmap;
//      uchar *pvlmap;
//      rawmap = new float[rawMap.size()];
//      pvlmap = new unsigned char[rawMap.size()];
//      for (int i=0; i<rawMap.size(); i++)
//	{
//	  rawmap[i] = rawMap[i];
//	  pvlmap[i] = pvlMap[i];
//	}
//
//      pvlFile.add_att("mappvl", rawMap.size(), (const ncbyte*)pvlmap);
//      pvlFile.add_att("mapraw", rawMap.size(), (const float*)rawmap);
//
//      delete [] rawmap;
//      delete [] pvlmap;
//
//
//      if (description.size() < 255)
//	description = description.leftJustified(255, ' ');
//      else
//	description.truncate(255);
//      pvlFile.add_att("description", (char*)description.toAscii().data());
//
//      pvlFile.sync();
//      pvlFile.close();
//    }
//  //-----------------------------------------------------------------------
//
//  Raw2Pvl::applyRaw2PvlConversion(mappedFilename,
//				  pvlFilename,
//				  depth, width, height);
//
//  QMessageBox::information(0, "Processing", "Done");
}

bool
RemapWidget::getVolumeInfo(QString volfile,
			   int &skipheaderbytes,
			   uchar &voxelType,
			   int &voxelUnit,
			   float &vx, float &vy, float &vz,
			   QString &description,
			   QList<float> &rawMap,
			   QList<uchar> &pvlMap,
			   int &depth,
			   int &width,
			   int &height)
{
//  NcError err(NcError::verbose_nonfatal);
//
//  NcFile pvlFile(volfile.toAscii().data(), NcFile::ReadOnly);
//
//  if (!pvlFile.is_valid())
//    {
//      QMessageBox::information(0, "Error",
//       QString("%1 is not a valid preprocessed volume file").arg(volfile));
//      return false;
//    }
//
//  int i;
//  NcAtt *att;
//  char *attval;
//  QString pvalue;
//
//  att = pvlFile.get_att("description");
//  if (att)
//    {
//      attval = att->as_string(0);
//      description = attval;
//      delete [] attval;
//    }
//
//  att = pvlFile.get_att("voxeltype");
//  if (att)
//    {
//      attval = att->as_string(0);
//      pvalue = attval;
//      if (pvalue == "unsigned char")
//	voxelType = Raw2Pvl::_UChar;
//      if (pvalue == "char")
//	voxelType = Raw2Pvl::_Char;
//      if (pvalue == "unsigned short")
//	voxelType = Raw2Pvl::_UShort;
//      if (pvalue == "short")
//	voxelType = Raw2Pvl::_Short;
//      if (pvalue == "int")
//	voxelType = Raw2Pvl::_Int;
//      if (pvalue == "float")
//	voxelType = Raw2Pvl::_Float;
//      delete [] attval;
//    }
//
//
//  att = pvlFile.get_att("voxelunit");
//  if (att)
//    { 
//      attval = att->as_string(0);
//      pvalue = attval;
//      voxelUnit = Raw2Pvl::_Nounit;
//      if (pvalue == "angstrom")
//	voxelUnit = Raw2Pvl::_Angstrom;
//      else if (pvalue == "nanometer")
//	voxelUnit = Raw2Pvl::_Nanometer;
//      else if (pvalue == "micron")
//	voxelUnit = Raw2Pvl::_Micron;
//      else if (pvalue == "millimeter")
//	voxelUnit = Raw2Pvl::_Millimeter;
//      else if (pvalue == "centimeter")
//	voxelUnit = Raw2Pvl::_Centimeter;
//      else if (pvalue == "meter")
//	voxelUnit = Raw2Pvl::_Meter;
//      else if (pvalue == "kilometer")
//	voxelUnit = Raw2Pvl::_Kilometer;
//      else if (pvalue == "parsec")
//	voxelUnit = Raw2Pvl::_Parsec;
//      else if (pvalue == "kiloparsec")
//	voxelUnit = Raw2Pvl::_Kiloparsec;
//      delete [] attval;
//    }
//  
//  
//  att = pvlFile.get_att("gridsize");
//  if (att)
//    {
//      depth = att->as_int(0);
//      width = att->as_int(1);
//      height = att->as_int(2);
//    }
//
//  att = pvlFile.get_att("voxelsize");
//  if (att)
//    {
//      vx = att->as_float(0);
//      vy = att->as_float(1);
//      vz = att->as_float(2);
//    }
//
//  att = pvlFile.get_att("skipheaderbytes");
//  if (att)
//    skipheaderbytes = att->as_int(0);
//  
//  att = pvlFile.get_att("mapraw");
//  if (att)
//    {
//      for(i=0; i<att->num_vals(); i++)
//	rawMap.append(att->as_float(i));
//  
//      att = pvlFile.get_att("mappvl");
//      for(i=0; i<att->num_vals(); i++)
//	pvlMap.append(att->as_ncbyte(i));
//    }
//
//  pvlFile.close();
  return true;
}

void
RemapWidget::handleTimeSeries()
{
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
					"Load Raw Files",
					Global::previousDirectory(),
					"RAW Files (*.raw *.*)");
  
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

  setFile(flnms, RAWVolume);

  m_timeseriesFiles = flnms;
}

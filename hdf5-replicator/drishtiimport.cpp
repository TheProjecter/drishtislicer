#include "global.h"
#include "staticfunctions.h"
#include "drishtiimport.h"
#include "remapwidget.h"
#include "fileslistdialog.h"
#include "raw2pvl.h"

#include <QFile>
#include <QTextStream>
#include <QDomDocument>

//#include <QLibrary>

DrishtiImport::DrishtiImport(QWidget *parent) :
  QMainWindow(parent)
{
  ui.setupUi(this);

  setWindowIcon(QPixmap(":/images/drishti_import_32.png"));

  setAcceptDrops(true);

  m_remapWidget = new RemapWidget();
  setCentralWidget(m_remapWidget);

  StaticFunctions::initQColorDialog();

  loadSettings();

  Global::setStatusBar(statusBar());
}

void
DrishtiImport::on_saveLimits_triggered()
{
  m_remapWidget->saveLimits();
}

void
DrishtiImport::on_loadLimits_triggered()
{
  m_remapWidget->loadLimits();
}

void
DrishtiImport::on_saveImage_triggered()
{
  m_remapWidget->saveImage();
}

void
DrishtiImport::on_actionHelp_triggered()
{
//  QString app;
//#if defined(Q_OS_LINUX)
//  app = 
//    QLibraryInfo::location(QLibraryInfo::BinariesPath) + 
//    QDir::separator() +
//    QLatin1String(DRISHTIIMPORT_ASSISTANT);
//#elif defined(Q_OS_WIN32)
//  app = QCoreApplication::applicationDirPath() + "/assistant";
//#elif defined(Q_OS_MAC)
//  QDir appd=QCoreApplication::applicationDirPath();
//  appd.cdUp();
//  appd.cdUp();
//  appd.cdUp();
//  appd.cd("Assistant.app");
//  appd.cd("Contents");
//  appd.cd("MacOS");
//  
//  app = appd.absoluteFilePath("Assistant");
//#else
//  #error Unsupported platform.
//#endif
//
//
//  QStringList args;
//  args << "-collectionFile";
//  args << QFileInfo(Global::documentationPath(),
//		   "drishtiimport.qhc").absoluteFilePath();
//  args << "-enableRemoteControl";
//
//
//  QProcess *process = new QProcess(this);
//  process->start(app, args);
//
//  if (!process->waitForStarted())
//    {
//      QMessageBox::critical(this,  "Remote Control",
//			    QString("Unable to launch Help."));
//      return;
//    }
}

void
DrishtiImport::loadSettings()
{
  QString homePath = QDir::homePath();
  QFileInfo settingsFile(homePath, ".drishti.import");
  QString flnm = settingsFile.absoluteFilePath();  

  if (! settingsFile.exists())
    return;

  QDomDocument document;
  QFile f(flnm.toAscii().data());
  if (f.open(QIODevice::ReadOnly))
    {
      document.setContent(&f);
      f.close();
    }

  QDomElement main = document.documentElement();
  QDomNodeList dlist = main.childNodes();
  for(uint i=0; i<dlist.count(); i++)
    {
      if (dlist.at(i).nodeName() == "previousdirectory")
	{
	  QString str = dlist.at(i).toElement().text();
	  Global::setPreviousDirectory(str);
	}
    }
}

void
DrishtiImport::saveSettings()
{
  QString str;
  QDomDocument doc("Drishti_Import_v1.0");

  QDomElement topElement = doc.createElement("DrishtiImportSettings");
  doc.appendChild(topElement);

  {
    QDomElement de0 = doc.createElement("previousdirectory");
    QDomText tn0;
    tn0 = doc.createTextNode(Global::previousDirectory());
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  QString homePath = QDir::homePath();
  QFileInfo settingsFile(homePath, ".drishti.import");
  QString flnm = settingsFile.absoluteFilePath();  

  QFile f(flnm.toAscii().data());
  if (f.open(QIODevice::WriteOnly))
    {
      QTextStream out(&f);
      doc.save(out, 2);
      f.close();
    }
  else
    QMessageBox::information(0, "Cannot save ", flnm.toAscii().data());
}

void
DrishtiImport::Old2New(QStringList flnms)
{
  if (flnms.count() > 1)
    {
      FilesListDialog fld(flnms);
      fld.exec();
      if (fld.result() == QDialog::Rejected)
	{
	  QMessageBox::information(0, "Convert", "No conversion done");
	  return;
	}
    }

  QFileInfo f(flnms[0]);
  Global::setPreviousDirectory(f.absolutePath());


  QString direc = QFileDialog::getExistingDirectory(0,
						    "Save Converted file to",
						    Global::previousDirectory(),
						    QFileDialog::ShowDirsOnly |
						    QFileDialog::DontResolveSymlinks); 

  for (uint i=0; i<flnms.count(); i++)
    Raw2Pvl::Old2New(flnms[i], direc);

  QMessageBox::information(0, "Conversion", "-----Done-----");
}

void
DrishtiImport::on_actionConvert_triggered()
{
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
				       "Load Old-style NetCDF File/s",
				       Global::previousDirectory(),
				       "NetCDF Files (*.nc)");
  
  if (flnms.size() == 0)
    return;

  Old2New(flnms);
}

void
DrishtiImport::on_actionTimeSeries_triggered()
{
  m_remapWidget->handleTimeSeries();
}

void
DrishtiImport::on_actionRAW_triggered()
{
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
				       "Load RAW File",
				       Global::previousDirectory(),
				       "RAW Files (*.raw *.raw.* *.*)");
  
  if (flnms.size() == 0)
    return;

  if (flnms.count() > 1)
    {
      FilesListDialog fld(flnms);
      fld.exec();
      if (fld.result() == QDialog::Rejected)
	return;
    }

  QFileInfo f(flnms[0]);
  Global::setPreviousDirectory(f.absolutePath());

  if (flnms.count() == 1)
    m_remapWidget->setFile(flnms, RemapWidget::RAWVolume);
  else
    m_remapWidget->setFile(flnms, RemapWidget::RawSlabs);
}

void
DrishtiImport::on_actionAnalyze_triggered()
{
  QString flnm;
  flnm = QFileDialog::getOpenFileName(0,
				      "Load HDR File",
				      Global::previousDirectory(),
				      "HDR Files (*.hdr)");
  
  if (flnm.isEmpty())
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  m_remapWidget->setFile(flnms, RemapWidget::AnalyzeVolume);
}

void
DrishtiImport::on_actionTOM_triggered()
{
  QString flnm;
  flnm = QFileDialog::getOpenFileName(0,
				      "Load TOM File",
				      Global::previousDirectory(),
				      "TOM Files (*.tom)");
  
  if (flnm.isEmpty())
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::TOMVolume);
}

void
DrishtiImport::on_actionNetCDF_triggered()
{
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
					"Load NetCDF File/s",
					Global::previousDirectory(),
					"NetCDF Files (*.nc)");
  
  if (flnms.size() == 0)
    return;

  if (flnms.count() > 1)
    {
      FilesListDialog fld(flnms);
      fld.exec();
      if (fld.result() == QDialog::Rejected)
	return;
    }

  QFileInfo f(flnms[0]);
  Global::setPreviousDirectory(f.absolutePath());

  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::NCVolume);
}

void
DrishtiImport::on_actionImage_directory_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "Image Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::ImageVolume);
}

void
DrishtiImport::on_actionRGB_directory_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "Image Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(true);
  m_remapWidget->setFile(flnms, RemapWidget::ImageVolume);
}

void
DrishtiImport::on_actionDicom_directory_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "Dicom Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::ImageMagickVolume);
}

void
DrishtiImport::on_actionImageMagick_directory_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "16/32 bit Grayscale Image Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::ImageMagickVolume);
}

void
DrishtiImport::on_actionHDF4_directory_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "HDF4 Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::HDF4Volume);
}

void
DrishtiImport::on_actionRaw_slices_triggered()
{
  QString flnm;
  flnm = QFileDialog::getExistingDirectory(0,
					   "Raw slices Directory",
					   Global::previousDirectory(),
					   QFileDialog::ShowDirsOnly |
					   QFileDialog::DontResolveSymlinks);
  
  if (flnm.size() == 0)
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;
  Global::setRGBVolume(false);
  m_remapWidget->setFile(flnms, RemapWidget::RawSlices);
}

void
DrishtiImport::on_actionSave_As_triggered()
{
  m_remapWidget->saveAs();
}

void
DrishtiImport::on_actionSave_Images_triggered()
{
  m_remapWidget->saveImages();
}

void
DrishtiImport::on_actionExit_triggered()
{
  saveSettings();
  close();
}

void
DrishtiImport::closeEvent(QCloseEvent *)
{
  on_actionExit_triggered();
}

void
DrishtiImport::dragEnterEvent(QDragEnterEvent *event)
{
  if (event && event->mimeData())
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls())
	{
	  QList<QUrl> urls = data->urls();

	  QFileInfo f((data->urls())[0].toLocalFile());
	  if (f.isDir())
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLs(urls, ".pvl.nc"))
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLs(urls, ".nc"))
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLs(urls, ".raw"))
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLsRegExp(urls, "*.raw.*"))
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLs(urls, ".hdr"))
	    {
	      event->acceptProposedAction();
	    }
	  else if (StaticFunctions::checkURLs(urls, ".tom"))
	    {
	      event->acceptProposedAction();
	    }
	  else
	    {
	      if (urls.count() == 1)
		{
		  bool ok = false;
		  QStringList slevels;
		  slevels << "Yes - proceed";
		  slevels << "No";  
		  QString option = QInputDialog::getItem(0,
				   "Drag and Drop",
				   QString("File ")+
				   urls[0].toLocalFile()+
				   QString(" does not have one of the acceptable extensions.\nTreat it as a RAW file?"),
				   slevels,
				   0,
				   false,
				   &ok);
		  if (!ok)
		    return;

		  QStringList op = option.split(' ');
		  if (op[0] == "Yes")		    
		    event->acceptProposedAction();
		}
	      else
		{
		  QString str;

		  for(uint i=0; i<urls.count(); i++)
		    str += urls[i].toLocalFile().toAscii() + "\n";

		  QMessageBox::information(0, "Drag and Drop",
					   QString("Some of the following files does not have ")+
					   QString("one of the acceptable extensions\n")+
					   str);
		}
	    }
	}
    }
}

void
DrishtiImport::dropEvent(QDropEvent *event)
{
  if (event && event->mimeData())
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls())
	{
	  QUrl url = data->urls()[0];
	  QFileInfo info(url.toLocalFile());
	  if (info.isDir())
	    {
	      QStringList dtypes;
	      dtypes << "Standard Images"
		     << "Standard Images for RGB volume"
		     << "Dicom Images"
		     << "16/32 bit Grayscale Images"
		     << "HDF4"
	             << "Raw slices";
	      QString option = QInputDialog::getItem(0,
						     "Select Directory Type",
						     "Directory Type",
						     dtypes,
						     0,
						     false);

	      QFileInfo f(url.toLocalFile());
	      Global::setPreviousDirectory(f.absolutePath());

	      QStringList flnms;
	      flnms << url.toLocalFile();

	      Global::setRGBVolume(false);
	      if (option == "Standard Images for RGB volume")
		{
		  Global::setRGBVolume(true);
		  m_remapWidget->setFile(flnms, RemapWidget::ImageVolume);
		}
	      else if (option == "Standard Images")
		m_remapWidget->setFile(flnms, RemapWidget::ImageVolume);
	      else if (option == "HDF4")
		m_remapWidget->setFile(flnms, RemapWidget::HDF4Volume);
	      else if (option == "Raw slices")
		m_remapWidget->setFile(flnms, RemapWidget::RawSlices);
	      else
		m_remapWidget->setFile(flnms, RemapWidget::ImageMagickVolume);
	    }
	  if (info.exists() && info.isFile())
	    {
	      if (StaticFunctions::checkExtension(url.toLocalFile(), ".pvl.nc"))
		{
		  bool ok = false;
		  QStringList slevels;
		  slevels << "Yes - proceed";
		  slevels << "No";  
		  QString option = QInputDialog::getItem(0,
				   "Drag and Drop",
				   QString("Convert ")+
				   url.toLocalFile()+
				   QString(" to new format ?"),
				   slevels,
				   0,
				   false,
				   &ok);
		  if (!ok)
		    return;

		  QStringList op = option.split(' ');
		  if (op[0] == "No")		    
		    return;

		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();

		  Old2New(flnms);
		}
	      else if (StaticFunctions::checkExtension(url.toLocalFile(), ".nc"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());

		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();

		  if (flnms.count() > 1)
		    {
		      FilesListDialog fld(flnms);
		      fld.exec();
		      if (fld.result() == QDialog::Rejected)
			return;
		    }

		  Global::setRGBVolume(false);
		  m_remapWidget->setFile(flnms, RemapWidget::NCVolume);
		}
	      else if (StaticFunctions::checkExtension(url.toLocalFile(), ".raw"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  Global::setRGBVolume(false);
		  if (flnms.count() == 1)
		    m_remapWidget->setFile(flnms, RemapWidget::RAWVolume);
		  else
		    m_remapWidget->setFile(flnms, RemapWidget::RawSlabs);
		}
	      else if (StaticFunctions::checkRegExp(url.toLocalFile(), "*.raw.*"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  if (flnms.count() > 1)
		    {
		      FilesListDialog fld(flnms);
		      fld.exec();
		      if (fld.result() == QDialog::Rejected)
			return;
		    }

		  Global::setRGBVolume(false);
		  m_remapWidget->setFile(flnms, RemapWidget::RawSlabs);
		}
	      else if (StaticFunctions::checkExtension(url.toLocalFile(), ".hdr"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  Global::setRGBVolume(false);
		  m_remapWidget->setFile(flnms, RemapWidget::AnalyzeVolume);
		}
	      else if (StaticFunctions::checkExtension(url.toLocalFile(), ".tom"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  Global::setRGBVolume(false);
		  m_remapWidget->setFile(flnms, RemapWidget::TOMVolume);
		}
	      else // -- treat the file as raw file
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(uint i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  Global::setRGBVolume(false);
		  m_remapWidget->setFile(flnms, RemapWidget::RAWVolume);
		}
	    }
	}
    }
}



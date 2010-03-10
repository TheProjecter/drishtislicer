#include "global.h"
#include "staticfunctions.h"
#include "drishtislicer.h"
#include "remapwidget.h"

#include <QFile>
#include <QTextStream>
#include <QDomDocument>

DrishtiSlicer::DrishtiSlicer(QWidget *parent) :
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
DrishtiSlicer::loadSettings()
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
  for(int i=0; i<dlist.count(); i++)
    {
      if (dlist.at(i).nodeName() == "previousdirectory")
	{
	  QString str = dlist.at(i).toElement().text();
	  Global::setPreviousDirectory(str);
	}
    }
}

void
DrishtiSlicer::saveSettings()
{
  QString str;
  QDomDocument doc("Drishti_Slicer_v1.0");

  QDomElement topElement = doc.createElement("DrishtiSlicerSettings");
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
DrishtiSlicer::on_actionBVF_triggered()
{
  QString flnm;
  flnm = QFileDialog::getOpenFileName(0,
				      "Load BVF File",
				      Global::previousDirectory(),
				      "BVF Files (*.bvf | *.rawbvf | *.dvf)");
  if (flnm.isEmpty())
    return;

  QFileInfo f(flnm);
  Global::setPreviousDirectory(f.absolutePath());

  QStringList flnms;
  flnms << flnm;  
  m_remapWidget->setFile(flnms);
}

void
DrishtiSlicer::closeEvent(QCloseEvent *)
{
  saveSettings();
  close();
}

void
DrishtiSlicer::dragEnterEvent(QDragEnterEvent *event)
{
  if (event && event->mimeData())
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls())
	{
	  QList<QUrl> urls = data->urls();

	  QFileInfo f((data->urls())[0].toLocalFile());
	  if (StaticFunctions::checkURLs(urls, ".bvf") ||
	      StaticFunctions::checkURLs(urls, ".rawbvf") ||
	      StaticFunctions::checkURLs(urls, ".dvf"))
	    {
	      event->acceptProposedAction();
	    }
	}
    }
}

void
DrishtiSlicer::dropEvent(QDropEvent *event)
{
  if (event && event->mimeData())
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls())
	{
	  QUrl url = data->urls()[0];
	  QFileInfo info(url.toLocalFile());
	  if (info.exists() && info.isFile())
	    {
	      if (StaticFunctions::checkExtension(url.toLocalFile(), ".bvf") ||
		  StaticFunctions::checkExtension(url.toLocalFile(), ".rawbvf") ||
		  StaticFunctions::checkExtension(url.toLocalFile(), ".dvf"))
		{
		  QFileInfo f((data->urls())[0].toLocalFile());
		  Global::setPreviousDirectory(f.absolutePath());
		  
		  QStringList flnms;
		  for(int i=0; i<data->urls().count(); i++)
		    flnms << (data->urls())[i].toLocalFile();
		  
		  m_remapWidget->setFile(flnms);
		}
	    }
	}
    }
}


void
DrishtiSlicer::on_actionExit_triggered()
{
  saveSettings();
  close();
}
void
DrishtiSlicer::on_actionSave_Images_triggered()
{
  m_remapWidget->saveImages();
}
void
DrishtiSlicer::on_saveImage_triggered()
{
  m_remapWidget->saveImage();
}


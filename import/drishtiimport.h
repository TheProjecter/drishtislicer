#ifndef DRISHTIIMPORT
#define DRISHTIIMPORT

#include <QtGui>
#include "ui_drishtiimport.h"

class RemapWidget;

class DrishtiImport : public QMainWindow
{
  Q_OBJECT

 public :
  DrishtiImport(QWidget *parent=0);

 protected :
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);
  void closeEvent(QCloseEvent *event);
  
 private slots :
  void on_actionHelp_triggered();
  void on_actionConvert_triggered();
  void on_actionTimeSeries_triggered();
  void on_actionRAW_triggered();
  void on_actionAnalyze_triggered();
  void on_actionTOM_triggered();
  void on_actionNetCDF_triggered();
  void on_actionImage_directory_triggered();
  void on_actionRGB_directory_triggered();
  void on_actionDicom_directory_triggered();
  void on_actionImageMagick_directory_triggered();
  void on_actionHDF4_directory_triggered();
  void on_actionRaw_slices_triggered();

  void on_actionSave_Images_triggered();
  void on_actionSave_As_triggered();
  void on_actionExit_triggered();

  void on_loadLimits_triggered();
  void on_saveLimits_triggered();
  void on_saveImage_triggered();

 private :
  Ui::DrishtiImport ui;

  RemapWidget *m_remapWidget;

  void loadSettings();
  void saveSettings();

  void Old2New(QStringList);
};

#endif

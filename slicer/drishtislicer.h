#ifndef DRISHTISLICER
#define DRISHTISLICER

#include <QtGui>
#include "ui_drishtislicer.h"

class RemapWidget;

class DrishtiSlicer : public QMainWindow
{
  Q_OBJECT

 public :
  DrishtiSlicer(QWidget *parent=0);

 protected :
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);
  void closeEvent(QCloseEvent *event);
  
 private slots :
  void on_actionBVF_triggered();
  void on_actionSave_Images_triggered();
  void on_actionExit_triggered();
  void on_saveImage_triggered();

 private :
  Ui::DrishtiSlicer ui;

  RemapWidget *m_remapWidget;

  void loadSettings();
  void saveSettings();
  void loadBvf(QString);
};

#endif

#ifndef REMAPWIDGET_H
#define REMAPWIDGET_H

#include "ui_remapwidget.h"

#include "remaphistogramwidget.h"
#include "remapbvf.h"
#include "remapimage.h"
#include "gradienteditorwidget.h"
#include "myslider.h"
#include "myscrollarea.h"

class RemapWidget : public QWidget
{
  Q_OBJECT

 public :
  RemapWidget(QWidget *parent=NULL);

  bool setFile(QString);

 public slots :
  void getHistogram();  
  void setRawMinMax();
  void newMapping();
  void getSlice(int);
  void getSliceLowres(int);
  void getRawValue(int, int, int);
  void newMinMax(float, float);
  void on_butZ_clicked();
  void on_butY_clicked();
  void on_butX_clicked();

  void saveImage();
  void saveImages();

  void saveTrimmedImages(int, int,
			 int, int,
			 int, int);

  void updateImage();

 private :
  Ui::RemapWidget ui;

  bool m_ok;
  QString m_volumeFile;

  AbstractRemapVolume *m_remapVolume;
  RemapHistogramWidget *m_histogramWidget;
  RemapImage *m_imageWidget;
  GradientEditorWidget *m_gradientWidget;
  MySlider *m_slider;

  //QScrollArea *m_scrollArea;
  MyScrollArea *m_scrollArea;

  int m_currSlice;
  int m_depth, m_width, m_height;
  int m_sslevel, m_ssd, m_ssw, m_ssh;

  void applyMapping(QString, QString,
		    int, int, int,
		    int);

  void showWidgets();
  void hideWidgets();
};

#endif

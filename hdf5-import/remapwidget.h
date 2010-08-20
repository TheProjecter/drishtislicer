#ifndef REMAPWIDGET_H
#define REMAPWIDGET_H

#include "ui_remapwidget.h"

#include "volumedata.h"

#include "remaphistogramwidget.h"
#include "remapimage.h"
#include "gradienteditorwidget.h"
#include "myslider.h"

class RemapWidget : public QWidget
{
  Q_OBJECT

 public :
  RemapWidget(QWidget *parent=NULL);

  bool setFile(QList<QString>,
	       QString);
  
  enum VolumeType
  {
    NoVolume,
    RAWVolume,
    NCVolume,
    TOMVolume,
    AnalyzeVolume,
    ImageVolume,
    ImageMagickVolume,
    HDF4Volume,
    RawSlices,
    RawSlabs
  };

 public slots :
  void loadLimits();
  void saveLimits();
  void saveImage();
  void saveAs();
  void saveImages();
  void getHistogram();  
  void setRawMinMax();
  void newMapping();
  void getSlice(int);
  void getRawValue(int, int, int);
  void newMinMax(float, float);
  void on_butZ_clicked();
  void on_butY_clicked();
  void on_butX_clicked();

  void saveTrimmed(int, int,
		   int, int,
		   int, int);
  void saveTrimmedImages(int, int,
			 int, int,
			 int, int);
  void extractRawVolume();

  void handleTimeSeries(QString, QString);

 private :
  Ui::RemapWidget ui;

  VolumeData m_volData;

  QList<QString> m_volumeFile;

  RemapHistogramWidget *m_histogramWidget;
  RemapImage *m_imageWidget;
  GradientEditorWidget *m_gradientWidget;
  MySlider *m_slider;

  QScrollArea *m_scrollArea;

  QStringList m_timeseriesFiles;

  int m_currSlice;

  void showWidgets();
  void hideWidgets();
};

#endif

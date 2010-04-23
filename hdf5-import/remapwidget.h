#ifndef REMAPWIDGET_H
#define REMAPWIDGET_H

#include "ui_remapwidget.h"

#include "volinterface.h"

#include "remaphistogramwidget.h"
#include "remapimage.h"
#include "gradienteditorwidget.h"
#include "myslider.h"

class RemapWidget : public QWidget
{
  Q_OBJECT

 public :
  RemapWidget(QWidget *parent=NULL);

  bool setFile(QList<QString> flnm,
	       int volType);
  
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

  void handleTimeSeries();

 private :
  Ui::RemapWidget ui;

  VolInterface *m_volInterface;

  QList<QString> m_volumeFile;
  int m_volumeType;

  RemapHistogramWidget *m_histogramWidget;
  RemapImage *m_imageWidget;
  GradientEditorWidget *m_gradientWidget;
  MySlider *m_slider;

  QScrollArea *m_scrollArea;

  QStringList m_timeseriesFiles;

  int m_currSlice;

  bool loadPlugin();

  bool reduceGridSize(QString,
		      int, int, int);

  void applyMapping(QString, QString,
		    int, int, int,
		    int);

  void createPVL(QString, QString, QString,
		 int, int, int,
		 int,
		 float, float, float,
		 QString);

  void showWidgets();
  void hideWidgets();


  bool getVolumeInfo(QString,
		     int&,
		     uchar&,
		     int&,
		     float&, float&, float&,
		     QString&,
		     QList<float>&,
		     QList<uchar>&,
		     int&,
		     int&,
		     int&);

  void applyMappingTimeSeries(QString,
			      QString,
			      int, int, int,
			      int,
			      int,
			      int,
			      QList<float>,
			      QList<uchar>);

  void createPVLTimeSeries(QString,
			   QString,
			   QString,
			   int, int, int,
			   int,
			   float, float, float,
			   QString,
			   int,
			   int,
			   QList<float>,
			   QList<uchar>);

};

#endif

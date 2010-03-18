#ifndef REMAPIMAGEVOLUME_H
#define REMAPIMAGEVOLUME_H

#include <QtGui>

#include "abstractremapvolume.h"

class RemapImageVolume : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapImageVolume();
  ~RemapImageVolume();

  static void savePvlHeader(QString,
			    int, int, int,
			    QString,
			    int);

  bool setFile(QList<QString>);

  void gridSize(int&, int&, int&);
  QList<uint> histogram();

  void setMinMax(float, float);
  float rawMin();
  float rawMax();  

  void setMap(QList<float>,
	      QList<uchar>);

  void getDepthSlice(int, uchar*);
  QImage getDepthSliceImage(int);
  QImage getWidthSliceImage(int);
  QImage getHeightSliceImage(int);

  QPair<QVariant,QVariant> rawValue(int, int, int);

  void saveTrimmed(QString,
		   int, int, int, int, int, int);
  void saveTrimmedRGB(QString,
		   int, int, int, int, int, int);

 private :
  int m_bytesPerVoxel;

  QList<QString> m_imageList;

  void findMinMax();
  void generateHistogram();
  void findMinMaxandGenerateHistogram();
};


#endif

#ifndef REMAPHDF4_H
#define REMAPHDF4_H

#include <QtGui>

#include "abstractremapvolume.h"

class RemapHDF4 : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapHDF4();
  ~RemapHDF4();

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

 private :
  int m_bytesPerVoxel;
  int m_Index;

  QList<QString> m_imageList;

  void findMinMax();
  void generateHistogram();
  void findMinMaxandGenerateHistogram();
};


#endif

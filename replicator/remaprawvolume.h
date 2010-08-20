#ifndef REMAPRAWVOLUME_H
#define REMAPRAWVOLUME_H

#include <QtGui>

#include "abstractremapvolume.h"

class RemapRawVolume : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapRawVolume();
  ~RemapRawVolume();

  bool setFile(QList<QString>);
  void replaceFile(QString);
  void setVoxelType(int);
  void setGridSize(int, int, int);
  void setSkipHeaderBytes(int);

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
  int m_skipBytes;
  int m_bytesPerVoxel;

  void findMinMax();
  void generateHistogram();
  void findMinMaxandGenerateHistogram();
};

#endif

#ifndef REMAPRAWSLABS_H
#define REMAPRAWSLABS_H

#include <QtGui>

#include "abstractremapvolume.h"

class RemapRawSlabs : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapRawSlabs();
  ~RemapRawSlabs();

  bool setFile(QList<QString>);
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
  QList<int> m_slices;

  void findMinMax();
  void generateHistogram();
  void findMinMaxandGenerateHistogram();
};

#endif

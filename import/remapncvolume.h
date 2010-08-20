#ifndef REMAPNCVOLUME_H
#define REMAPNCVOLUME_H

#include <QtGui>

#include "abstractremapvolume.h"

class RemapNcVolume : public AbstractRemapVolume
{
  Q_OBJECT
    
 public :
  RemapNcVolume();
  ~RemapNcVolume();

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

  QList<int> m_depthList;

  QString m_varName;

  QList<QString> listAllVariables();
  void findMinMax();
  void generateHistogram();
  void findMinMaxandGenerateHistogram();
};

#endif

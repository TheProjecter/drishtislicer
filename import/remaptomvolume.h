#ifndef REMAPTOMVOLUME_H
#define REMAPTOMVOLUME_H

#include <QtGui>

#include "abstractremapvolume.h"
#include "tomhead.h"

class RemapTomVolume : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapTomVolume();
  ~RemapTomVolume();

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

  void extractRawVolume(QString);

 private :
  thead m_tHead;

  int m_skipBytes;
  int m_bytesPerVoxel;

  void generateHistogram();
};

#endif

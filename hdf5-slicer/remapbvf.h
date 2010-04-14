#ifndef REMAPBVF_H
#define REMAPBVF_H

#include <QtGui>

#include "abstractremapvolume.h"
#include "blockfilereader.h"

class RemapBvf : public AbstractRemapVolume
{
  Q_OBJECT

 public :
  RemapBvf();
  ~RemapBvf();

  bool setFile(QString);
  void replaceFile(QString);
  void setVoxelType(int);
  void setGridSize(int, int, int);
  void setSkipHeaderBytes(int);

  void lowresGridSize(int&, int&, int&, int&);
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
  QImage getDepthSliceLowresImage(int);
  QImage getWidthSliceLowresImage(int);
  QImage getHeightSliceLowresImage(int);

  int lowresLoD();
  void setMaxLoD(int);
  void startDepthSliceImage(int, int, int, int, int);
  void startWidthSliceImage(int, int, int, int, int);
  void startHeightSliceImage(int, int, int, int, int);

  QPair<QVariant,QVariant> rawValue(int, int, int);


  void saveTrimmed(QString,
		   int, int, int, int, int, int);

 signals :
  void getDepthSlice(int, int, int, int, int, int);
  void getWidthSlice(int, int, int, int, int, int);
  void getHeightSlice(int, int, int, int, int, int);

  void setHiresImage(QImage, int, int, int);
  void updateHistogram(QList<uint>);

 public slots :
  void depthSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);
  void widthSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);
  void heightSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);


 private :
  BlockFileReader m_bfReader;

  int m_skipBytes;
  int m_bytesPerVoxel;
  int m_currentSlice, m_currentAxis;

  uchar *m_depthSliceData;
  uchar *m_widthSliceData;
  uchar *m_heightSliceData;

  int m_maxLevel;
  int m_dstart, m_dend;
  int m_wstart, m_wend;
  int m_hstart, m_hend;

  void initializeHistogram();
};

#endif

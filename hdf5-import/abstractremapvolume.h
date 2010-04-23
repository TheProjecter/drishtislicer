#ifndef ABSTRACTREMAPVOLUME_H
#define ABSTRACTREMAPVOLUME_H

#include <QtGui>
#include "common.h"

class AbstractRemapVolume : public QObject
{
  Q_OBJECT

 public :

  virtual bool setFile(QList<QString>) = 0;
  virtual void replaceFile(QString) {}
  virtual void gridSize(int&, int&, int&) = 0;
  
  virtual void voxelSize(float& vx, float& vy, float& vz)
  {
    vx = m_voxelSizeX;
    vy = m_voxelSizeY;
    vz = m_voxelSizeZ;
  }
  virtual QString description() { return m_description; }
  virtual int voxelType() { return m_voxelType; }
  virtual int headerBytes() { return m_headerBytes; }

  virtual QList<uint> histogram() = 0;
  
  virtual void setMinMax(float, float) = 0;
  virtual float rawMin() = 0;
  virtual float rawMax() = 0;
   
  virtual void setMap(QList<float>,
		      QList<uchar>) = 0;

  QList<float> rawMap() { return m_rawMap; }
  QList<uchar> pvlMap() { return m_pvlMap; }

  virtual void getDepthSlice(int, uchar*) = 0;

  virtual QImage getDepthSliceImage(int) = 0;
  virtual QImage getWidthSliceImage(int) = 0;
  virtual QImage getHeightSliceImage(int) = 0;

  virtual QPair<QVariant,QVariant> rawValue(int, int, int) = 0;

  virtual void saveTrimmed(QString,
			   int, int, int, int, int, int) = 0;

 protected :
  QList<QString> m_fileName;
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_headerBytes;
  float m_voxelSizeX;
  float m_voxelSizeY;
  float m_voxelSizeZ;
  QString m_description;
  
  float m_rawMin, m_rawMax;
  QList<uint> m_histogram;

  QList<float> m_rawMap;
  QList<uchar> m_pvlMap;
   
  unsigned char *m_image;
};

#endif

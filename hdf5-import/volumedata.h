#ifndef VOLUMEDATA_H
#define VOLUMEDATA_H

#include "volinterface.h"

#include <QObject>

class VolumeData : public QObject
{
  Q_OBJECT

 public :
  VolumeData();
  ~VolumeData();

  bool setFile(QStringList, QString);
  void replaceFile(QString);

  void gridSize(int&, int&, int&);
  void voxelSize(float&, float&, float&);
  QString description();
  int voxelUnit();
  int voxelType();
  int headerBytes();

  QList<uint> histogram();
  
  void setMinMax(float, float);
  float rawMin();
  float rawMax();
   
  void setMap(QList<float>, QList<uchar>);

  QList<float> rawMap();
  QList<uchar> pvlMap();

  void getDepthSlice(int, uchar*);

  QImage getDepthSliceImage(int);
  QImage getWidthSliceImage(int);
  QImage getHeightSliceImage(int);

  QPair<QVariant,QVariant> rawValue(int, int, int);

  void saveTrimmed(QString, int, int, int, int, int, int);

 private :
  VolInterface *m_volInterface;

  QStringList m_fileName;
  int m_depth, m_width, m_height;
  int m_voxelUnit;
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

  int m_skipBytes;
  int m_bytesPerVoxel;

  void clear();
  bool loadPlugin(QString);
};

#endif

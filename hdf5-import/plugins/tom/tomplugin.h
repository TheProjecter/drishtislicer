#ifndef TOMPLUGIN_H
#define TOMPLUGIN_H

#include <QObject>
#include "volinterface.h"
#include "tomhead.h"

class TomPlugin : public QObject, VolInterface
{
  Q_OBJECT
  Q_INTERFACES(VolInterface)

 public :
  QStringList registerPlugin();

  void init();
  void clear();

  bool setFile(QStringList);
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
   
  void getDepthSlice(int, uchar*);
  void getWidthSlice(int, uchar*);
  void getHeightSlice(int, uchar*);

  QVariant rawValue(int, int, int);

  void saveTrimmed(QString, int, int, int, int, int, int);

  void generateHistogram();
 private :
  thead m_tHead;

  QStringList m_fileName;
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_voxelUnit;
  int m_headerBytes;
  float m_voxelSizeX;
  float m_voxelSizeY;
  float m_voxelSizeZ;
  QString m_description;
  
  float m_rawMin, m_rawMax;
  QList<uint> m_histogram;

  int m_skipBytes;
  int m_bytesPerVoxel;
};

#endif

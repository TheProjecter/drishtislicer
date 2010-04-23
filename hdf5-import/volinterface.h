#ifndef VOLINTERFACE_H
#define VOLINTERFACE_H

#include <QtCore>

class VolInterface
{
 public :
  virtual ~VolInterface() {}

  virtual void init() = 0;
  virtual void clear() = 0;

  virtual bool setFile(QStringList) = 0;
  virtual void replaceFile(QString) = 0;

  virtual void gridSize(int&, int&, int&) = 0;
  virtual void voxelSize(float&, float&, float&) = 0;
  virtual QString description() = 0;
  virtual int voxelUnit() = 0;
  virtual int voxelType() = 0;
  virtual int headerBytes() = 0;

  virtual QList<uint> histogram() = 0;
  
  virtual void setMinMax(float, float) = 0;
  virtual float rawMin() = 0;
  virtual float rawMax() = 0;
   
  virtual void setMap(QList<float>,
		      QList<uchar>) = 0;

  virtual QList<float> rawMap() = 0;
  virtual QList<uchar> pvlMap() = 0;

  virtual void getDepthSlice(int, uchar*) = 0;

  virtual QImage getDepthSliceImage(int) = 0;
  virtual QImage getWidthSliceImage(int) = 0;
  virtual QImage getHeightSliceImage(int) = 0;

  virtual QPair<QVariant,QVariant> rawValue(int, int, int) = 0;

  virtual void saveTrimmed(QString,
			   int, int, int, int, int, int) = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(VolInterface,
		    "drishti.import.Plugin.VolInterface/1.0");
QT_END_NAMESPACE

#endif

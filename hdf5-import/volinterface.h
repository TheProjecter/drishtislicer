#ifndef VOLINTERFACE_H
#define VOLINTERFACE_H

#include <QtCore>

class VolInterface
{
 public :
  virtual ~VolInterface() {}

  virtual QStringList registerPlugin() = 0;

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

  virtual void generateHistogram() = 0;

  virtual void getDepthSlice(int, uchar*) = 0;
  virtual void getWidthSlice(int, uchar*) = 0;
  virtual void getHeightSlice(int, uchar*) = 0;

  virtual QVariant rawValue(int, int, int) = 0;

  virtual void saveTrimmed(QString,
			   int, int, int, int, int, int) = 0;

};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(VolInterface,
		    "drishti.import.Plugin.VolInterface/1.0");
QT_END_NAMESPACE

#endif

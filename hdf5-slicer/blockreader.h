#ifndef BLOCKREADER_H
#define BLOCKREADER_H

#include <QtGui>
#include <QtCore>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "H5Cpp.h"
using namespace H5;

#include "common.h"

class BlockReader : public QThread
{
  Q_OBJECT

 public :
  BlockReader(QObject *parent = 0);
  ~BlockReader();

  void setMaxCacheSize(int);
  void setBlockSize(int);
  void setGridSize(int, int, int);
  void setBytesPerVoxel(int);
  void setVoxelType(int);
  void setBaseFilename(QString);
  void setMinLevel(int);
  void setBlockGridSize(int, int, int);

  void setBlockCache(int, QHash<long, uchar*> *);

  void loadBlocks(int, QVector<int>);
  void stopLoading();

 signals :
  void readDone();

 protected :
  void run();

 private :
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_minLevel;
  int m_level;
  QVector<int> m_blkno;

  int m_prevfno[10];
  QList<long> m_lru[10];
  QHash<long, uchar*> *m_blockCache[10];

  //H5File *m_hdf5file[10];
  H5File *m_hdf5file;
  DataSet m_hdf5dataset[10];
  IntType m_dataType;  

  QString m_baseFilename;
  QWaitCondition m_condition;
  QMutex m_mutex;
  bool m_interrupt;
  int m_bytesPerVoxel;
  int m_dblocks, m_wblocks, m_hblocks;
  int m_maxCacheSize;
  int m_blockSize;

  int m_bpb;

  void loadLevel(int);
  void getBlock(int, int, uchar*);
};

#endif

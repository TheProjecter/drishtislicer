#ifndef BLOCKREADER_H
#define BLOCKREADER_H

#include <QtGui>
#include <QtCore>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

class BlockReader : public QThread
{
  Q_OBJECT

 public :
  BlockReader(QObject *parent = 0);
  ~BlockReader();

  void setMaxCacheSize(int);
  void setBlockSize(int);
  void setBytesPerVoxel(int);
  void setBaseFilename(QString);
  void setBlockGridSize(int, int, int);

  void setUniform(int, QBitArray);
  void setFileBlocks(int, QVector<int>);
  void setBlockOffset(int, QList< QVector<qint64> >);
  void setBlockCache(int, QHash<long, uchar*> *);

  void loadBlocks(int, QVector<int>);
  void stopLoading();

 signals :
  void readDone();

 protected :
  void run();

 private :
  int m_level;
  QVector<int> m_blkno;

  int m_prevfno[10];
  QFile m_qfile[10];
  QList<long> m_lru[10];
  QBitArray m_uniform[10];
  QVector<int> m_fileBlocks[10];
  QList< QVector<qint64> >m_blockOffset[10];
  QHash<long, uchar*> *m_blockCache[10];


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

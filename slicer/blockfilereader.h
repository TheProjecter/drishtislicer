#ifndef BLOCKFILEREADER_H
#define BLOCKFILEREADER_H

#include <QtCore>
#include <QMutex>

#include "blockreader.h"

class BlockFileReader : public QObject
{
  Q_OBJECT

 public :
  BlockFileReader();
  ~BlockFileReader();
  
  enum VoxelType
  {
    _UChar = 0,
    _Char,
    _UShort,
    _Short,
    _Int,
    _Float
  };

  QString fileName();
  bool exists();

  QString baseFilename();
  int depth();
  int width();
  int height();

  int dblocks();
  int wblocks();
  int hblocks();

  int totalBlocks();
  qint64 maxFileSize();
  int blockSize();

  void setMaxCacheSize(int);
  int maxCacheSize();
  int minLevel();

  void getLowresGrid(int&, int&, int&, int&);

  void setBaseFilename(QString);
  void setHeaderSize(int);
  void setVoxelType(int);
  
  void setMaxFileSize(qint64);
  void setBlockSize(int);

  void setDepth(int);
  void setWidth(int);
  void setHeight(int);
  
  void startSliceBlock();
  void endSliceBlock();
  uchar* getSliceBlock(int);

  uchar* depthSlice();
  uchar* widthSlice();
  uchar* heightSlice();

  uchar* getLowresDepthSlice(int, int&, int&);
  uchar* getLowresWidthSlice(int, int&, int&);
  uchar* getLowresHeightSlice(int, int&, int&);

  bool depthBlocksPresent(int, int, int, int, int, int);
  bool widthBlocksPresent(int, int, int, int, int, int);
  bool heightBlocksPresent(int, int, int, int, int, int);

 signals :
  void depthSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);
  void widthSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);
  void heightSlice(int, int, bool,
		  QPair<int, int>, QPair<int, int>);

 public slots :
  void readDone();
  void getDepthSlice(int, int, int, int, int, int);
  void getWidthSlice(int, int, int, int, int, int);
  void getHeightSlice(int, int, int, int, int, int);
  
 private :
  BlockReader m_blockReader;

  int m_minLevel;
  int m_level, m_currentAxis, m_currentSlice;

  QString m_baseFilename;
  int m_header;
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_bytesPerVoxel;

  int m_maxCacheSize;
  bool m_dumpSlice;
  long m_maxFileSize;
  int m_blockSize;
  int m_totBlocks;
  int m_dblocks, m_wblocks, m_hblocks;
  
  int m_prevfno;
  QFile m_qfile;
  QString m_filename;

  uchar m_sslevel;
  int m_ssd, m_ssw, m_ssh;
  uchar *m_ssvol;

  uchar *m_slice;
  uchar *m_lowresSlice;

  QBitArray m_uniform[10];
  QVector<int> m_fileBlocks[10];
  QList< QVector<qint64> >m_blockOffset[10];
  QHash<long, uchar*> m_blockCache[10];

  int m_mindblocks, m_minwblocks, m_minhblocks;
  int m_maxdblocks, m_maxwblocks, m_maxhblocks;
  int m_startdblocks, m_startwblocks, m_starthblocks;
  int m_enddblocks, m_endwblocks, m_endhblocks;

  uchar *m_depthSlice;
  uchar *m_widthSlice;
  uchar *m_heightSlice;

  
  void calculateBlocksForDepthSlice(int);
  void calculateBlocksForWidthSlice(int);
  void calculateBlocksForHeightSlice(int);

  void reset();
  void clearCache();

  void dumpSliceBlocks(int);
  void saveDict();
  void loadDict();
  void getBlock(int, int, int, uchar*);

  void initializeBlockReader();
  void depthSliceDone();
  void widthSliceDone();
  void heightSliceDone();
};

#endif

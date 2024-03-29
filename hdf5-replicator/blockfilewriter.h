#ifndef BLOCKFILEWRITER_H
#define BLOCKFILEWRITER_H

#include <QtCore>


#include <iostream>
#include <string>

#include "H5Cpp.h"
using namespace H5;

class BlockFileWriter
{
 public :
  BlockFileWriter();
  ~BlockFileWriter();
  
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

  void setBaseFilename(QString);
  void setHeaderSize(int);
  void setVoxelType(int);
  
  void setMaxFileSize(qint64);
  void setBlockSize(int);

  void setDepth(int);
  void setWidth(int);
  void setHeight(int);
  void createFile(bool);

  void startAddSlice();
  void endAddSlice();  
  void setSlice(int, uchar*);

  void startSliceBlock();
  void endSliceBlock();

 private :
  QString m_baseFilename;
  int m_header;
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_bytesPerVoxel;

  int m_maxCacheSize;
  bool m_dumpSlice;
  qint64 m_maxFileSize;
  int m_blockSize;
  int m_totBlocks;
  int m_dblocks, m_wblocks, m_hblocks;
  
  int m_sslevel, m_ssd, m_ssw, m_ssh;
  uchar *m_ssvol;

  uchar *m_sliceAcc;
  uchar *m_slice;
  int m_minLevel;

  H5File *m_hdf5file;
  DataSet m_hdf5dataset[10];
  DataSet m_lowres;

  void reset();

  void saveDict();

  void dumpSliceBlocks(int, int);

  void genSliceBlocks();
  void genSliceBlocks(int);
};

#endif

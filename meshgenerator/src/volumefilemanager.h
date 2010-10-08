#ifndef VOLUMEFILEMANAGER_H
#define VOLUMEFILEMANAGER_H

#include <QStringList>
#include <QFile>

class VolumeFileManager
{
 public :
  VolumeFileManager();
  ~VolumeFileManager();
  
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

  void setFilenameList(QStringList);
  void setBaseFilename(QString);
  void setHeaderSize(int);
  void setSlabSize(int);
  void setVoxelType(int);

  void setDepth(int);
  void setWidth(int);
  void setHeight(int);
  void createFile(bool);
  
  void removeFile();

  uchar* getSlice(int);
  void setSlice(int, uchar*);
  uchar* rawValue(int, int, int);
  uchar* interpolatedRawValue(float, float, float);

 private :
  QString m_baseFilename;
  QStringList m_filenames;
  int m_header, m_slabSize;
  int m_depth, m_width, m_height;
  int m_voxelType;
  int m_bytesPerVoxel;
  uchar *m_slice;

  QFile m_qfile;
  QString m_filename;
  int m_slabno, m_prevslabno;  

  void reset();
};

#endif

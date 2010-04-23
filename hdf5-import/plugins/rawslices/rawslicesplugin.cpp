#include <QtGui>
#include "common.h"
#include "rawslicesplugin.h"
#include "loadrawdialog.h"

void
RawSlicesPlugin::init()
{
  m_fileName.clear();
  m_imageList.clear();

  m_description.clear();
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_voxelUnit = _Micron;
  m_voxelSizeX = m_voxelSizeY = m_voxelSizeZ = 1;
  m_bytesPerVoxel = 1;
  m_rawMin = m_rawMax = 0;
  m_histogram.clear();

  m_rawMap.clear();
  m_pvlMap.clear();

  m_image = 0;
}

void
RawSlicesPlugin::clear()
{
  m_fileName.clear();
  m_imageList.clear();

  m_description.clear();
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_voxelUnit = _Micron;
  m_voxelSizeX = m_voxelSizeY = m_voxelSizeZ = 1;
  m_bytesPerVoxel = 1;
  m_rawMin = m_rawMax = 0;
  m_histogram.clear();

  m_rawMap.clear();
  m_pvlMap.clear();

  if (m_image)
    delete [] m_image;
  m_image = 0;
}

void
RawSlicesPlugin::voxelSize(float& vx, float& vy, float& vz)
  {
    vx = m_voxelSizeX;
    vy = m_voxelSizeY;
    vz = m_voxelSizeZ;
  }
QString RawSlicesPlugin::description() { return m_description; }
int RawSlicesPlugin::voxelType() { return m_voxelType; }
int RawSlicesPlugin::voxelUnit() { return m_voxelUnit; }
int RawSlicesPlugin::headerBytes() { return m_headerBytes; }

void
RawSlicesPlugin::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
  
  generateHistogram();
}
float RawSlicesPlugin::rawMin() { return m_rawMin; }
float RawSlicesPlugin::rawMax() { return m_rawMax; }
QList<uint> RawSlicesPlugin::histogram() { return m_histogram; }
QList<float> RawSlicesPlugin::rawMap() { return m_rawMap; }
QList<uchar> RawSlicesPlugin::pvlMap() { return m_pvlMap; }

void
RawSlicesPlugin::setMap(QList<float> rm,
		  QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

void
RawSlicesPlugin::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

void
RawSlicesPlugin::replaceFile(QString flnm)
{
  m_fileName.clear();
  m_fileName << flnm;
}

bool
RawSlicesPlugin::setFile(QStringList files)
{
  m_fileName = files;

  m_imageList.clear();


  // list all image files in the directory
  QStringList imageNameFilter;
  imageNameFilter << "*";
  QStringList imgfiles= QDir(m_fileName[0]).entryList(imageNameFilter,
						      QDir::NoSymLinks|
						      QDir::NoDotAndDotDot|
						      QDir::Readable|
						      QDir::Files);


  m_imageList.clear();
  for(uint i=0; i<imgfiles.size(); i++)
    {
      QFileInfo fileInfo(m_fileName[0], imgfiles[i]);
      QString imgfl = fileInfo.absoluteFilePath();
      m_imageList.append(imgfl);
    }


  // --- load various parameters from the raw file ---
  LoadRawDialog loadRawDialog(0,
			      (char *)m_fileName[0].toAscii().data());
  loadRawDialog.exec();
  if (loadRawDialog.result() == QDialog::Rejected)
    return false;
  
  m_voxelType = loadRawDialog.voxelType();
  m_headerBytes = loadRawDialog.skipHeaderBytes();

  int nX, nY, nZ;
  loadRawDialog.gridSize(nX, nY, nZ);

  m_depth = m_imageList.size();
  m_width = nX;
  m_height = nY;


  m_bytesPerVoxel = 1;
  if (m_voxelType == _UChar) m_bytesPerVoxel = 1;
  else if (m_voxelType == _Char) m_bytesPerVoxel = 1;
  else if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Short) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Int) m_bytesPerVoxel = 4;
  else if (m_voxelType == _Float) m_bytesPerVoxel = 4;

  if (m_voxelType == _UChar ||
      m_voxelType == _Char ||
      m_voxelType == _UShort ||
      m_voxelType == _Short)
    {
      findMinMaxandGenerateHistogram();
    }
  else
    {
      findMinMax();
      generateHistogram();
    }

  m_rawMap.append(m_rawMin);
  m_rawMap.append(m_rawMax);
  m_pvlMap.append(0);
  m_pvlMap.append(255);

  return true;
}


#define MINMAXANDHISTOGRAM()				\
  {							\
    for(uint j=0; j<m_width*m_height; j++)		\
      {							\
	int val = ptr[j];				\
	m_rawMin = qMin(m_rawMin, (float)val);		\
	m_rawMax = qMax(m_rawMax, (float)val);		\
							\
	int idx = val-rMin;				\
	m_histogram[idx]++;				\
      }							\
  }

void
RawSlicesPlugin::findMinMaxandGenerateHistogram()
{
  float rSize;
  float rMin;
  m_histogram.clear();
  if (m_voxelType == _UChar ||
      m_voxelType == _Char)
    {
      if (m_voxelType == _UChar) rMin = 0;
      if (m_voxelType == _Char) rMin = -127;
      rSize = 255;
      for(uint i=0; i<256; i++)
	m_histogram.append(0);
    }
  else if (m_voxelType == _UShort ||
      m_voxelType == _Short)
    {
      if (m_voxelType == _UShort) rMin = 0;
      if (m_voxelType == _Short) rMin = -32767;
      rSize = 65536;
      for(uint i=0; i<65536; i++)
	m_histogram.append(0);
    }
  else
    {
      QMessageBox::information(0, "Error", "Why am i here ???");
      return;
    }

  //==================
  // do not calculate histogram
  if (m_voxelType == _UChar)
    {
      m_rawMin = 0;
      m_rawMax = 255;
      return;
    }
  //==================

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  m_rawMin = 10000000;
  m_rawMax = -10000000;

  QProgressDialog progress("Generating Histogram",
			   0,
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      //----------------------------
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes);
      fin.read((char*)tmp, nbytes);
      fin.close();
      //----------------------------

      if (m_voxelType == _UChar)
	{
	  uchar *ptr = tmp;
	  MINMAXANDHISTOGRAM();
	}
      else if (m_voxelType == _Char)
	{
	  char *ptr = (char*) tmp;
	  MINMAXANDHISTOGRAM();
	}
      if (m_voxelType == _UShort)
	{
	  ushort *ptr = (ushort*) tmp;
	  MINMAXANDHISTOGRAM();
	}
      else if (m_voxelType == _Short)
	{
	  short *ptr = (short*) tmp;
	  MINMAXANDHISTOGRAM();
	}
      else if (m_voxelType == _Int)
	{
	  int *ptr = (int*) tmp;
	  MINMAXANDHISTOGRAM();
	}
      else if (m_voxelType == _Float)
	{
	  float *ptr = (float*) tmp;
	  MINMAXANDHISTOGRAM();
	}
    }

  delete [] tmp;

  while(m_histogram.last() == 0)
    m_histogram.removeLast();

  while(m_histogram.first() == 0)
    m_histogram.removeFirst();

  progress.setValue(100);
  qApp->processEvents();
}


#define FINDMINMAX()					\
  {							\
    for(uint j=0; j<m_width*m_height; j++)		\
      {							\
	float val = ptr[j];				\
	m_rawMin = qMin(m_rawMin, val);			\
	m_rawMax = qMax(m_rawMax, val);			\
      }							\
  }

void
RawSlicesPlugin::findMinMax()
{
  QProgressDialog progress("Finding Min and Max",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_headerBytes);

  m_rawMin = 10000000;
  m_rawMax = -10000000;
  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      //----------------------------
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes);
      fin.read((char*)tmp, nbytes);
      fin.close();
      //----------------------------

      if (m_voxelType == _UChar)
	{
	  uchar *ptr = tmp;
	  FINDMINMAX();
	}
      else if (m_voxelType == _Char)
	{
	  char *ptr = (char*) tmp;
	  FINDMINMAX();
	}
      if (m_voxelType == _UShort)
	{
	  ushort *ptr = (ushort*) tmp;
	  FINDMINMAX();
	}
      else if (m_voxelType == _Short)
	{
	  short *ptr = (short*) tmp;
	  FINDMINMAX();
	}
      else if (m_voxelType == _Int)
	{
	  int *ptr = (int*) tmp;
	  FINDMINMAX();
	}
      else if (m_voxelType == _Float)
	{
	  float *ptr = (float*) tmp;
	  FINDMINMAX();
	}
    }
  fin.close();

  delete [] tmp;

  progress.setValue(100);
  qApp->processEvents();
}

#define GENHISTOGRAM()					\
  {							\
    for(uint j=0; j<m_width*m_height; j++)		\
      {							\
	float fidx = (ptr[j]-m_rawMin)/rSize;		\
	fidx = qBound(0.0f, fidx, 1.0f);		\
	int idx = fidx*histogramSize;			\
	m_histogram[idx]+=1;				\
      }							\
  }

void
RawSlicesPlugin::generateHistogram()
{
  QProgressDialog progress("Generating Histogram",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  float rSize = m_rawMax-m_rawMin;
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_headerBytes);

  m_histogram.clear();
  if (m_voxelType == _UChar ||
      m_voxelType == _Char ||
      m_voxelType == _UShort ||
      m_voxelType == _Short)
    {
      for(uint i=0; i<rSize+1; i++)
	m_histogram.append(0);
    }
  else
    {      
      for(uint i=0; i<65536; i++)
	m_histogram.append(0);
    }

  int histogramSize = m_histogram.size()-1;
  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      //----------------------------
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes);
      fin.read((char*)tmp, nbytes);
      fin.close();
      //----------------------------

      if (m_voxelType == _UChar)
	{
	  uchar *ptr = tmp;
	  GENHISTOGRAM();
	}
      else if (m_voxelType == _Char)
	{
	  char *ptr = (char*) tmp;
	  GENHISTOGRAM();
	}
      if (m_voxelType == _UShort)
	{
	  ushort *ptr = (ushort*) tmp;
	  GENHISTOGRAM();
	}
      else if (m_voxelType == _Short)
	{
	  short *ptr = (short*) tmp;
	  GENHISTOGRAM();
	}
      else if (m_voxelType == _Int)
	{
	  int *ptr = (int*) tmp;
	  GENHISTOGRAM();
	}
      else if (m_voxelType == _Float)
	{
	  float *ptr = (float*) tmp;
	  GENHISTOGRAM();
	}
    }
  fin.close();

  delete [] tmp;

  while(m_histogram.last() == 0)
    m_histogram.removeLast();
  while(m_histogram.first() == 0)
    m_histogram.removeFirst();

//  QMessageBox::information(0, "",  QString("%1 %2 : %3").\
//			   arg(m_rawMin).arg(m_rawMax).arg(rSize));

  progress.setValue(100);
  qApp->processEvents();
}

void
RawSlicesPlugin::getDepthSlice(int slc,
			      uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  QFile fin(m_imageList[slc]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_headerBytes);
  fin.read((char*)slice, nbytes);
  fin.close();
}

QImage
RawSlicesPlugin::getDepthSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;
  m_image = new uchar[m_width*m_height];

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  //----------------------------
  QFile fin(m_imageList[slc]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_headerBytes);
  fin.read((char*)tmp, nbytes);
  fin.close();
  //----------------------------

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_width*m_height; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v;

      if (m_voxelType == _UChar)
	v = ((uchar *)tmp)[i];
      else if (m_voxelType == _Char)
	v = ((char *)tmp)[i];
      else if (m_voxelType == _UShort)
	v = ((ushort *)tmp)[i];
      else if (m_voxelType == _Short)
	v = ((short *)tmp)[i];
      else if (m_voxelType == _Int)
	v = ((int *)tmp)[i];
      else if (m_voxelType == _Float)
	v = ((float *)tmp)[i];

      if (v < m_rawMap[0])
	{
	  idx = 0;
	  frc = 0;
	}
      else if (v > m_rawMap[rawSize])
	{
	  idx = rawSize-1;
	  frc = 1;
	}
      else
	{
	  for(uint m=0; m<rawSize; m++)
	    {
	      if (v >= m_rawMap[m] &&
		  v <= m_rawMap[m+1])
		{
		  idx = m;
		  frc = ((float)v-(float)m_rawMap[m])/
		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
		}
	    }
	}

      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
      m_image[i] = pv;
    }
  QImage img = QImage(m_image,
		      m_height, m_width, m_height,
		      QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RawSlicesPlugin::getWidthSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;
  m_image = new uchar[m_depth*m_height];

  int nbytes = m_depth*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  for(uint i=0; i<m_depth; i++)
    {
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes +
	       slc*m_height*m_bytesPerVoxel);
      fin.read((char*)(tmp+i*m_height*m_bytesPerVoxel),
	       m_height*m_bytesPerVoxel);
      fin.close();
    }

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_depth*m_height; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v;

      if (m_voxelType == _UChar)
	v = ((uchar *)tmp)[i];
      else if (m_voxelType == _Char)
	v = ((char *)tmp)[i];
      else if (m_voxelType == _UShort)
	v = ((ushort *)tmp)[i];
      else if (m_voxelType == _Short)
	v = ((short *)tmp)[i];
      else if (m_voxelType == _Int)
	v = ((int *)tmp)[i];
      else if (m_voxelType == _Float)
	v = ((float *)tmp)[i];

      if (v < m_rawMap[0])
	{
	  idx = 0;
	  frc = 0;
	}
      else if (v > m_rawMap[rawSize])
	{
	  idx = rawSize-1;
	  frc = 1;
	}
      else
	{
	  for(uint m=0; m<rawSize; m++)
	    {
	      if (v >= m_rawMap[m] &&
		  v <= m_rawMap[m+1])
		{
		  idx = m;
		  frc = ((float)v-(float)m_rawMap[m])/
		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
		}
	    }
	}

      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
      m_image[i] = pv;
    }
  QImage img = QImage(m_image,
		      m_height, m_depth, m_height,
		      QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RawSlicesPlugin::getHeightSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;
  m_image = new uchar[m_depth*m_width];

  int nbytes = m_depth*m_width*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  int ndum = m_width*m_height*m_bytesPerVoxel;
  uchar *dum = new uchar[ndum];  
  uint it=0;
  for(uint i=0; i<m_depth; i++)
    {
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes);
      fin.read((char*)dum, ndum);
      fin.close();

      for(uint j=0; j<m_width; j++)
	{
	  memcpy(tmp+it*m_bytesPerVoxel,
		 dum+(j*m_height+slc)*m_bytesPerVoxel,
		 m_bytesPerVoxel);
	  it++;
	}
    }
  delete [] dum;

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_depth*m_width; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v;

      if (m_voxelType == _UChar)
	v = ((uchar *)tmp)[i];
      else if (m_voxelType == _Char)
	v = ((char *)tmp)[i];
      else if (m_voxelType == _UShort)
	v = ((ushort *)tmp)[i];
      else if (m_voxelType == _Short)
	v = ((short *)tmp)[i];
      else if (m_voxelType == _Int)
	v = ((int *)tmp)[i];
      else if (m_voxelType == _Float)
	v = ((float *)tmp)[i];

      if (v < m_rawMap[0])
	{
	  idx = 0;
	  frc = 0;
	}
      else if (v > m_rawMap[rawSize])
	{
	  idx = rawSize-1;
	  frc = 1;
	}
      else
	{
	  for(uint m=0; m<rawSize; m++)
	    {
	      if (v >= m_rawMap[m] &&
		  v <= m_rawMap[m+1])
		{
		  idx = m;
		  frc = ((float)v-(float)m_rawMap[m])/
		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
		}
	    }
	}

      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
      m_image[i] = pv;
    }
  QImage img = QImage(m_image,
		      m_width, m_depth, m_width,
		      QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QPair<QVariant, QVariant>
RawSlicesPlugin::rawValue(int d, int w, int h)
{
  QPair<QVariant, QVariant> pair;

  if (d < 0 || d >= m_depth ||
      w < 0 || w >= m_width ||
      h < 0 || h >= m_height)
    {
      pair.first = QVariant("OutOfBounds");
      pair.second = QVariant("OutOfBounds");
      return pair;
    }

  QFile fin(m_imageList[d]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_headerBytes +
	   m_bytesPerVoxel*(w*m_height +h));

  QVariant v;

  if (m_voxelType == _UChar)
    {
      unsigned char a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((uint)a);
    }
  else if (m_voxelType == _Char)
    {
      char a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((int)a);
    }
  else if (m_voxelType == _UShort)
    {
      unsigned short a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((uint)a);
    }
  else if (m_voxelType == _Short)
    {
      short a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((int)a);
    }
  else if (m_voxelType == _Int)
    {
      int a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((int)a);
    }
  else if (m_voxelType == _Float)
    {
      float a;
      fin.read((char*)&a, m_bytesPerVoxel);
      v = QVariant((double)a);
    }
  fin.close();
  

  int rawSize = m_rawMap.size()-1;
  int idx = rawSize;
  float frc = 0;
  float val;

  if (v.type() == QVariant::UInt)
    val = v.toUInt();
  else if (v.type() == QVariant::Int)
    val = v.toInt();
  else if (v.type() == QVariant::Double)
    val = v.toDouble();

  if (val <= m_rawMap[0])
    {
      idx = 0;
      frc = 0;
    }
  else if (val >= m_rawMap[rawSize])
    {
      idx = rawSize-1;
      frc = 1;
    }
  else
    {
      for(uint m=0; m<rawSize; m++)
	{
	  if (val >= m_rawMap[m] &&
	      val <= m_rawMap[m+1])
	    {
	      idx = m;
	      frc = ((float)val-(float)m_rawMap[m])/
		((float)m_rawMap[m+1]-(float)m_rawMap[m]);
	    }
	}
    }
  
  uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);

  pair.first = v;
  pair.second = QVariant((uint)pv);
  return pair;
}

void
RawSlicesPlugin::saveTrimmed(QString trimFile,
			    int dmin, int dmax,
			    int wmin, int wmax,
			    int hmin, int hmax)
{
  QProgressDialog progress("Saving trimmed volume",
			   0,
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int mX, mY, mZ;
  mX = dmax-dmin+1;
  mY = wmax-wmin+1;
  mZ = hmax-hmin+1;

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  uchar vt;
  if (m_voxelType == _UChar) vt = 0; // unsigned byte
  if (m_voxelType == _Char) vt = 1; // signed byte
  if (m_voxelType == _UShort) vt = 2; // unsigned short
  if (m_voxelType == _Short) vt = 3; // signed short
  if (m_voxelType == _Int) vt = 4; // int
  if (m_voxelType == _Float) vt = 8; // float
  
  QFile fout(trimFile);
  fout.open(QFile::WriteOnly);

  fout.write((char*)&vt, 1);
  fout.write((char*)&mX, 4);
  fout.write((char*)&mY, 4);
  fout.write((char*)&mZ, 4);

  for(uint i=dmin; i<=dmax; i++)
    {
      //----------------------------
      QFile fin(m_imageList[i]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_headerBytes);
      fin.read((char*)tmp, nbytes);
      fin.close();
      //----------------------------      

      for(uint j=wmin; j<=wmax; j++)
	{
	  memcpy(tmp+(j-wmin)*mZ*m_bytesPerVoxel,
		 tmp+(j*nZ + hmin)*m_bytesPerVoxel,
		 mZ*m_bytesPerVoxel);
	}
	  
      fout.write((char*)tmp, mY*mZ*m_bytesPerVoxel);

      progress.setValue((int)(100*(float)(i-dmin)/(float)mX));
      qApp->processEvents();
    }

  fout.close();

  delete [] tmp;

  m_headerBytes = 13; // to be used for applyMapping function
}

//-------------------------------
//-------------------------------
Q_EXPORT_PLUGIN2(rawslicesplugin, RawSlicesPlugin);

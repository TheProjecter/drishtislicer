#include <QtGui>
#include "common.h"
#include "tomplugin.h"

void
TomPlugin::init()
{
  m_fileName.clear();
  m_description.clear();
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_voxelUnit = _Micron;
  m_voxelSizeX = m_voxelSizeY = m_voxelSizeZ = 1;
  m_skipBytes = 0;
  m_bytesPerVoxel = 1;
  m_rawMin = m_rawMax = 0;
  m_histogram.clear();

  m_rawMap.clear();
  m_pvlMap.clear();

  m_image = 0;
}

void
TomPlugin::clear()
{
  m_fileName.clear();
  m_description.clear();
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_voxelUnit = _Micron;
  m_voxelSizeX = m_voxelSizeY = m_voxelSizeZ = 1;
  m_skipBytes = 0;
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
TomPlugin::voxelSize(float& vx, float& vy, float& vz)
  {
    vx = m_voxelSizeX;
    vy = m_voxelSizeY;
    vz = m_voxelSizeZ;
  }
QString TomPlugin::description() { return m_description; }
int TomPlugin::voxelType() { return m_voxelType; }
int TomPlugin::voxelUnit() { return m_voxelUnit; }
int TomPlugin::headerBytes() { return m_headerBytes; }

void
TomPlugin::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
  
  generateHistogram();
}
float TomPlugin::rawMin() { return m_rawMin; }
float TomPlugin::rawMax() { return m_rawMax; }
QList<uint> TomPlugin::histogram() { return m_histogram; }
QList<float> TomPlugin::rawMap() { return m_rawMap; }
QList<uchar> TomPlugin::pvlMap() { return m_pvlMap; }

void
TomPlugin::setMap(QList<float> rm,
		  QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

void
TomPlugin::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

void
TomPlugin::replaceFile(QString flnm)
{
  m_fileName.clear();
  m_fileName << flnm;
}

bool
TomPlugin::setFile(QStringList files)
{
  m_fileName = files;

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.read((char*)&m_tHead, 512);
  fin.close();
  

  m_description = QString("%1 %2 %3 %4 %5 %6 %7").	\
    arg(m_tHead.owner).					\
    arg(m_tHead.user).					\
    arg(m_tHead.specimen).				\
    arg(m_tHead.scan).					\
    arg(m_tHead.comment).				\
    arg(m_tHead.time).					\
    arg(m_tHead.duration);

  m_voxelSizeX = m_tHead.pixel_size;
  m_voxelSizeY = m_tHead.pixel_size;
  m_voxelSizeZ = m_tHead.pixel_size;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;
  m_skipBytes = 512;
  m_headerBytes = m_skipBytes;
  m_depth = m_tHead.zsize;
  m_width = m_tHead.ysize;
  m_height = m_tHead.xsize;

  m_rawMin = 0;
  m_rawMax = 255;
  generateHistogram();

  m_rawMap.append(m_rawMin);
  m_rawMap.append(m_rawMax);
  m_pvlMap.append(0);
  m_pvlMap.append(255);

  return true;
}

#define GENHISTOGRAM()					\
  {							\
    for(uint j=0; j<nY*nZ; j++)				\
      {							\
	float fidx = (ptr[j]-m_rawMin)/rSize;		\
	fidx = qBound(0.0f, fidx, 1.0f);		\
	int idx = fidx*histogramSize;			\
	m_histogram[idx]+=1;				\
      }							\
  }

void
TomPlugin::generateHistogram()
{
  QProgressDialog progress("Generating Histogram",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  float rSize = m_rawMax-m_rawMin;

  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);

  m_histogram.clear();
  for(uint i=0; i<rSize+1; i++)
    m_histogram.append(0);

  int histogramSize = m_histogram.size()-1;
  for(uint i=0; i<nX; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)nX));
      qApp->processEvents();


      fin.read((char*)tmp, nbytes);

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

  progress.setValue(100);
  qApp->processEvents();
}

void
TomPlugin::getDepthSlice(int slc,
			      uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slc);
  fin.read((char*)slice, nbytes);
  fin.close();
}

QImage
TomPlugin::getDepthSliceImage(int slc)
{
  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nY*nZ];


  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slc);
  fin.read((char*)tmp, nbytes);
  fin.close();

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nY*nZ; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v = ((uchar *)tmp)[i];

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
  QImage img = QImage(m_image, nZ, nY, nZ, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
TomPlugin::getWidthSliceImage(int slc)
{
  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  if (slc < 0 || slc >= nY)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      return img;
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nZ];

  int nbytes = nX*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);

  for(uint k=0; k<nX; k++)
    {
      fin.seek(m_skipBytes +
	       (slc*nZ + k*nY*nZ)*m_bytesPerVoxel);

      fin.read((char*)(tmp+k*nZ*m_bytesPerVoxel),
	       nZ*m_bytesPerVoxel);

    }
  fin.close();


  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nX*nZ; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v = ((uchar *)tmp)[i];

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
  QImage img = QImage(m_image, nZ, nX, nZ, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
TomPlugin::getHeightSliceImage(int slc)
{
  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  if (slc < 0 || slc >= nZ)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      return img;
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nY];

  int nbytes = nX*nY*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);

  int ndum = nY*nZ*m_bytesPerVoxel;
  uchar *dum = new uchar[ndum];
  
  uint it=0;
  for(uint k=0; k<nX; k++)
    {
      fin.read((char*)dum, ndum);
      for(uint j=0; j<nY; j++)
	{
	  memcpy(tmp+it*m_bytesPerVoxel,
		 dum+(j*nZ+slc)*m_bytesPerVoxel,
		 m_bytesPerVoxel);
	  it++;
	}
    }
  delete [] dum;
  fin.close();


  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nX*nY; i++)
    {
      int idx = m_rawMap.size()-1;
      float frc = 0;
      float v = ((uchar *)tmp)[i];

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
  QImage img = QImage(m_image, nY, nX, nY, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QPair<QVariant, QVariant>
TomPlugin::rawValue(int d, int w, int h)
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

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes +
	   m_bytesPerVoxel*(d*m_width*m_height +
			    w*m_height +
			    h));

  unsigned char val;
  fin.read((char*)&val, m_bytesPerVoxel);
  QVariant v = QVariant((uint)val);

  fin.close();
 

  int rawSize = m_rawMap.size()-1;
  int idx = rawSize;
  float frc = 0;

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
TomPlugin::saveTrimmed(QString trimFile,
			    int dmin, int dmax,
			    int wmin, int wmax,
			    int hmin, int hmax)
{
  QProgressDialog progress("Saving trimmed volume",
			   "Cancel",
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

  uchar vt = 0; // unsigned byte
  
  QFile fout(trimFile);
  fout.open(QFile::WriteOnly);

  fout.write((char*)&vt, 1);
  fout.write((char*)&mX, 4);
  fout.write((char*)&mY, 4);
  fout.write((char*)&mZ, 4);

  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*dmin);

  for(uint i=dmin; i<=dmax; i++)
    {
      fin.read((char*)tmp, nbytes);

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

  fin.close();
  fout.close();

  delete [] tmp;

  m_headerBytes = 13; // to be used for applyMapping function
}

//-------------------------------
//-------------------------------
Q_EXPORT_PLUGIN2(tomplugin, TomPlugin);

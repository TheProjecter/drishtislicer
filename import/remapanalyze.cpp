#include "remapanalyze.h"
#include "loadrawdialog.h"
#include "staticfunctions.h"

QString RemapAnalyze::imgFile() { return m_imgFile; }

void RemapAnalyze::voxelSize(float& vx, float& vy, float& vz)
{
  vx = m_voxelSizeX;
  vy = m_voxelSizeY;
  vz = m_voxelSizeZ;
}

RemapAnalyze::RemapAnalyze()
{
  m_fileName.clear();
  m_depth = m_width = m_height = 0;
  m_skipBytes = 0;
  m_headerBytes = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;

  m_rawMin = m_rawMax = 0;
  m_histogram.clear();

  m_rawMap.clear();
  m_pvlMap.clear();

  m_image = 0;
}

RemapAnalyze::~RemapAnalyze()
{
  m_fileName.clear();
  m_depth = m_width = m_height = 0;
  m_skipBytes = 0;
  m_headerBytes = 0;
  m_voxelType = _UChar;
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
RemapAnalyze::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
  
  generateHistogram();
}

void
RemapAnalyze::setMap(QList<float> rm,
		       QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

void RemapAnalyze::setVoxelType(int vt) { m_voxelType = vt; }
void
RemapAnalyze::setSkipHeaderBytes(int b)
{
  m_skipBytes = b;
  m_headerBytes = b;
}

float RemapAnalyze::rawMin() { return m_rawMin; }
float RemapAnalyze::rawMax() { return m_rawMax; }
QList<uint> RemapAnalyze::histogram() { return m_histogram; }

void
RemapAnalyze::setGridSize(int d, int w, int h)
{
  m_depth = d;
  m_width = w;
  m_height = h;
}

void
RemapAnalyze::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

bool
RemapAnalyze::setFile(QList<QString> fl)
{
  m_fileName = fl;

  if (StaticFunctions::checkExtension(fl[0], "hdr"))
    {
      m_hdrFile = fl[0];
      m_imgFile = m_hdrFile;
      m_imgFile.chop(3);
      m_imgFile += "img";
    }
  else if (StaticFunctions::checkExtension(fl[0], "img"))
    {
      m_imgFile = fl[0];
      m_hdrFile = m_imgFile;
      m_hdrFile.chop(3);
      m_hdrFile += "hdr";
    }

  QFile fin(m_hdrFile);
  fin.open(QFile::ReadOnly);
  fin.read((char*)&(m_analyzeHeader.hk), 40);
  fin.read((char*)&(m_analyzeHeader.dime), 108);
  fin.read((char*)&(m_analyzeHeader.hist), 200);
  fin.close();

  m_byteSwap = false;
  if (m_analyzeHeader.hk.sizeof_hdr != 348)
    {
      uchar *sptr = (uchar*)(& m_analyzeHeader.hk.sizeof_hdr);
      StaticFunctions::swapbytes(sptr, 4);
      if (m_analyzeHeader.hk.sizeof_hdr != 348)
	{
	  QMessageBox::information(0, "Error",
	       QString("Header Size is not 348 : %1").arg(m_analyzeHeader.hk.sizeof_hdr));
	  return false;
	}
      else
	{
	  uchar *sptr;
	  
	  for(uint i=0; i<8; i++)
	    {
	      sptr = (uchar*) &(m_analyzeHeader.dime.dim[i]);
	      StaticFunctions::swapbytes(sptr, 2);
	    }
	  
	  for(uint i=0; i<8; i++)
	    {
	      sptr = (uchar*) &(m_analyzeHeader.dime.pixdim[i]);
	      StaticFunctions::swapbytes(sptr, 4);
	    }
	  
	  sptr = (uchar*) &(m_analyzeHeader.dime.datatype);
	  StaticFunctions::swapbytes(sptr, 2);

	  sptr = (uchar*) &(m_analyzeHeader.dime.bitpix);
	  StaticFunctions::swapbytes(sptr, 2);

	  m_byteSwap = true;
	}
      }
  if (m_analyzeHeader.dime.datatype == 0)
    {
      if (m_analyzeHeader.dime.bitpix == 8)
	m_voxelType = _UChar;
      else if (m_analyzeHeader.dime.bitpix == 16)
	m_voxelType = _Short;
      else
	{
	  QStringList dtypes;
	  dtypes << "Int"
		 << "Float";
      
	  QString option = QInputDialog::getItem(0,
						 "Data Type",
						 "Img file does not specify data type. Please choose one. ",
						 dtypes,
						 0,
						 false);
	  
	  if (option == "Int")
	    m_voxelType = _Int;
	  else if (option == "Float")
	    m_voxelType = _Float;
	}
    }
  else
    {
      if (m_analyzeHeader.dime.datatype == 2)
	m_voxelType = _UChar;
      else if (m_analyzeHeader.dime.datatype == 4)
	m_voxelType = _Short;
      else if (m_analyzeHeader.dime.datatype == 8)
	m_voxelType = _Int;
      else if (m_analyzeHeader.dime.datatype == 16)
	m_voxelType = _Float;
    }

  if (m_voxelType == _UChar)
    m_byteSwap = false;
  
  m_skipBytes = 0;
  
  m_depth = m_analyzeHeader.dime.dim[3];
  m_width = m_analyzeHeader.dime.dim[2];
  m_height = m_analyzeHeader.dime.dim[1];  

  m_voxelSizeX = m_analyzeHeader.dime.pixdim[1];
  m_voxelSizeY = m_analyzeHeader.dime.pixdim[2];
  m_voxelSizeZ = m_analyzeHeader.dime.pixdim[3];

  m_headerBytes = m_skipBytes;
  //------------------------------

//  QMessageBox::information(0, "",
//			   QString("%1 %2 %3\n%4\n%5").			\
//			   arg(m_depth).arg(m_width).arg(m_height).	\
//			   arg(m_analyzeHeader.dime.datatype).		\
//			   arg(m_imgFile));  
  
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
    for(uint j=0; j<nY*nZ; j++)				\
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
RemapAnalyze::findMinMaxandGenerateHistogram()
{
  QProgressDialog progress("Generating Histogram",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

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

  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);

  m_rawMin = 10000000;
  m_rawMax = -10000000;
  for(uint i=0; i<nX; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)nX));
      qApp->processEvents();

      fin.read((char*)tmp, nbytes);

      if (m_byteSwap && m_bytesPerVoxel > 1)
	StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

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
  fin.close();

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
    for(uint j=0; j<nY*nZ; j++)				\
      {							\
	float val = ptr[j];				\
	m_rawMin = qMin(m_rawMin, val);			\
	m_rawMax = qMax(m_rawMax, val);			\
      }							\
  }

void
RemapAnalyze::findMinMax()
{
  QProgressDialog progress("Finding Min and Max",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);

  m_rawMin = 10000000;
  m_rawMax = -10000000;
  for(uint i=0; i<nX; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)nX));
      qApp->processEvents();

      fin.read((char*)tmp, nbytes);

      if (m_byteSwap && m_bytesPerVoxel > 1)
	StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

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
    for(uint j=0; j<nY*nZ; j++)				\
      {							\
	float fidx = (ptr[j]-m_rawMin)/rSize;		\
	fidx = qBound(0.0f, fidx, 1.0f);		\
	int idx = fidx*histogramSize;			\
	m_histogram[idx]+=1;				\
      }							\
  }

void
RemapAnalyze::generateHistogram()
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

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);

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
  for(uint i=0; i<nX; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)nX));
      qApp->processEvents();

      fin.read((char*)tmp, nbytes);

      if (m_byteSwap && m_bytesPerVoxel > 1)
	StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

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
RemapAnalyze::getDepthSlice(int slc,
			    uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slc);
  fin.read((char*)slice, nbytes);
  fin.close();

  if (m_byteSwap && m_bytesPerVoxel > 1)
    StaticFunctions::swapbytes(slice, m_bytesPerVoxel, nbytes);      
}

QImage
RemapAnalyze::getDepthSliceImage(int slc)
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


  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slc);
  fin.read((char*)tmp, nbytes);
  fin.close();

  if (m_byteSwap && m_bytesPerVoxel > 1)
    StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nY*nZ; i++)
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
  QImage img = QImage(m_image, nZ, nY, nZ, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RemapAnalyze::getWidthSliceImage(int slc)
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

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);

  for(uint k=0; k<nX; k++)
    {
      fin.seek(m_skipBytes +
	       (slc*nZ + k*nY*nZ)*m_bytesPerVoxel);

      fin.read((char*)(tmp+k*nZ*m_bytesPerVoxel),
	       nZ*m_bytesPerVoxel);

    }
  fin.close();

  if (m_byteSwap && m_bytesPerVoxel > 1)
    StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nX*nZ; i++)
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
  QImage img = QImage(m_image, nZ, nX, nZ, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RemapAnalyze::getHeightSliceImage(int slc)
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

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);

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

  if (m_byteSwap && m_bytesPerVoxel > 1)
    StaticFunctions::swapbytes(tmp, m_bytesPerVoxel, nbytes);      

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nX*nY; i++)
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
  QImage img = QImage(m_image, nY, nX, nY, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QPair<QVariant, QVariant>
RemapAnalyze::rawValue(int d, int w, int h)
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

  QFile fin(m_imgFile);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes +
	   m_bytesPerVoxel*(d*m_width*m_height +
			    w*m_height +
			    h));

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
      if (m_byteSwap)
	{
	  uchar *sptr = (uchar*)(&a);
	  StaticFunctions::swapbytes(sptr, 2);
	}
      v = QVariant((uint)a);
    }
  else if (m_voxelType == _Short)
    {
      short a;
      fin.read((char*)&a, m_bytesPerVoxel);
      if (m_byteSwap)
	{
	  uchar *sptr = (uchar*)(&a);
	  StaticFunctions::swapbytes(sptr, 2);
	}
      v = QVariant((int)a);
    }
  else if (m_voxelType == _Int)
    {
      int a;
      fin.read((char*)&a, m_bytesPerVoxel);
      if (m_byteSwap)
	{
	  uchar *sptr = (uchar*)(&a);
	  StaticFunctions::swapbytes(sptr, 4);
	}
      v = QVariant((int)a);
    }
  else if (m_voxelType == _Float)
    {
      float a;
      fin.read((char*)&a, m_bytesPerVoxel);
      if (m_byteSwap)
	{
	  uchar *sptr = (uchar*)(&a);
	  StaticFunctions::swapbytes(sptr, 4);
	}
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
RemapAnalyze::saveTrimmed(QString trimFile,
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

  QFile fin(m_imgFile);
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
	  
      if (m_byteSwap && m_bytesPerVoxel > 1)
	StaticFunctions::swapbytes(tmp,
				   m_bytesPerVoxel,
				   mY*mZ*m_bytesPerVoxel);      

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

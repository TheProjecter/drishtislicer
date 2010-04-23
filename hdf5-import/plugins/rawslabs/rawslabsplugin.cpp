#include <QtGui>
#include "common.h"
#include "rawslabsplugin.h"
#include "loadrawdialog.h"

void
RawSlabsPlugin::init()
{
  m_fileName.clear();
  m_slices.clear();

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
RawSlabsPlugin::clear()
{
  m_fileName.clear();
  m_slices.clear();

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
RawSlabsPlugin::voxelSize(float& vx, float& vy, float& vz)
  {
    vx = m_voxelSizeX;
    vy = m_voxelSizeY;
    vz = m_voxelSizeZ;
  }
QString RawSlabsPlugin::description() { return m_description; }
int RawSlabsPlugin::voxelType() { return m_voxelType; }
int RawSlabsPlugin::voxelUnit() { return m_voxelUnit; }
int RawSlabsPlugin::headerBytes() { return m_headerBytes; }

void
RawSlabsPlugin::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
  
  generateHistogram();
}
float RawSlabsPlugin::rawMin() { return m_rawMin; }
float RawSlabsPlugin::rawMax() { return m_rawMax; }
QList<uint> RawSlabsPlugin::histogram() { return m_histogram; }
QList<float> RawSlabsPlugin::rawMap() { return m_rawMap; }
QList<uchar> RawSlabsPlugin::pvlMap() { return m_pvlMap; }

void
RawSlabsPlugin::setMap(QList<float> rm,
		  QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

void
RawSlabsPlugin::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

void
RawSlabsPlugin::replaceFile(QString flnm)
{
  m_fileName.clear();
  m_fileName << flnm;
}

bool
RawSlabsPlugin::setFile(QStringList files)
{
  m_fileName = files;

  {
    m_slices.clear();

    QFileInfo fi(files[0]);

    QString hdrflnm;
    hdrflnm = QFileDialog::getOpenFileName(0,
					"Load text header file",
					fi.absolutePath(),
					"Files (*.*)");

    int nfX0, nfY0, nfZ0;
    
    if (hdrflnm.isEmpty())
      {
	uchar nvt0;
	QFile fd(m_fileName[0]);
	fd.open(QFile::ReadOnly);
	fd.read((char*)&nvt0, sizeof(unsigned char));
	fd.read((char*)&nfX0, sizeof(int));
	fd.read((char*)&nfY0, sizeof(int));
	fd.read((char*)&nfZ0, sizeof(int));
	fd.close();
    
	m_voxelType = _UChar;
	if (nvt0 == 0) m_voxelType = _UChar;
	if (nvt0 == 1) m_voxelType = _Char;
	if (nvt0 == 2) m_voxelType = _UShort;
	if (nvt0 == 3) m_voxelType = _Short;
	if (nvt0 == 4) m_voxelType = _Int;
	if (nvt0 == 8) m_voxelType = _Float;

	m_headerBytes = m_skipBytes = 13;
	int nslices = nfX0;
	m_slices.append(nslices);
	
	for (int nf=1; nf<m_fileName.count(); nf++)
	  {
	    uchar nvt;
	    int nfX, nfY, nfZ;
	    QFile fd(m_fileName[nf]);
	    fd.open(QFile::ReadOnly);
	    fd.read((char*)&nvt, sizeof(unsigned char));
	    fd.read((char*)&nfX, sizeof(int));
	    fd.read((char*)&nfY, sizeof(int));
	    fd.read((char*)&nfZ, sizeof(int));
	    fd.close();
	    if (nvt != nvt0 || nfY != nfY0 || nfZ != nfZ0)
	      {
		QMessageBox::information(0, "", "Raw File format does not match");
		return false;
	      }
	    
	    nslices += nfX;
	    m_slices.append(nslices);
	  }
      }
    else
      {
	// load header information from a text file
	
	int nvt0, nvt;
	int nfX, nfY, nfZ;
	
	QFile fd(hdrflnm);
	fd.open(QFile::ReadOnly | QFile::Text);
	QTextStream in(&fd);

	QString line = (in.readLine()).simplified();
	QStringList words = line.split(" ", QString::SkipEmptyParts);
	if (words.count() >= 2)
	  {
	    int nvt0 = words[0].toInt();
	    m_voxelType = _UChar;
	    if (nvt0 == 0) m_voxelType = _UChar;
	    if (nvt0 == 1) m_voxelType = _Char;
	    if (nvt0 == 2) m_voxelType = _UShort;
	    if (nvt0 == 3) m_voxelType = _Short;
	    if (nvt0 == 4) m_voxelType = _Int;
	    if (nvt0 == 8) m_voxelType = _Float;
	    m_headerBytes = m_skipBytes = words[1].toInt();
	  }
	else
	  {
	    QMessageBox::information(0, "",
	     QString("Expecting voxeltype and headerbytes\nGot this %1").arg(line));
	    return false;
	  }

	int nslices = 0;
	while (!in.atEnd())
	  {
	    line = (in.readLine()).simplified();
	    words = line.split(" ", QString::SkipEmptyParts);
	    if (words.count() >= 3)
	      {
		nfX = words[0].toInt();
		nfY = words[1].toInt();
		nfZ = words[2].toInt();
		
		if (nslices == 0)
		  {
		    nfY0 = nfY;
		    nfZ0 = nfZ;
		  }
		else
		  {
		    if (nfY0 != nfY || nfZ0 != nfZ)
		      {
			QMessageBox::information(0, "",
			   QString("Slice size not same :: %1 %2 : %3 %4").\
			     arg(nfY0).arg(nfZ0).arg(nfY).arg(nfZ));
			return false;
		      }
		  }

		nslices += nfX;
		m_slices.append(nslices);
	      }
	    else
	      {
		QMessageBox::information(0, "",
	          QString("Expecting dimensions\nGot this %1").arg(line));
		return false;
	      }
	  }	
      }
    m_depth = m_slices[m_slices.count()-1];
    m_width = nfY0;
    m_height = nfZ0;
  }
  //------------------------------
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
RawSlabsPlugin::findMinMaxandGenerateHistogram()
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

  //==================
  // do not calculate histogram
  if (m_voxelType == _UChar)
    {
      m_rawMin = 0;
      m_rawMax = 255;
      return;
    }
  //==================


  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  for (int nf=0; nf<m_fileName.count(); nf++)
    {
      QFile fin(m_fileName[nf]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_skipBytes);
      
      m_rawMin = 10000000;
      m_rawMax = -10000000;
      int istart = ((nf > 0) ? m_slices[nf-1] : 0);
      int iend = m_slices[nf];
      for(uint i=istart; i<iend; i++)
	{
	  progress.setValue((int)(100.0*(float)i/(float)m_depth));
	  qApp->processEvents();
	  
	  fin.read((char*)tmp, nbytes);
	  
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
    for(int j=0; j<nY*nZ; j++)				\
      {							\
	float val = ptr[j];				\
	m_rawMin = qMin(m_rawMin, val);			\
	m_rawMax = qMax(m_rawMax, val);			\
      }							\
  }

void
RawSlabsPlugin::findMinMax()
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

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  for (int nf=0; nf<m_fileName.count(); nf++)
    {
      QFile fin(m_fileName[nf]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_skipBytes);

      m_rawMin = 10000000;
      m_rawMax = -10000000;
      int istart = ((nf > 0) ? m_slices[nf-1] : 0);
      int iend = m_slices[nf];

      for(int i=istart; i<iend; i++)
	{
	  progress.setValue((int)(100.0*(float)i/(float)m_depth));
	  qApp->processEvents();

	  fin.read((char*)tmp, nbytes);

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
    }
  
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
RawSlabsPlugin::generateHistogram()
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

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

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

  for (int nf=0; nf<m_fileName.count(); nf++)
    {
      QFile fin(m_fileName[nf]);
      fin.open(QFile::ReadOnly);
      fin.seek(m_skipBytes);

      int istart = ((nf > 0) ? m_slices[nf-1] : 0);
      int iend = m_slices[nf];
      for(uint i=istart; i<iend; i++)
	{
	  progress.setValue((int)(100.0*(float)i/(float)m_depth));
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
    }

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
RawSlabsPlugin::getDepthSlice(int slc,
			     uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  int fno = 0;
  for(int nf=0; nf<m_slices.count(); nf++)
    if (slc<m_slices[nf])
      {
	fno = nf;
	break;
      }
  int slcno = ((fno > 0) ? slc-m_slices[fno-1] : slc);
  QFile fin(m_fileName[fno]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slcno);
  fin.read((char*)slice, nbytes);
  fin.close();
}

QImage
RawSlabsPlugin::getDepthSliceImage(int slc)
{
  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nY*nZ];

  //-----------------------------
  int fno = 0;
  for(int nf=0; nf<m_slices.count(); nf++)
    if (slc<m_slices[nf])
      {
	fno = nf;
	break;
      }
  int slcno = ((fno > 0) ? slc-m_slices[fno-1] : slc);
  QFile fin(m_fileName[fno]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes + nbytes*slcno);
  fin.read((char*)tmp, nbytes);
  fin.close();
  //-----------------------------

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<nY*nZ; i++)
    {
      int idx = 0;
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
RawSlabsPlugin::getWidthSliceImage(int slc)
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
  int prevfno = 0;
  for(uint k=0; k<m_depth; k++)
    {
      int fno = 0;
      for(int nf=0; nf<m_slices.count(); nf++)
	if (k<m_slices[nf])
	  {
	    fno = nf;
	    break;
	  }
      if (fno != prevfno)
	{
	  fin.close();
	  fin.setFileName(m_fileName[fno]);
	  fin.open(QFile::ReadOnly);
	}
      prevfno = fno;

      int slcno = ((fno > 0) ? k-m_slices[fno-1] : k);

      fin.seek(m_skipBytes +
	       (slcno*m_width*m_height + 
		slc*m_height)*m_bytesPerVoxel);
      fin.read((char*)(tmp+k*m_height*m_bytesPerVoxel),
	       m_height*m_bytesPerVoxel);
    }
  fin.close();


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
RawSlabsPlugin::getHeightSliceImage(int slc)
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

//  QFile fin(m_fileName[0]);
//  fin.open(QFile::ReadOnly);
//  fin.seek(m_skipBytes);

  int ndum = nY*nZ*m_bytesPerVoxel;
  uchar *dum = new uchar[ndum];
  
  uint it=0;
  QFile fin(m_fileName[0]);
  fin.open(QFile::ReadOnly);
  fin.seek(m_skipBytes);
  int prevfno = 0;
  for(uint k=0; k<nX; k++)
    {
      int fno = 0;
      for(int nf=0; nf<m_slices.count(); nf++)
	if (k<m_slices[nf])
	  {
	    fno = nf;
	    break;
	  }
      if (fno != prevfno)
	{
	  fin.close();
	  fin.setFileName(m_fileName[fno]);
	  fin.open(QFile::ReadOnly);
	  fin.seek(m_skipBytes);
	}
      prevfno = fno;

      int slcno = ((fno > 0) ? k-m_slices[fno-1] : k);

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
RawSlabsPlugin::rawValue(int d, int w, int h)
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

  //-----------------------------
  int fno = 0;
  for(int nf=0; nf<m_slices.count(); nf++)
    if (d<m_slices[nf])
      {
	fno = nf;
	break;
      }
  int slcno = ((fno > 0) ? d-m_slices[fno-1] : d);
  QFile fin(m_fileName[fno]);
  fin.open(QFile::ReadOnly);
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  fin.seek(m_skipBytes + nbytes*slcno);
  fin.seek(m_skipBytes +
	   m_bytesPerVoxel*(slcno*m_width*m_height +
			    w*m_height +
			    h));
//  QFile fin(m_fileName[0]);
//  fin.open(QFile::ReadOnly);
//  fin.seek(m_skipBytes +
//	   m_bytesPerVoxel*(d*m_width*m_height +
//			    w*m_height +
//			    h));
  //-----------------------------

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
RawSlabsPlugin::saveTrimmed(QString trimFile,
			   int dmin, int dmax,
			   int wmin, int wmax,
			   int hmin, int hmax)
{
  QMessageBox::information(0, "", "deprecated");
  return;
}


//-------------------------------
//-------------------------------
Q_EXPORT_PLUGIN2(rawslabsplugin, RawSlabsPlugin);

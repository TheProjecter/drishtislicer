#include "volumefilemanager.h"
#include <QtGui>

VolumeFileManager::VolumeFileManager()
{
  m_slice = 0;
  m_filenames.clear();
  reset();
}

VolumeFileManager::~VolumeFileManager() { reset(); }

void
VolumeFileManager::reset()
{
  m_baseFilename.clear();
  m_filenames.clear();
  m_header = m_slabSize = 0;
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;

  m_filename.clear();
  m_slabno = m_prevslabno = -1;

  if (m_slice)
    delete [] m_slice;
  m_slice = 0;
}


void VolumeFileManager::setFilenameList(QStringList flist) { m_filenames = flist; }
void VolumeFileManager::setBaseFilename(QString bfn) { m_baseFilename = bfn; }
void VolumeFileManager::setDepth(int d) { m_depth = d; }
void VolumeFileManager::setWidth(int w) { m_width = w; }
void VolumeFileManager::setHeight(int h) { m_height = h; }
void VolumeFileManager::setHeaderSize(int hs) { m_header = hs; }
void VolumeFileManager::setSlabSize(int ss) { m_slabSize = ss; }
void VolumeFileManager::setVoxelType(int vt)
{
  m_voxelType = vt;
  if (m_voxelType == _UChar) m_bytesPerVoxel = 1;
  if (m_voxelType == _Char) m_bytesPerVoxel = 1;
  if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
  if (m_voxelType == _Short) m_bytesPerVoxel = 2;
  if (m_voxelType == _Int) m_bytesPerVoxel = 4;
  if (m_voxelType == _Float) m_bytesPerVoxel = 4;
}

QString VolumeFileManager::fileName() { return m_filename; }

void
VolumeFileManager::removeFile()
{
  int nslabs = m_depth/m_slabSize;
  if (nslabs*m_slabSize < m_depth) nslabs++;
  for(int ns=0; ns<nslabs; ns++)
    {
      m_filename = m_baseFilename +
	QString(".%1").arg(ns+1, 3, 10, QChar('0'));

      QFile::remove(m_filename);
    }

  reset();
}

bool
VolumeFileManager::exists()
{
  int bps = m_width*m_height*m_bytesPerVoxel;
  int nslabs = m_depth/m_slabSize;
  if (nslabs*m_slabSize < m_depth) nslabs++;

  for(int ns=0; ns<nslabs; ns++)
    {
      if (ns < m_filenames.count())
	m_filename = m_filenames[ns];
      else
	m_filename = m_baseFilename +
	  QString(".%1").arg(ns+1, 3, 10, QChar('0'));

      int nslices = qMin(m_slabSize, m_depth-ns*m_slabSize);
      qint64 fsize = nslices;
      fsize *= bps;

      m_qfile.setFileName(m_filename);

      if (m_qfile.exists() == false ||
	  m_qfile.size() != m_header+fsize)
	return false;
    }

  return true;
}

void
VolumeFileManager::createFile(bool writeHeader)
{
  int bps = m_width*m_height*m_bytesPerVoxel;
  if (!m_slice)
    m_slice = new uchar[bps];
  memset(m_slice, 0, bps);

  m_slabno = m_prevslabno = -1;
  int nslabs = m_depth/m_slabSize;
  if (nslabs*m_slabSize < m_depth) nslabs++;
  
  uchar vt;
  if (m_voxelType == _UChar) vt = 0; // unsigned byte
  if (m_voxelType == _Char) vt = 1; // signed byte
  if (m_voxelType == _UShort) vt = 2; // unsigned short
  if (m_voxelType == _Short) vt = 3; // signed short
  if (m_voxelType == _Int) vt = 4; // int
  if (m_voxelType == _Float) vt = 8; // float

  QProgressDialog progress(QString("Allocating space for\n%1\non disk").\
			   arg(m_baseFilename),
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  for(int ns=0; ns<nslabs; ns++)
    {
      m_filename = m_baseFilename +
	QString(".%1").arg(ns+1, 3, 10, QChar('0'));

      progress.setLabelText(m_filename);
      qApp->processEvents();

      m_qfile.setFileName(m_filename);
      m_qfile.open(QFile::WriteOnly);

      int nslices = qMin(m_slabSize, m_depth-ns*m_slabSize);      
      if (writeHeader)
	{
	  m_qfile.write((char*)&vt, 1);
	  m_qfile.write((char*)&nslices, 4);
	  m_qfile.write((char*)&m_width, 4);
	  m_qfile.write((char*)&m_height, 4);
	}

      for(int t=0; t<nslices; t++)
	{
	  progress.setValue((int)(100*(float)t/(float)nslices));
	  qApp->processEvents();
	  m_qfile.write((char*)m_slice, bps);
	}

      m_qfile.close();
    }

  progress.setValue(100);
}

uchar*
VolumeFileManager::getSlice(int d)
{
  int bps = m_width*m_height*m_bytesPerVoxel;
  if (!m_slice)
    m_slice = new uchar[bps];

  m_slabno = d/m_slabSize;

  if (m_slabno < m_filenames.count())
    m_filename = m_filenames[m_slabno];
  else
    m_filename = m_baseFilename +
	         QString(".%1").arg(m_slabno+1, 3, 10, QChar('0'));
  m_qfile.setFileName(m_filename);
  m_qfile.open(QFile::ReadOnly);
  m_qfile.seek(m_header + (d-m_slabno*m_slabSize)*bps);
  m_qfile.read((char*)m_slice, bps);
  m_qfile.close();

  return m_slice;
}

void
VolumeFileManager::setSlice(int d, uchar *tmp)
{
  int bps = m_width*m_height*m_bytesPerVoxel;
  m_slabno = d/m_slabSize;
  if (m_slabno < m_filenames.count())
    m_filename = m_filenames[m_slabno];
  else
    m_filename = m_baseFilename +
                 QString(".%1").arg(m_slabno+1, 3, 10, QChar('0'));
  m_qfile.setFileName(m_filename);
  m_qfile.open(QFile::ReadWrite);
  m_qfile.seek(m_header + (d-m_slabno*m_slabSize)*bps);
  m_qfile.write((char*)tmp, bps);
  m_qfile.close();
}

uchar*
VolumeFileManager::rawValue(int d, int w, int h)
{
  if (d < 0 || d >= m_depth ||
      w < 0 || w >= m_width ||
      h < 0 || h >= m_height)
    return 0;

  int bps = m_width*m_height*m_bytesPerVoxel;
  if (!m_slice)
    m_slice = new uchar[bps];

  // at most we will be reading an 8 byte value
  // initialize first 8 bytes to 0
  memset(m_slice, 0, 8);

  m_slabno = d/m_slabSize;
  if (m_slabno < m_filenames.count())
    m_filename = m_filenames[m_slabno];
  else
    m_filename = m_baseFilename +
	         QString(".%1").arg(m_slabno+1, 3, 10, QChar('0'));
  m_qfile.setFileName(m_filename);
  m_qfile.open(QFile::ReadOnly);
  m_qfile.seek(m_header +
	       (d-m_slabno*m_slabSize)*bps +
	       (w*m_height + h)*m_bytesPerVoxel);
  m_qfile.read((char*)m_slice, m_bytesPerVoxel);
  m_qfile.close();
  return m_slice;
}

#define interpVal(T)					\
  T *v[8];						\
  for(int i=0; i<8; i++)				\
    v[i] = (T*)(rv + i*m_bytesPerVoxel);		\
							\
  T vb = ((1-dd)*(1-ww)*(1-hh)*(*v[0]) +		\
	  (1-dd)*(1-ww)*(  hh)*(*v[1]) +		\
	  (1-dd)*(  ww)*(1-hh)*(*v[2]) +		\
	  (1-dd)*(  ww)*(  hh)*(*v[3]) +		\
	  (  dd)*(1-ww)*(1-hh)*(*v[4]) +		\
	  (  dd)*(1-ww)*(  hh)*(*v[5]) +		\
	  (  dd)*(  ww)*(1-hh)*(*v[6]) +		\
	  (  dd)*(  ww)*(  hh)*(*v[7]));		\
  memcpy(m_slice, &vb, sizeof(T));


uchar*
VolumeFileManager::interpolatedRawValue(float dv, float wv, float hv)
{
  int d = dv;
  int w = wv;
  int h = hv;
  int d1 = d+1;
  int w1 = w+1;
  int h1 = h+1;
  float dd = dv-d;
  float ww = wv-w;
  float hh = hv-h;
  
  int bps = m_width*m_height*m_bytesPerVoxel;
  if (!m_slice)
    m_slice = new uchar[bps];

  // at most we will be reading an 8 byte value
  // initialize first 8 bytes to 0
  memset(m_slice, 0, 8);

  if (d < 0 || d1 >= m_depth ||
      w < 0 || w1 >= m_width ||
      h < 0 || h1 >= m_height)
    return m_slice;

  int da[8], wa[8], ha[8];
  da[0]=d;  wa[0]=w;  ha[0]=h;
  da[1]=d;  wa[1]=w;  ha[1]=h1;
  da[2]=d;  wa[2]=w1; ha[2]=h;
  da[3]=d;  wa[3]=w1; ha[3]=h1;
  da[4]=d1; wa[4]=w;  ha[4]=h;
  da[5]=d1; wa[5]=w;  ha[5]=h1;
  da[6]=d1; wa[6]=w1; ha[6]=h;
  da[7]=d1; wa[7]=w1; ha[7]=h1;

  uchar *rv = new uchar[8*m_bytesPerVoxel];

  int pslno = -1;
  for(int i=0; i<8; i++)
    {
      m_slabno = da[i]/m_slabSize;
      if (m_slabno != pslno)
	{
	  if (m_qfile.isOpen())
	    m_qfile.close();

	  if (m_slabno < m_filenames.count())
	    m_filename = m_filenames[m_slabno];
	  else
	    m_filename = m_baseFilename +
	      QString(".%1").arg(m_slabno+1, 3, 10, QChar('0'));

	  m_qfile.setFileName(m_filename);
	  m_qfile.open(QFile::ReadOnly);
	}

      m_qfile.seek(m_header +
		   (da[i]-m_slabno*m_slabSize)*bps +
		   (wa[i]*m_height + ha[i])*m_bytesPerVoxel);
      m_qfile.read((char*)(rv+i*m_bytesPerVoxel), m_bytesPerVoxel);

      pslno = m_slabno;
    }
  m_qfile.close();
  
  if (m_voxelType == _UChar)
    {
      interpVal(uchar);
    }
  else if (m_voxelType == _Char)
    {
      interpVal(char);
    }
  else if (m_voxelType == _UShort)
    {
      interpVal(ushort);
    }
  else if (m_voxelType == _Short)
    {
      interpVal(short);
    }
  else if (m_voxelType == _Int)
    {
      interpVal(int);
    }
  else if (m_voxelType == _Float)
    {
      interpVal(float);
    }
  
  delete [] rv;

  return m_slice;
}

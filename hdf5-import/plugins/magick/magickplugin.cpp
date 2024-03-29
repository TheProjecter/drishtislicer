#include <QtGui>
#include "common.h"
#include "magickplugin.h"

#include <Magick++.h>
using namespace std;
using namespace Magick;

void MagickPlugin::generateHistogram() {} // to satisfy the interface

QStringList
MagickPlugin::registerPlugin()
{
  QStringList regString;
  regString << "directory";
  regString << "DICOM/16bit Image Directory";
  regString << "files";
  regString << "DICOM/16bit Image Files";
  
  return regString;
}

void
MagickPlugin::init()
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
}

void
MagickPlugin::clear()
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
}

void
MagickPlugin::voxelSize(float& vx, float& vy, float& vz)
  {
    vx = m_voxelSizeX;
    vy = m_voxelSizeY;
    vz = m_voxelSizeZ;
  }
QString MagickPlugin::description() { return m_description; }
int MagickPlugin::voxelType() { return m_voxelType; }
int MagickPlugin::voxelUnit() { return m_voxelUnit; }
int MagickPlugin::headerBytes() { return m_headerBytes; }

void
MagickPlugin::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
}
float MagickPlugin::rawMin() { return m_rawMin; }
float MagickPlugin::rawMax() { return m_rawMax; }
QList<uint> MagickPlugin::histogram() { return m_histogram; }

void
MagickPlugin::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

// ---- class for sorting DICOM images ----
class tag
{
 public :
  int id;
  float loc;
  tag(int i, float l)
  {
    id = i;
    loc= l;
  };
  bool operator<(const tag& a) const
  {
    return (loc < a.loc);
  };
  tag& operator=(const tag& a)
  {
    id = a.id;
    loc = a.loc;
    return *this;
  };
};

void
MagickPlugin::replaceFile(QString flnm)
{
  m_fileName.clear();
  m_fileName << flnm;
}

void
MagickPlugin::setImageFiles(QStringList files)
{
  QProgressDialog progress("Enumerating files - may take some time...",
			   0,
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  // if loading dicom files sort these on SliceLocation tag
  QList <tag> dcmtags;
  
  QStringList flist;
  flist.clear();
  
  for(uint i=0; i<files.size(); i++)
    {
      progress.setValue(100*(float)i/(float)files.size());
      qApp->processEvents();
      
      QFileInfo fileInfo(m_fileName[0], files[i]);
      QString imgfl = fileInfo.absoluteFilePath();

      Image image;
      image.defineSet("dcm", "display-range");
      image.defineValue("dcm", "display-range", "reset");
      image.read((char*)imgfl.toAscii().data());
      QString str = QString::fromStdString(image.attribute("Dcm:SliceLocation"));

      int voxdepth = image.depth();
            
      if (!str.isEmpty())
	{
	  flist.append(imgfl);
	  dcmtags.append(tag(i, str.toFloat()));
	  progress.setLabelText(QString("Slice Location %1 (bits per pixel : %2)"). \
				arg(str).				\
				arg(voxdepth));
	}
      else
	m_imageList.append(imgfl);
    }
  
  if (flist.size() > 0)
    {
      progress.setValue(50);
      progress.setLabelText(QString("Sorting %1 slices").arg(flist.size()));
      qApp->processEvents();
      
      qSort(dcmtags);
      
      m_imageList.clear();
      for(uint i=0; i<flist.size(); i++)
	m_imageList.append(flist[dcmtags[i].id]);
    }

  progress.setValue(100);
  qApp->processEvents();

  m_depth = m_imageList.size();
  Image image((char*)m_imageList[0].toAscii().data());
  m_width = image.columns();
  m_height = image.rows();

  if (image.depth() == 8)
    m_voxelType = _UChar;
  else if (image.depth() == 16)
    m_voxelType = _UShort;
  else if (image.depth() == 32)
    m_voxelType = _Float;

  m_headerBytes = 0;

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
    findMinMaxandGenerateHistogram();
  else
    QMessageBox::information(0, "Error",
			       "Currently accepting only 1- and 2-byte images");
}

bool
MagickPlugin::setFile(QStringList files)
{
  if (files.size() == 0)
    return false;

  m_fileName = files;

  m_imageList.clear();

  QFileInfo f(m_fileName[0]);
  if (f.isDir())
    {
      // list all image files in the directory
      QStringList imageNameFilter;
      imageNameFilter << "*";
      QStringList imgfiles= QDir(m_fileName[0]).entryList(imageNameFilter,
							  QDir::NoSymLinks|
							  QDir::NoDotAndDotDot|
							  QDir::Readable|
							  QDir::Files);
      
      setImageFiles(imgfiles);
    }
  else
    setImageFiles(files);

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
MagickPlugin::findMinMaxandGenerateHistogram()
{
  QProgressDialog progress("Generating Histogram",
			   0,
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

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  m_rawMin = 10000000;
  m_rawMax = -10000000;

  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      Image img;
      img.defineSet("dcm", "display-range");
      img.defineValue("dcm", "display-range", "reset");
      img.read((char*)m_imageList[i].toAscii().data());
      StorageType storageType = CharPixel;
      if (m_voxelType == _UShort) storageType = ShortPixel;
      if (m_voxelType == _Float) storageType = FloatPixel;
      img.write(0, 0,
		img.columns(), img.rows(),
		"I",
		storageType,
		tmp);

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

void
MagickPlugin::getDepthSlice(int slc,
			    uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;

  uchar *tmp1 = new uchar[nbytes];

  Image imgL;
  imgL.defineSet("dcm", "display-range");
  imgL.defineValue("dcm", "display-range", "reset");
  imgL.read((char*)m_imageList[slc].toAscii().data());

  StorageType storageType = CharPixel;
  if (m_voxelType == _UShort) storageType = ShortPixel;
  if (m_voxelType == _Float) storageType = FloatPixel;
  imgL.write(0, 0,
	     imgL.columns(), imgL.rows(),
	     "I",
	     storageType,
	     tmp1);

  if (m_voxelType == _UChar)
    {
      for(uint j=0; j<m_width; j++)
	for(uint k=0; k<m_height; k++)
	  slice[j*m_height+k] = tmp1[k*m_width+j];
    }
  else if (m_voxelType == _UShort)
    {
      ushort *p0 = (ushort*)slice;
      ushort *p1 = (ushort*)tmp1;
      for(uint j=0; j<m_width; j++)
	for(uint k=0; k<m_height; k++)
	  p0[j*m_height+k] = p1[k*m_width+j];
    }
  else if (m_voxelType == _Float)
    {
      float *p0 = (float*)slice;
      float *p1 = (float*)tmp1;
      for(uint j=0; j<m_width; j++)
	for(uint k=0; k<m_height; k++)
	  p0[j*m_height+k] = p1[k*m_width+j];
    }
  delete [] tmp1;
}

void
MagickPlugin::getWidthSlice(int slc,
			    uchar *slice)
{
  QProgressDialog progress("Extracting Slice",
			   0,
			   0, 100,
			   0);
  progress.setMinimumDuration(0);
  uchar *imgSlice = new uchar[m_width*m_height*m_bytesPerVoxel];
  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      Image imgL;
      imgL.defineSet("dcm", "display-range");
      imgL.defineValue("dcm", "display-range", "reset");
      imgL.read((char*)m_imageList[i].toAscii().data());

      StorageType storageType = CharPixel;
      if (m_voxelType == _UShort) storageType = ShortPixel;
      if (m_voxelType == _Float) storageType = FloatPixel;
      imgL.write(slc, 0,
		 1, imgL.rows(),
		 "I",
		 storageType,
		 imgSlice);



      if (m_voxelType == _UChar)
	{
	  for(uint j=0; j<m_height; j++)
	    slice[i*m_height+j] = imgSlice[j];
	}
      else if (m_voxelType == _UShort)
	{
	  ushort *p0 = (ushort*)slice;
	  ushort *p1 = (ushort*)imgSlice;
	  for(uint j=0; j<m_height; j++)
	    p0[i*m_height+j] = p1[j];
	}
      else if (m_voxelType == _Float)
	{
	  float *p0 = (float*)slice;
	  float *p1 = (float*)imgSlice;
	  for(uint j=0; j<m_height; j++)
	    p0[i*m_height+j] = p1[j];
	}
    }
  delete [] imgSlice;
  progress.setValue(100);
  qApp->processEvents();
}

void
MagickPlugin::getHeightSlice(int slc,
			     uchar *slice)
{
  uchar *imgSlice = new uchar[m_width*m_height*m_bytesPerVoxel];
  QProgressDialog progress("Extracting Slice",
			   0,
			   0, 100,
			   0);
  progress.setMinimumDuration(0);
  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      Image imgL;
      imgL.defineSet("dcm", "display-range");
      imgL.defineValue("dcm", "display-range", "reset");
      imgL.read((char*)m_imageList[i].toAscii().data());

      StorageType storageType = CharPixel;
      if (m_voxelType == _UShort) storageType = ShortPixel;
      if (m_voxelType == _Float) storageType = FloatPixel;
      imgL.write(0, slc,
		 imgL.columns(), 1,
		 "I",
		 storageType,
		 imgSlice);


      if (m_voxelType == _UChar)
	{
	  for(uint j=0; j<m_width; j++)
	    slice[i*m_width+j] = imgSlice[j];
	}
      else if (m_voxelType == _UShort)
	{
	  ushort *p0 = (ushort*)slice;
	  ushort *p1 = (ushort*)imgSlice;
	  for(uint j=0; j<m_width; j++)
	    p0[i*m_width+j] = p1[j];
	}
      else if (m_voxelType == _Float)
	{
	  float *p0 = (float*)slice;
	  float *p1 = (float*)imgSlice;
	  for(uint j=0; j<m_width; j++)
	    p0[i*m_width+j] = p1[j];
	}
    }
  delete [] imgSlice;
  progress.setValue(100);
  qApp->processEvents();
}

QVariant
MagickPlugin::rawValue(int d, int w, int h)
{
  QVariant v;

  if (d < 0 || d >= m_depth ||
      w < 0 || w >= m_width ||
      h < 0 || h >= m_height)
    {
      v = QVariant("OutOfBounds");
      return v;
    }

  uchar *tmp = new uchar[10];

  Image imgL;
  imgL.defineSet("dcm", "display-range");
  imgL.defineValue("dcm", "display-range", "reset");
  imgL.read((char*)m_imageList[d].toAscii().data());

  StorageType storageType = CharPixel;
  if (m_voxelType == _UShort) storageType = ShortPixel;
  if (m_voxelType == _Float) storageType = FloatPixel;
  imgL.crop(Geometry(1, 1, w, h)); // width, height. xoffset, yoffset
  imgL.write(0, 0,
	     1, 1,
	     "I",
	     storageType,
	     tmp);
  
  if (m_voxelType == _UChar)
    {
      uchar a = *tmp;
      v = QVariant((uint)a);
    }
  else if (m_voxelType == _UShort)
    {
      ushort a = *(ushort*)tmp;
      v = QVariant((uint)a);
    }
  else if (m_voxelType == _Float)
    {
      float a = *(float*)tmp;
      v = QVariant((double)a);
    }

  return v;
}

void
MagickPlugin::saveTrimmed(QString trimFile,
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
  uchar *tmp1 = new uchar[nbytes];

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

  //for(uint i=dmin; i<=dmax; i++)
  for(int i=dmax; i>=dmin; i--)
    {
      Image imgL;
      imgL.defineSet("dcm", "display-range");
      imgL.defineValue("dcm", "display-range", "reset");
      imgL.read((char*)m_imageList[i].toAscii().data());

      StorageType storageType = CharPixel;
      if (m_voxelType == _UShort) storageType = ShortPixel;
      if (m_voxelType == _Float) storageType = FloatPixel;
      imgL.write(0, 0,
		 imgL.columns(), imgL.rows(),
		 "I",
		 storageType,
		 tmp1);
      
      if (m_voxelType == _UChar)
	{
	  for(uint j=0; j<m_width; j++)
	    for(uint k=0; k<m_height; k++)
	      tmp[j*m_height+k] = tmp1[k*m_width+j];
	}
      else if (m_voxelType == _UShort)
	{
	  ushort *p0 = (ushort*)tmp;
	  ushort *p1 = (ushort*)tmp1;
	  for(uint j=0; j<m_width; j++)
	    for(uint k=0; k<m_height; k++)
	      p0[j*m_height+k] = p1[k*m_width+j];
	}
      else if (m_voxelType == _Float)
	{
	  float *p0 = (float*)tmp;
	  float *p1 = (float*)tmp1;
	  for(uint j=0; j<m_width; j++)
	    for(uint k=0; k<m_height; k++)
	      p0[j*m_height+k] = p1[k*m_width+j];
	}
      

      for(uint j=wmin; j<=wmax; j++)
	{
	  memcpy(tmp+(j-wmin)*mZ*m_bytesPerVoxel,
		 tmp+(j*nZ + hmin)*m_bytesPerVoxel,
		 mZ*m_bytesPerVoxel);
	}
	  
      fout.write((char*)tmp, mY*mZ*m_bytesPerVoxel);

      progress.setValue((int)(100*(float)(dmax-i)/(float)mX));
      qApp->processEvents();
    }

  fout.close();

  delete [] tmp;
  delete [] tmp1;

  progress.setValue(100);

  m_headerBytes = 13; // to be used for applyMapping function
}

//-------------------------------
//-------------------------------
Q_EXPORT_PLUGIN2(magickplugin, MagickPlugin);

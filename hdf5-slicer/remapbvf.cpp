#include <QtXml>

#include <math.h>

#include "remapbvf.h"

#include <cmath>

#ifdef Q_WS_WIN
#include <float.h>
#define ISNAN(v) _isnan(v)
#else
#ifdef Q_WS_MAC
static int isnan(float x)
{
  return x != x;
}
#define ISNAN(v) isnan(v)
#else
#define ISNAN(v) isnan(v)
#endif // __apple__
#endif


RemapBvf::RemapBvf()
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
  
  m_maxLevel = 0;
  m_depthSliceData = 0;
  m_widthSliceData = 0;
  m_heightSliceData = 0;
}

RemapBvf::~RemapBvf()
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

  if (m_depthSliceData)
    delete m_depthSliceData;
  m_depthSliceData = 0;

  if (m_widthSliceData)
    delete m_widthSliceData;
  m_widthSliceData = 0;

  if (m_heightSliceData)
    delete m_heightSliceData;
  m_heightSliceData = 0;
}

void RemapBvf::setMaxLoD(int l) { m_maxLevel = l; }
int RemapBvf::lowresLoD() { return m_bfReader.minLevel(); }

void
RemapBvf::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
}

void
RemapBvf::setMap(QList<float> rm,
		 QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

void RemapBvf::setVoxelType(int vt) { m_voxelType = vt; }
void
RemapBvf::setSkipHeaderBytes(int b)
{
  m_skipBytes = b;
  m_headerBytes = b;
}

float RemapBvf::rawMin() { return m_rawMin; }
float RemapBvf::rawMax() { return m_rawMax; }
QList<uint> RemapBvf::histogram() { return m_histogram; }

void
RemapBvf::setGridSize(int d, int w, int h)
{
  m_depth = d;
  m_width = w;
  m_height = h;
}

void
RemapBvf::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

void
RemapBvf::replaceFile(QString flnm)
{
  m_fileName = flnm;
}

bool
RemapBvf::setFile(QString fl)
{
  m_fileName = fl;

  long maxFileSize;
  int blockSize;

  QDomDocument document;
  QFile f(m_fileName.toAscii().data());
  if (f.open(QIODevice::ReadOnly))
    {
      document.setContent(&f);
      f.close();
    }
  
  QDomElement main = document.documentElement();
  QDomNodeList dlist = main.childNodes();
  for(int i=0; i<dlist.count(); i++)
    {
      if (dlist.at(i).nodeName() == "description")
	{
	  m_description = dlist.at(i).toElement().text();
	}
      else if (dlist.at(i).nodeName() == "voxeltype")
	{
	  QString pvalue = dlist.at(i).toElement().text();
	  if (pvalue == "unsigned char")
	    m_voxelType = _UChar;
	  else if (pvalue == "char")
	    m_voxelType = _Char;
	  else if (pvalue == "unsigned short")
	    m_voxelType = _UShort;
	  else if (pvalue == "short")
	    m_voxelType = _Short;
	  else if (pvalue == "int")
	    m_voxelType = _Int;
	  else if (pvalue == "float")
	    m_voxelType = _Float;
	  else if (pvalue == "rgb")
	    m_voxelType = _Rgb;
	  else if (pvalue == "rgba")
	    m_voxelType = _Rgba;
	}
      else if (dlist.at(i).nodeName() == "voxelsize")
	{
	  QStringList str = (dlist.at(i).toElement().text()).split(" ", QString::SkipEmptyParts);
	  m_voxelSizeX = str[0].toFloat();
	  m_voxelSizeY = str[1].toFloat();
	  m_voxelSizeZ = str[2].toFloat();
	}
      else if (dlist.at(i).nodeName() == "gridsize")
	{
	  QStringList str = (dlist.at(i).toElement().text()).split(" ", QString::SkipEmptyParts);
	  m_depth = str[0].toInt();
	  m_width = str[1].toInt();
	  m_height = str[2].toInt();
	}
      else if (dlist.at(i).nodeName() == "maxfilesize")
	{
	  maxFileSize = (dlist.at(i).toElement().text()).toLong();
	}
      else if (dlist.at(i).nodeName() == "blocksize")
	{
	  blockSize = (dlist.at(i).toElement().text()).toInt();
	}
      else if (dlist.at(i).nodeName() == "rawmap")
	{
	  QStringList str = (dlist.at(i).toElement().text()).split(" ", QString::SkipEmptyParts);
	  for(int is=0; is<str.count(); is++)
	    m_rawMap << str[is].toFloat();
	}
      else if (dlist.at(i).nodeName() == "pvlmap")
	{
	  QStringList str = (dlist.at(i).toElement().text()).split(" ", QString::SkipEmptyParts);
	  for(int is=0; is<str.count(); is++)
	    m_pvlMap << str[is].toFloat();
	}
    }

  m_headerBytes = m_skipBytes = 0;

//  // -- override if we are dealing with mapped file - .bvf
//  if (m_fileName.right(6) != "rawbvf")
//    m_voxelType = _UChar;

  m_bytesPerVoxel = 1;
  if (m_voxelType == _UChar) m_bytesPerVoxel = 1;
  else if (m_voxelType == _Char) m_bytesPerVoxel = 1;
  else if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Short) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Int) m_bytesPerVoxel = 4;
  else if (m_voxelType == _Float) m_bytesPerVoxel = 4;
  else if (m_voxelType == _Rgb) m_bytesPerVoxel = 3;
  else if (m_voxelType == _Rgba) m_bytesPerVoxel = 4;

  m_currentSlice = m_currentAxis = -1;

  m_bfReader.setBaseFilename(m_fileName);
  m_bfReader.setDepth(m_depth);
  m_bfReader.setWidth(m_width);
  m_bfReader.setHeight(m_height);
  m_bfReader.setVoxelType(m_voxelType);
  m_bfReader.setHeaderSize(m_headerBytes);
  m_bfReader.setBlockSize(blockSize);
  m_bfReader.setMaxFileSize(maxFileSize);

  connect(this, SIGNAL(getDepthSlice(int, int, int,int, int,int)),
	  &m_bfReader, SLOT(getDepthSlice(int, int, int,int, int,int)));
  connect(this, SIGNAL(getWidthSlice(int, int, int,int, int,int)),
	  &m_bfReader, SLOT(getWidthSlice(int, int, int,int, int,int)));
  connect(this, SIGNAL(getHeightSlice(int, int, int,int, int,int)),
	  &m_bfReader, SLOT(getHeightSlice(int, int, int,int, int,int)));

  connect(&m_bfReader, SIGNAL(depthSlice(int, int, bool,
				QPair<int, int>, QPair<int, int>)),
	  this, SLOT(depthSlice(int, int, bool,
				QPair<int, int>, QPair<int, int>)),
  	  Qt::DirectConnection);

  connect(&m_bfReader, SIGNAL(widthSlice(int, int, bool,
			        QPair<int, int>, QPair<int, int>)),
	  this, SLOT(widthSlice(int, int, bool,
				QPair<int, int>, QPair<int, int>)),
	  Qt::DirectConnection);
  
  connect(&m_bfReader, SIGNAL(heightSlice(int, int, bool,
				QPair<int, int>, QPair<int, int>)),
	  this, SLOT(heightSlice(int, int, bool,
				QPair<int, int>, QPair<int, int>)),
	  Qt::DirectConnection);

  if (! m_bfReader.exists())
    {
      QMessageBox::critical(0, "Error", "File sizes do not match");
      return false;
    }

  initializeHistogram();

  m_rawMap.append(m_rawMin);
  m_rawMap.append(m_rawMax);
  m_pvlMap.append(0);
  m_pvlMap.append(255);


  if (m_voxelType != _Rgb && m_voxelType != _Rgba)
    {
      m_depthSliceData = new uchar[m_height*m_width];
      m_widthSliceData = new uchar[m_depth*m_height];
      m_heightSliceData = new uchar[m_depth*m_width];
    }
  else
    {
      m_depthSliceData = new uchar[4*m_height*m_width];
      m_widthSliceData = new uchar[4*m_depth*m_height];
      m_heightSliceData = new uchar[4*m_depth*m_width];
    }

  return true;
}

void
RemapBvf::initializeHistogram()
{
  float rSize;
  float rMin;
  m_histogram.clear();
  if (m_voxelType == _UChar ||
      m_voxelType == _Char ||
      m_voxelType == _Rgb ||
      m_voxelType == _Rgba)
    {
      if (m_voxelType == _UChar) m_rawMin = 0;
      if (m_voxelType == _Char) m_rawMin = -127;
      for(int i=0; i<256; i++)
	m_histogram.append(0);
      m_rawMax = m_rawMin + 255;
    }
  else if (m_voxelType == _UShort ||
	   m_voxelType == _Short)
    {
      m_rawMin = 0;
      if (m_voxelType == _UShort) m_rawMin = 0;
      if (m_voxelType == _Short) m_rawMin = -32767;
      for(int i=0; i<65536; i++)
	m_histogram.append(0);
      m_rawMax = m_rawMin + 65535;
    }
  else
    {
      if (m_rawMap.count() > 0)
	{
	  m_rawMin = m_rawMap[0];
	  m_rawMax = m_rawMap[m_rawMap.count()-1];
	}
      else
	{
	  m_rawMin = 0;
	  m_rawMax = 1000;
	}
      for(int i=0; i<65536; i++)
	m_histogram.append(0);
    }
}

void
RemapBvf::getDepthSlice(int slc,
			uchar *slice)
{
  int nbytes = m_width*m_height*m_bytesPerVoxel;
  //memcpy(slice, m_bfReader.getDepthSlice(slc), nbytes);
}

void
RemapBvf::depthSlice(int slc, int level, bool partlyRead,
		      QPair<int, int> wlimits, QPair<int, int> hlimits)
{
  if (m_currentAxis != 0 ||
      m_currentSlice != slc)
    return;

  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    {
      depthSliceRgb(slc, level, partlyRead,
		    wlimits, hlimits);
      return;
    }


  int wmin = wlimits.first;
  int wmax = wlimits.second;
  int hmin = hlimits.first;
  int hmax = hlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth;
  int nY = m_width/p2;
  int nZ = m_height/p2;

  if (slc < 0 || slc >= nX)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
      return;
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nY*nZ];

  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.depthSlice(), nbytes);

  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int w=wmin; w<=wmax; w++)
    for(int h=hmin; h<=hmax; h++)
      {
	int i = w*nZ + h;
	
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
	
	float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);
	
	uchar pv = 255*rv;
	m_image[i] = pv;
	
	int idx = nbins*rv;
	m_histogram[idx] ++;
      }
  QImage img = QImage(m_image, nZ, nY, nZ, QImage::Format_Indexed8);
  
  QImage timg = img.scaled(m_height, m_width,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int w0 = wmin*p2;
  int w1 = qMin(m_width-1, wmax*p2);
  int h0 = hmin*p2;
  int h1 = qMin(m_height-1, hmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int w=w0; w<=w1; w++)
    for(int h=h0; h<=h1; h++)
      {
	int ib = w*m_height + h;
	int ib1 = w*(m_height+boff) + h;
	*(m_depthSliceData+ib) = *(tbits + comp*ib1);
      }      
  
  img = QImage(m_depthSliceData, m_height, m_width, m_height, QImage::Format_Indexed8);
  
  delete [] tmp;

  emit setHiresImage(img, m_height, m_width, level);
  emit updateHistogram(m_histogram);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;

      emit getDepthSlice(level-1, slc,
			 m_wstart, m_wend, m_hstart, m_hend);
    }
}

void
RemapBvf::depthSliceRgb(int slc, int level, bool partlyRead,
			QPair<int, int> wlimits, QPair<int, int> hlimits)
{
  int wmin = wlimits.first;
  int wmax = wlimits.second;
  int hmin = hlimits.first;
  int hmax = hlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth;
  int nY = m_width/p2;
  int nZ = m_height/p2;

  if (slc < 0 || slc >= nX)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
      return;
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nY*nZ];


  int nbytes = nY*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.depthSlice(), nbytes);

  if (m_voxelType == _Rgb)
    {
      for(int w=wmin; w<=wmax; w++)
	for(int h=hmin; h<=hmax; h++)
	  {
	    int i = w*nZ + h;
	    m_image[4*i+0] = tmp[3*i+2];
	    m_image[4*i+1] = tmp[3*i+1];
	    m_image[4*i+2] = tmp[3*i+0];
	    m_image[4*i+3] = 255;
	  }
    }
  else
    {
      for(int w=wmin; w<=wmax; w++)
	for(int h=hmin; h<=hmax; h++)
	  {
	    int i = w*nZ + h;
	    m_image[4*i+0] = tmp[4*i+2];
	    m_image[4*i+1] = tmp[4*i+1];
	    m_image[4*i+2] = tmp[4*i+0];
	    m_image[4*i+3] = tmp[4*i+3];
	  }
    }
  
  
  QImage img = QImage(m_image, nZ, nY, QImage::Format_ARGB32);

  QImage timg = img.scaled(m_height, m_width,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int w0 = wmin*p2;
  int w1 = qMin(m_width-1, wmax*p2);
  int h0 = hmin*p2;
  int h1 = qMin(m_height-1, hmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int w=w0; w<=w1; w++)
    for(int h=h0; h<=h1; h++)
      {
	int ib = w*m_height + h;
	int ib1 = w*(m_height+boff) + h;
	*(m_depthSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_depthSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_depthSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_depthSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }
  
  img = QImage(m_depthSliceData, m_height, m_width, QImage::Format_ARGB32);
  
  delete [] tmp;

  emit setHiresImage(img, m_height, m_width, level);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;

      emit getDepthSlice(level-1, slc,
			 m_wstart, m_wend, m_hstart, m_hend);
    }
}

void
RemapBvf::widthSlice(int slc, int level, bool partlyRead,
		     QPair<int, int> dlimits, QPair<int, int> hlimits)
{
  if (m_currentAxis != 1 ||
      m_currentSlice != slc)
    return;
  
  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    {
      widthSliceRgb(slc, level, partlyRead,
		    dlimits, hlimits);
      return;
    }

  int dmin = dlimits.first;
  int dmax = dlimits.second;
  int hmin = hlimits.first;
  int hmax = hlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth/p2;
  int nY = m_width;
  int nZ = m_height/p2;

  if (slc < 0 || slc >= nY)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nZ];

  int nbytes = nX*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.widthSlice(), nbytes);
 
  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int d=dmin; d<=dmax; d++)
    for(int h=hmin; h<=hmax; h++)
      {
	int i = d*nZ + h;
	
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
	
	float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);
	
	uchar pv = 255*rv;
	m_image[i] = pv;
	
	int idx = nbins*rv;
	m_histogram[idx] ++;
      }
  QImage img = QImage(m_image, nZ, nX, nZ, QImage::Format_Indexed8);
  
  QImage timg = img.scaled(m_height, m_depth,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int d0 = dmin*p2;
  int d1 = qMin(m_depth-1, dmax*p2);
  int h0 = hmin*p2;
  int h1 = qMin(m_height-1, hmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int d=d0; d<=d1; d++)
    for(int h=h0; h<=h1; h++)
      {
	int ib = d*m_height + h;
	int ib1 = d*(m_height+boff) + h;
	*(m_widthSliceData+ib) = *(tbits + comp*ib1);
      }
  
  img = QImage(m_widthSliceData, m_height, m_depth, m_height, QImage::Format_Indexed8);

  delete [] tmp;

  emit setHiresImage(img, m_height, m_depth, level);
  emit updateHistogram(m_histogram);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;

      emit getWidthSlice(level-1, slc,
			 m_dstart, m_dend, m_hstart, m_hend);
    }
}

void
RemapBvf::widthSliceRgb(int slc, int level, bool partlyRead,
			QPair<int, int> dlimits, QPair<int, int> hlimits)
{
  int dmin = dlimits.first;
  int dmax = dlimits.second;
  int hmin = hlimits.first;
  int hmax = hlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth/p2;
  int nY = m_width;
  int nZ = m_height/p2;

  if (slc < 0 || slc >= nY)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nX*nZ];

  int nbytes = nX*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.widthSlice(), nbytes);

  if (m_voxelType == _Rgb)
    {
      for(int d=dmin; d<=dmax; d++)
	for(int h=hmin; h<=hmax; h++)
	  {
	    int i = d*nZ + h;
	    m_image[4*i+0] = tmp[3*i+2];
	    m_image[4*i+1] = tmp[3*i+1];
	    m_image[4*i+2] = tmp[3*i+0];
	    m_image[4*i+3] = 255;
	  }
    }
  else
    {
      for(int d=dmin; d<=dmax; d++)
	for(int h=hmin; h<=hmax; h++)
	  {
	    int i = d*nZ + h;
	    m_image[4*i+0] = tmp[4*i+2];
	    m_image[4*i+1] = tmp[4*i+1];
	    m_image[4*i+2] = tmp[4*i+0];
	    m_image[4*i+3] = tmp[4*i+3];
	  }
    }
  
  QImage img = QImage(m_image, nZ, nX, QImage::Format_ARGB32);
  
  QImage timg = img.scaled(m_height, m_depth,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int d0 = dmin*p2;
  int d1 = qMin(m_depth-1, dmax*p2);
  int h0 = hmin*p2;
  int h1 = qMin(m_height-1, hmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int d=d0; d<=d1; d++)
    for(int h=h0; h<=h1; h++)
      {
	int ib = d*m_height + h;
	int ib1 = d*(m_height+boff) + h;
	*(m_widthSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_widthSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_widthSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_widthSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }
  
  img = QImage(m_widthSliceData, m_height, m_depth, QImage::Format_ARGB32);
  
  delete [] tmp;
  
  emit setHiresImage(img, m_height, m_depth, level);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;
      
      emit getWidthSlice(level-1, slc,
			 m_dstart, m_dend, m_hstart, m_hend);
    }
}

void
RemapBvf::heightSlice(int slc, int level, bool partlyRead,
		      QPair<int, int> dlimits, QPair<int, int> wlimits)
{
  if (m_currentAxis != 2 ||
      m_currentSlice != slc)
    return;

  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    {
      heightSliceRgb(slc, level, partlyRead,
		     dlimits, wlimits);
      return;
    }

  int dmin = dlimits.first;
  int dmax = dlimits.second;
  int wmin = wlimits.first;
  int wmax = wlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth/p2;
  int nY = m_width/p2;
  int nZ = m_height;

  if (slc < 0 || slc >= nZ)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nY];
  memset(m_image, 0, nX*nY);

  int nbytes = nX*nY*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.heightSlice(), nbytes);

  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int d=dmin; d<=dmax; d++)
    for(int w=wmin; w<=wmax; w++)
    {
      int i = d*nY + w;

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

      float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);

      uchar pv = 255*rv;
      m_image[i] = pv;

      int idx = nbins*rv;
      m_histogram[idx] ++;
    }
  QImage img = QImage(m_image, nY, nX, nY, QImage::Format_Indexed8);

  QImage timg = img.scaled(m_width, m_depth,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int d0 = dmin*p2;
  int d1 = qMin(m_depth-1, dmax*p2);
  int w0 = wmin*p2;
  int w1 = qMin(m_width-1, wmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_width;
  
  for(int d=d0; d<=d1; d++)
    for(int w=w0; w<=w1; w++)
      {
	int ib = d*m_width + w;
	int ib1 = d*(m_width+boff) + w;
	*(m_heightSliceData+ib) = *(tbits + comp*ib1);
      }
  
  img = QImage(m_heightSliceData, m_width, m_depth, m_width, QImage::Format_Indexed8);

  delete [] tmp;

  emit setHiresImage(img, m_width, m_depth, level);
  emit updateHistogram(m_histogram);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;

      emit getHeightSlice(level-1, slc,
			  m_dstart, m_dend, m_wstart, m_wend);
    }
}

void
RemapBvf::heightSliceRgb(int slc, int level, bool partlyRead,
			 QPair<int, int> dlimits, QPair<int, int> wlimits)
{
  int dmin = dlimits.first;
  int dmax = dlimits.second;
  int wmin = wlimits.first;
  int wmax = wlimits.second;

  int p2 = qPow(2, level);
  int nX = m_depth/p2;
  int nY = m_width/p2;
  int nZ = m_height;

  if (slc < 0 || slc >= nZ)
    {
      QImage img = QImage(100, 100, QImage::Format_Indexed8);
      emit setHiresImage(img, 100, 100, 5);
    }

  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nX*nY];

  int nbytes = nX*nY*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, m_bfReader.heightSlice(), nbytes);

  if (m_voxelType == _Rgb)
    {
      for(int d=dmin; d<=dmax; d++)
	for(int w=wmin; w<=wmax; w++)
	  {
	    int i = d*nY + w;
	    m_image[4*i+0] = tmp[3*i+2];
	    m_image[4*i+1] = tmp[3*i+1];
	    m_image[4*i+2] = tmp[3*i+0];
	    m_image[4*i+3] = 255;
	  }
    }
  else
    {
      for(int d=dmin; d<=dmax; d++)
	for(int w=wmin; w<=wmax; w++)
	  {
	    int i = d*nY + w;
	    m_image[4*i+0] = tmp[4*i+2];
	    m_image[4*i+1] = tmp[4*i+1];
	    m_image[4*i+2] = tmp[4*i+0];
	    m_image[4*i+3] = tmp[4*i+3];
	  }
    }

  QImage img = QImage(m_image, nY, nX, QImage::Format_ARGB32);

  QImage timg = img.scaled(m_width, m_depth,
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int d0 = dmin*p2;
  int d1 = qMin(m_depth-1, dmax*p2);
  int w0 = wmin*p2;
  int w1 = qMin(m_width-1, wmax*p2);
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_width;
  
  for(int d=d0; d<=d1; d++)
    for(int w=w0; w<=w1; w++)
      {
	int ib = d*m_width + w;
	int ib1 = d*(m_width+boff) + w;
	*(m_heightSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_heightSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_heightSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_heightSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }
  
  img = QImage(m_heightSliceData, m_width, m_depth, QImage::Format_ARGB32);

  delete [] tmp;

  emit setHiresImage(img, m_width, m_depth, level);

  if (level > m_maxLevel && !partlyRead)
    {
      for(int i=0; i<m_histogram.count(); i++)
	m_histogram[i] = 0;

      emit getHeightSlice(level-1, slc,
			  m_dstart, m_dend, m_wstart, m_wend);
    }
}



void RemapBvf::lowresGridSize(int &sslevel,
			      int &ssd, int &ssw, int &ssh)
{
  m_bfReader.getLowresGrid(sslevel,
			     ssd, ssw, ssh);
}

//---------------------------
QImage
RemapBvf::getDepthSliceImage(int slc)
{
  m_dstart = 0;  m_dend = m_depth;
  m_wstart = 0;  m_wend = m_width;
  m_hstart = 0;  m_hend = m_height;

  if (m_bfReader.minLevel() > 0)
    emit getDepthSlice(m_bfReader.minLevel()-1, slc,
		       m_wstart, m_wend, m_hstart, m_hend);

  return getDepthSliceLowresImage(slc);
}
QImage
RemapBvf::getWidthSliceImage(int slc)
{
  m_dstart = 0;  m_dend = m_depth;
  m_wstart = 0;  m_wend = m_width;
  m_hstart = 0;  m_hend = m_height;

  if (m_bfReader.minLevel() > 0)
    emit getWidthSlice(m_bfReader.minLevel()-1, slc,
		       m_dstart, m_dend, m_hstart, m_hend);

  return getWidthSliceLowresImage(slc);
}
QImage
RemapBvf::getHeightSliceImage(int slc)
{
  m_dstart = 0;  m_dend = m_depth;
  m_wstart = 0;  m_wend = m_width;
  m_hstart = 0;  m_hend = m_height;

  if (m_bfReader.minLevel() > 0)
    emit getHeightSlice(m_bfReader.minLevel()-1, slc,
			m_dstart, m_dend, m_wstart, m_wend);

  return getHeightSliceLowresImage(slc);
}
//---------------------------

//---------------------------
void
RemapBvf::startDepthSliceImage(int slc,
			       int wstart, int wend,
			       int hstart, int hend)
{
  if (m_bfReader.minLevel() == 0)
    return;

  m_currentAxis = 0;
  m_currentSlice = slc;

  m_wstart = wstart;
  m_wend = wend;
  m_hstart = hstart;
  m_hend = hend;

  int level = 0;
  bool done = false;
  while (!done)
    {
      if (m_bfReader.depthBlocksPresent(level, slc,
					m_wstart, m_wend,
					m_hstart, m_hend))
	done = true;
      else
	level++;

      if (level >= m_bfReader.minLevel()-1)
	{
	  level = m_bfReader.minLevel()-1;
	  done = true;
	}
    }

  emit getDepthSlice(level, slc,
		     m_wstart, m_wend, m_hstart, m_hend);
}
void
RemapBvf::startWidthSliceImage(int slc,
			       int dstart, int dend,
			       int hstart, int hend)
{
  if (m_bfReader.minLevel() == 0)
    return;

  m_currentAxis = 1;
  m_currentSlice = slc;

  m_dstart = dstart;
  m_dend = dend;
  m_hstart = hstart;
  m_hend = hend;

  int level = 0;
  bool done = false;
  while (!done)
    {
      if (m_bfReader.widthBlocksPresent(level, slc,
					m_dstart, m_dend,
					m_hstart, m_hend))
	done = true;
      else
	level++;

      if (level >= m_bfReader.minLevel()-1)
	{
	  level = m_bfReader.minLevel()-1;
	  done = true;
	}
    }

  emit getWidthSlice(level, slc,
		     m_dstart, m_dend, m_hstart, m_hend);
//  emit getWidthSlice(m_bfReader.minLevel()-1, slc,
//		     m_dstart, m_dend, m_hstart, m_hend);
}
void
RemapBvf::startHeightSliceImage(int slc,
				int dstart, int dend,
				int wstart, int wend)
{
  if (m_bfReader.minLevel() == 0)
    return;

  m_currentAxis = 2;
  m_currentSlice = slc;

  m_dstart = dstart;
  m_dend = dend;
  m_wstart = wstart;
  m_wend = wend;

  int level = 0;
  bool done = false;
  while (!done)
    {
      if (m_bfReader.heightBlocksPresent(level, slc,
					 m_dstart, m_dend,
					 m_wstart, m_wend))
	done = true;
      else
	level++;

      if (level >= m_bfReader.minLevel()-1)
	{
	  level = m_bfReader.minLevel()-1;
	  done = true;
	}
    }

  emit getHeightSlice(level, slc,
		     m_dstart, m_dend, m_wstart, m_wend);
//  emit getHeightSlice(m_bfReader.minLevel()-1, slc,
//		     m_dstart, m_dend, m_wstart, m_wend);
}
//---------------------------

QImage
RemapBvf::getDepthSliceLowresImage(int slc)
{
  m_currentAxis = 0;
  m_currentSlice = slc;

  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    return getDepthSliceLowresImageRgb(slc);


  // update histogram image
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;

  
  int nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresDepthSlice(slc, nY, nZ);
  
  int nbytes = nY*nZ*m_bytesPerVoxel;
  if (m_image)
    delete [] m_image;
  m_image = new uchar[nY*nZ];

  uchar *tmp = new uchar[nbytes];
  
  memcpy(tmp, lowresSlice, nbytes);
  
  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int i=0; i<nY*nZ; i++)
    {
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
      
      float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);
      
      uchar pv = 255*rv;
      m_image[i] = pv;
      
      int idx = nbins*rv;
      m_histogram[idx] ++;
    }
  
  delete [] tmp;
  
  QImage img = QImage(m_image, nZ, nY, nZ, QImage::Format_Indexed8);

  QImage timg = img.scaled(m_height, m_width, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int w=0; w<m_width; w++)
    for(int h=0; h<m_height; h++)
      {
	int ib = w*m_height + h;
	int ib1 = w*(m_height+boff) + h;
	*(m_depthSliceData+ib) = *(tbits + comp*ib1);
      }      
  
  emit updateHistogram(m_histogram);
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;

  return img;
}

QImage
RemapBvf::getDepthSliceLowresImageRgb(int slc)
{
  int nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresDepthSlice(slc, nY, nZ);
  
  int nbytes = nY*nZ*m_bytesPerVoxel;
  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nY*nZ];

  if (m_voxelType == _Rgb)
    {
      for(int i=0; i<nY*nZ; i++)
	{
	  m_image[4*i+0] = lowresSlice[3*i+2];
	  m_image[4*i+1] = lowresSlice[3*i+1];
	  m_image[4*i+2] = lowresSlice[3*i+0];
	  m_image[4*i+3] = 255;
	}
    }
  else
    memcpy(m_image, lowresSlice, 4*nY*nZ);
  
  QImage img = QImage(m_image, nZ, nY, QImage::Format_ARGB32);
  
  QImage timg = img.scaled(m_height, m_width, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;
  
  for(int w=0; w<m_width; w++)
    for(int h=0; h<m_height; h++)
      {
	int ib = w*m_height + h;
	int ib1 = w*(m_height+boff) + h;
	*(m_depthSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_depthSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_depthSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_depthSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }    

  return img;
}


QImage
RemapBvf::getWidthSliceLowresImage(int slc)
{
  m_currentAxis = 1;
  m_currentSlice = slc;

  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    return getWidthSliceLowresImageRgb(slc);

  // update histogram image
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;


  int nX, nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresWidthSlice(slc, nX, nZ);

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nZ];

  int nbytes = nX*nZ*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, lowresSlice, nbytes);

  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int i=0; i<nX*nZ; i++)
    {
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

      float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);

      uchar pv = 255*rv;
      m_image[i] = pv;

      int idx = nbins*rv;
      m_histogram[idx] ++;
    }
  delete [] tmp;

  QImage img = QImage(m_image, nZ, nX, nZ, QImage::Format_Indexed8);

  QImage timg = img.scaled(m_height, m_depth, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;

  for(int d=0; d<m_depth; d++)
    for(int h=0; h<m_height; h++)
      {
	int ib = d*m_height + h;
	int ib1 = d*(m_height+boff) + h;
	*(m_widthSliceData+ib) = *(tbits + comp*ib1);
      }

  emit updateHistogram(m_histogram);
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;

  return img;
}

QImage
RemapBvf::getWidthSliceLowresImageRgb(int slc)
{
  int nX, nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresWidthSlice(slc, nX, nZ);

  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nX*nZ];

  if (m_voxelType == _Rgb)
    {
      for(int i=0; i<nX*nZ; i++)
	{
	  m_image[4*i+0] = lowresSlice[3*i+2];
	  m_image[4*i+1] = lowresSlice[3*i+1];
	  m_image[4*i+2] = lowresSlice[3*i+0];
	  m_image[4*i+3] = 255;
	}
    }
  else
    memcpy(m_image, lowresSlice, 4*nX*nZ);
  
  QImage img = QImage(m_image, nZ, nX, QImage::Format_ARGB32);

  QImage timg = img.scaled(m_height, m_depth, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;
  int boff = (timg.bytesPerLine()/comp) - m_height;

  for(int d=0; d<m_depth; d++)
    for(int h=0; h<m_height; h++)
      {
	int ib = d*m_height + h;
	int ib1 = d*(m_height+boff) + h;
	*(m_widthSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_widthSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_widthSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_widthSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }

  return img;
}


QImage
RemapBvf::getHeightSliceLowresImage(int slc)
{
  m_currentAxis = 2;
  m_currentSlice = slc;

  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    return getHeightSliceLowresImageRgb(slc);

  // update histogram image
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;


  int nX, nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresHeightSlice(slc, nX, nY);

  if (m_image)
    delete [] m_image;
  m_image = new uchar[nX*nY];

  int nbytes = nX*nY*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  memcpy(tmp, lowresSlice, nbytes);

  int nbins = m_histogram.size()-1;
  int rawSize = m_rawMap.size()-1;
  for(int i=0; i<nX*nY; i++)
    {
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

      float rv = qBound(0.0f, (v-m_rawMin)/(m_rawMax-m_rawMin), 1.0f);

      uchar pv = 255*rv;
      m_image[i] = pv;

      int idx = nbins*rv;
      m_histogram[idx] ++;
    }
  delete [] tmp;

  QImage img = QImage(m_image, nY, nX, nY, QImage::Format_Indexed8);

  QImage timg = img.scaled(m_width, m_depth, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;

  int boff = (timg.bytesPerLine()/comp) - m_width;
  for(int d=0; d<m_depth; d++)
    for(int w=0; w<m_width; w++)
      {
	int ib = d*m_width + w;
	int ib1 = d*(m_width+boff) + w;
	*(m_heightSliceData+ib) = *(tbits + comp*ib1);
      }

  emit updateHistogram(m_histogram);
  for(int i=0; i<m_histogram.count(); i++)
    m_histogram[i] = 0;

  return img;
}

QImage
RemapBvf::getHeightSliceLowresImageRgb(int slc)
{
  int nX, nY, nZ;
  uchar *lowresSlice = m_bfReader.getLowresHeightSlice(slc, nX, nY);

  if (m_image)
    delete [] m_image;
  m_image = new uchar[4*nX*nY];

  if (m_voxelType == _Rgb)
    {
      for(int i=0; i<nX*nY; i++)
	{
	  m_image[4*i+0] = lowresSlice[3*i+2];
	  m_image[4*i+1] = lowresSlice[3*i+1];
	  m_image[4*i+2] = lowresSlice[3*i+0];
	  m_image[4*i+3] = 255;
	}
    }
  else
    memcpy(m_image, lowresSlice, 4*nX*nY);

  QImage img = QImage(m_image, nY, nX, QImage::Format_ARGB32);

  QImage timg = img.scaled(m_width, m_depth, 
			   Qt::IgnoreAspectRatio,
			   Qt::SmoothTransformation);
  const uchar *tbits = timg.bits();
  int comp = 4;
  if(timg.depth() == 8) comp = 1;

  int boff = (timg.bytesPerLine()/comp) - m_width;
  for(int d=0; d<m_depth; d++)
    for(int w=0; w<m_width; w++)
      {
	int ib = d*m_width + w;
	int ib1 = d*(m_width+boff) + w;
	*(m_heightSliceData+4*ib+0) = *(tbits + 4*ib1+0);
	*(m_heightSliceData+4*ib+1) = *(tbits + 4*ib1+1);
	*(m_heightSliceData+4*ib+2) = *(tbits + 4*ib1+2);
	*(m_heightSliceData+4*ib+3) = *(tbits + 4*ib1+3);
      }

  return img;
}


QPair<QVariant, QVariant>
RemapBvf::rawValue(int d, int w, int h)
{
  QPair<QVariant, QVariant> pair;

  pair.first = QVariant("OutOfBounds");
  pair.second = QVariant("OutOfBounds");
  return pair;

//  if (d < 0 || d >= m_depth ||
//      w < 0 || w >= m_width ||
//      h < 0 || h >= m_height)
//    {
//      pair.first = QVariant("OutOfBounds");
//      pair.second = QVariant("OutOfBounds");
//      return pair;
//    }
//
//  QFile fin(m_fileName);
//  fin.open(QFile::ReadOnly);
//  fin.seek(m_skipBytes +
//	   m_bytesPerVoxel*(d*m_width*m_height +
//			    w*m_height +
//			    h));
//
//  QVariant v;
//
//  if (m_voxelType == _UChar)
//    {
//      unsigned char a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((uint)a);
//    }
//  else if (m_voxelType == _Char)
//    {
//      char a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _UShort)
//    {
//      unsigned short a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((uint)a);
//    }
//  else if (m_voxelType == _Short)
//    {
//      short a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _Int)
//    {
//      int a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _Float)
//    {
//      float a;
//      fin.read((char*)&a, m_bytesPerVoxel);
//      v = QVariant((double)a);
//    }
//  fin.close();
// 
//
//  int rawSize = m_rawMap.size()-1;
//  int idx = rawSize;
//  float frc = 0;
//  float val;
//
//  if (v.type() == QVariant::UInt)
//    val = v.toUInt();
//  else if (v.type() == QVariant::Int)
//    val = v.toInt();
//  else if (v.type() == QVariant::Double)
//    val = v.toDouble();
//
//  if (val <= m_rawMap[0])
//    {
//      idx = 0;
//      frc = 0;
//    }
//  else if (val >= m_rawMap[rawSize])
//    {
//      idx = rawSize-1;
//      frc = 1;
//    }
//  else
//    {
//      for(int m=0; m<rawSize; m++)
//	{
//	  if (val >= m_rawMap[m] &&
//	      val <= m_rawMap[m+1])
//	    {
//	      idx = m;
//	      frc = ((float)val-(float)m_rawMap[m])/
//		((float)m_rawMap[m+1]-(float)m_rawMap[m]);
//	    }
//	}
//    }
//  
//  uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
//
//  pair.first = v;
//  pair.second = QVariant((uint)pv);
//  return pair;
}

void
RemapBvf::saveTrimmed(QString trimFile,
		      int dmin, int dmax,
		      int wmin, int wmax,
		      int hmin, int hmax)
{
//  QProgressDialog progress("Saving trimmed volume",
//			   "Cancel",
//			   0, 100,
//			   0);
//  progress.setMinimumDuration(0);
//  
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int mX, mY, mZ;
//  mX = dmax-dmin+1;
//  mY = wmax-wmin+1;
//  mZ = hmax-hmin+1;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  uchar vt;
//  if (m_voxelType == _UChar) vt = 0; // unsigned byte
//  if (m_voxelType == _Char) vt = 1; // signed byte
//  if (m_voxelType == _UShort) vt = 2; // unsigned short
//  if (m_voxelType == _Short) vt = 3; // signed short
//  if (m_voxelType == _Int) vt = 4; // int
//  if (m_voxelType == _Float) vt = 8; // float
//  
//  QFile fout(trimFile);
//  fout.open(QFile::WriteOnly);
//
//  fout.write((char*)&vt, 1);
//  fout.write((char*)&mX, 4);
//  fout.write((char*)&mY, 4);
//  fout.write((char*)&mZ, 4);
//
//  QFile fin(m_fileName);
//  fin.open(QFile::ReadOnly);
//  fin.seek(m_skipBytes + nbytes*dmin);
//
//  for(int i=dmin; i<=dmax; i++)
//    {
//      fin.read((char*)tmp, nbytes);
//
//      for(int j=wmin; j<=wmax; j++)
//	{
//	  memcpy(tmp+(j-wmin)*mZ*m_bytesPerVoxel,
//		 tmp+(j*nZ + hmin)*m_bytesPerVoxel,
//		 mZ*m_bytesPerVoxel);
//	}
//	  
//      fout.write((char*)tmp, mY*mZ*m_bytesPerVoxel);
//
//      progress.setValue((int)(100*(float)(i-dmin)/(float)mX));
//      qApp->processEvents();
//    }
//
//  fin.close();
//  fout.close();
//
//  delete [] tmp;
//
//  m_headerBytes = 13; // to be used for applyMapping function
}

//-------------------------------
//-------------------------------

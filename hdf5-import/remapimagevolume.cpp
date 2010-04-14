#include "remapimagevolume.h"
#include "global.h"
#include <QDomDocument>
#include "volumefilemanager.h"

RemapImageVolume::RemapImageVolume()
{
  m_fileName.clear();
  m_imageList.clear();

  m_depth = m_width = m_height = 0;
  m_headerBytes = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;

  m_rawMin = m_rawMax = 0;
  m_histogram.clear();

  m_rawMap.clear();
  m_pvlMap.clear();


  m_image = 0;
}

RemapImageVolume::~RemapImageVolume()
{
  m_fileName.clear();
  m_imageList.clear();

  m_depth = m_width = m_height = 0;
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
RemapImageVolume::setMinMax(float rmin, float rmax)
{
  m_rawMin = rmin;
  m_rawMax = rmax;
}

void
RemapImageVolume::setMap(QList<float> rm,
			 QList<uchar> pm)
{
  m_rawMap = rm;
  m_pvlMap = pm;
}

float RemapImageVolume::rawMin() { return m_rawMin; }
float RemapImageVolume::rawMax() { return m_rawMax; }
QList<uint> RemapImageVolume::histogram() { return m_histogram; }

void
RemapImageVolume::gridSize(int& d, int& w, int& h)
{
  d = m_depth;
  w = m_width;
  h = m_height;
}

bool
RemapImageVolume::setFile(QList<QString> fl)
{
  m_fileName = fl;
  
  // list all image files in the directory
  QStringList imageNameFilter;
  imageNameFilter << "*.bmp";
  imageNameFilter << "*.gif";
  imageNameFilter << "*.jpg";
  imageNameFilter << "*.jpeg";
  imageNameFilter << "*.png";
  imageNameFilter << "*.pbm";
  imageNameFilter << "*.pgm";
  imageNameFilter << "*.ppm";
  imageNameFilter << "*.tif";
  imageNameFilter << "*.tiff";
  imageNameFilter << "*.xbm";
  imageNameFilter << "*.xpm";
  QStringList files= QDir(m_fileName[0]).entryList(imageNameFilter,
						   QDir::NoSymLinks|
						   QDir::NoDotAndDotDot|
						   QDir::Readable|
						   QDir::Files);
  m_imageList.clear();
  for(uint i=0; i<files.size(); i++)
    {
      QFileInfo fileInfo(m_fileName[0], files[i]);
      QString imgfl = fileInfo.absoluteFilePath();
      m_imageList.append(imgfl);
    }

  m_depth = m_imageList.size();
  QImage img = QImage(m_imageList[0]);
  m_height = img.width();
  m_width = img.height();
  m_headerBytes = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;
  findMinMaxandGenerateHistogram();

  m_rawMap.append(m_rawMin);
  m_rawMap.append(m_rawMax);
  m_pvlMap.append(0);
  m_pvlMap.append(255);

  return true;
}

void
RemapImageVolume::findMinMaxandGenerateHistogram()
{
  m_histogram.clear();
  for(uint i=0; i<256; i++)
    m_histogram.append(0);

  m_rawMin = 10000000;
  m_rawMax = -10000000;

  // do not evaluate histogram for images for rgb volume
  if (Global::rgbVolume())
    return;

  QProgressDialog progress("Generating Histogram",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  for(uint i=0; i<m_depth; i++)
    {
      progress.setValue((int)(100.0*(float)i/(float)m_depth));
      qApp->processEvents();

      QImage img = QImage(m_imageList[i]);
      if (img.format() != QImage::Format_RGB32)
	img = img.convertToFormat(QImage::Format_RGB32);

      uchar *imgbits = img.bits();
      for(uint j=0; j<m_width*m_height; j++)
	{ 
	  int val = imgbits[4*j];
	  m_histogram[val]++;
	  m_rawMin = qMin(m_rawMin, (float)val);
	  m_rawMax = qMax(m_rawMax, (float)val);
	}
    }

  while(m_histogram.last() == 0)
    m_histogram.removeLast();

  while(m_histogram.first() == 0)
    m_histogram.removeFirst();

  progress.setValue(100);
  qApp->processEvents();
}

void
RemapImageVolume::getDepthSlice(int slc,
				uchar *slice)
{
  QImage imgL = QImage(m_imageList[slc]);
  if (imgL.format() != QImage::Format_ARGB32)
    imgL = imgL.convertToFormat(QImage::Format_ARGB32);

  uchar *imgbits = imgL.bits();
  for(uint j=0; j<m_width*m_height; j++)
    slice[j] = imgbits[4*j];
}

QImage
RemapImageVolume::getDepthSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;
  m_image = new uchar[m_width*m_height];

  int nbytes = m_width*m_height*m_bytesPerVoxel;
  uchar *tmp = new uchar[nbytes];

  QImage imgL = QImage(m_imageList[slc]);
  if (imgL.format() != QImage::Format_ARGB32)
    imgL = imgL.convertToFormat(QImage::Format_ARGB32);

  if (Global::rgbVolume())
    {
      delete [] tmp;
      QImage img = imgL;
      return img;
    }

  uchar *imgbits = imgL.bits();
  for(uint j=0; j<m_width*m_height; j++)
    tmp[j] = imgbits[4*j];

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_width*m_height; i++)
    {
      int idx = m_rawMap.size()-1;
      float v = ((uchar *)tmp)[i];
      float frc = 0;

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
  QImage img = QImage(m_image, m_height, m_width, m_height, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RemapImageVolume::getWidthSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;

  if (Global::rgbVolume())
    m_image = new uchar[4*m_depth*m_height];
  else
    m_image = new uchar[m_depth*m_height];


  int nbytes;
  if (Global::rgbVolume())
    nbytes = m_depth*m_height*4;
  else
    nbytes = m_depth*m_height*m_bytesPerVoxel;

  uchar *tmp = new uchar[nbytes];

  for(uint i=0; i<m_depth; i++)
    {
      QImage imgL = QImage(m_imageList[i]);
      if (imgL.format() != QImage::Format_ARGB32)
	imgL = imgL.convertToFormat(QImage::Format_ARGB32);

      uchar *imgbits = imgL.bits();
      if (Global::rgbVolume())
	{
	  for(uint j=0; j<m_height; j++)
	    {
	      tmp[4*(i*m_height+j)+0] = imgbits[4*(slc*m_height+j)+0];
	      tmp[4*(i*m_height+j)+1] = imgbits[4*(slc*m_height+j)+1];
	      tmp[4*(i*m_height+j)+2] = imgbits[4*(slc*m_height+j)+2];
	      tmp[4*(i*m_height+j)+3] = imgbits[4*(slc*m_height+j)+3];
	    }
	}
      else
	{
	  for(uint j=0; j<m_height; j++)
	    tmp[i*m_height+j] = imgbits[4*(slc*m_height+j)];
	}
    }

  if (Global::rgbVolume())
    {  
      memcpy(m_image, tmp, 4*m_depth*m_height);
      QImage img = QImage(m_image,
			  m_height, m_depth,
			  QImage::Format_ARGB32);

      delete [] tmp;

      return img;
    }

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_depth*m_height; i++)
    {
      int idx = m_rawMap.size()-1;
      float v = ((uchar *)tmp)[i];
      float frc = 0;

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
  QImage img = QImage(m_image, m_height, m_depth, m_height, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QImage
RemapImageVolume::getHeightSliceImage(int slc)
{
  if (m_image)
    delete [] m_image;

  if (Global::rgbVolume())
    m_image = new uchar[4*m_depth*m_width];
  else
    m_image = new uchar[m_depth*m_width];

  int nbytes;
  if (Global::rgbVolume())
    nbytes = m_depth*m_width*4;
  else
    nbytes = m_depth*m_width*m_bytesPerVoxel;

  uchar *tmp = new uchar[nbytes];


  for(uint i=0; i<m_depth; i++)
    {
      QImage imgL = QImage(m_imageList[i]);
      if (imgL.format() != QImage::Format_ARGB32)
	imgL = imgL.convertToFormat(QImage::Format_ARGB32);

      uchar *imgbits = imgL.bits();
      if (Global::rgbVolume())
	{
	  for(uint j=0; j<m_width; j++)
	    {
	      tmp[4*(i*m_width+j)+0] = imgbits[4*(j*m_height+slc)+0];
	      tmp[4*(i*m_width+j)+1] = imgbits[4*(j*m_height+slc)+1];
	      tmp[4*(i*m_width+j)+2] = imgbits[4*(j*m_height+slc)+2];
	      tmp[4*(i*m_width+j)+3] = imgbits[4*(j*m_height+slc)+3];
	    }
	}
      else
	{
	  for(uint j=0; j<m_width; j++)
	    tmp[i*m_width+j] = imgbits[4*(j*m_height+slc)];
	}
    }

  if (Global::rgbVolume())
    {  
      memcpy(m_image, tmp, 4*m_depth*m_width);
      QImage img = QImage(m_image,
			  m_width, m_depth,
			  QImage::Format_ARGB32);

      delete [] tmp;

      return img;
    }

  int rawSize = m_rawMap.size()-1;
  for(uint i=0; i<m_depth*m_width; i++)
    {
      int idx = m_rawMap.size()-1;
      float v = ((uchar *)tmp)[i];
      float frc = 0;

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
  QImage img = QImage(m_image, m_width, m_depth, m_width, QImage::Format_Indexed8);

  delete [] tmp;

  return img;
}

QPair<QVariant, QVariant>
RemapImageVolume::rawValue(int d, int w, int h)
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

  QImage imgL = QImage(m_imageList[d]);
  if (imgL.format() != QImage::Format_RGB32)
    imgL = imgL.convertToFormat(QImage::Format_RGB32);

  uchar *imgbits = imgL.bits();

  if (Global::rgbVolume())
    {
      uchar r = imgbits[4*(w*m_height+h)+0];
      uchar g = imgbits[4*(w*m_height+h)+1];
      uchar b = imgbits[4*(w*m_height+h)+2];
      uchar a = imgbits[4*(w*m_height+h)+3];
      
      pair.first = QVariant(QString(" (%1 %2 %3 %4)").\
			    arg(r).arg(g).arg(b).arg(a));
      pair.second = QVariant("rgba");
      return pair;
    }

  uint val = imgbits[4*(w*m_height+h)];
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

  pair.first = QVariant((uint)val);
  pair.second = QVariant((uint)pv);
  return pair;
}

void
RemapImageVolume::saveTrimmed(QString trimFile,
			    int dmin, int dmax,
			    int wmin, int wmax,
			    int hmin, int hmax)
{
  if (Global::rgbVolume())
    {
      saveTrimmedRGB(trimFile,
		     dmin, dmax,
		     wmin, wmax,
		     hmin, hmax);
      return;
    }

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

  int bpv = m_bytesPerVoxel;
  int nbytes = nY*nZ*bpv;
  uchar *tmp = new uchar[nbytes];

  uchar vt = 0;

  QFile fout(trimFile);
  fout.open(QFile::WriteOnly);

  fout.write((char*)&vt, 1);
  fout.write((char*)&mX, 4);
  fout.write((char*)&mY, 4);
  fout.write((char*)&mZ, 4);

  //for(uint i=dmin; i<=dmax; i++)
  for(int i=dmax; i>=dmin; i--)
    {

      QImage imgL = QImage(m_imageList[i]);
      if (imgL.format() != QImage::Format_ARGB32)
	imgL = imgL.convertToFormat(QImage::Format_ARGB32);
      
      uchar *imgbits = imgL.bits();
      for(uint j=0; j<m_width*m_height; j++)
	tmp[j] = imgbits[4*j];

      for(uint j=wmin; j<=wmax; j++)
	{
	  memcpy(tmp+(j-wmin)*mZ*bpv,
		 tmp+(j*nZ + hmin)*bpv,
		 mZ*bpv);
	}
      
      fout.write((char*)tmp, mY*mZ*bpv);
      
      progress.setValue((int)(100*(float)(i-dmin)/(float)mX));
      qApp->processEvents();
    }

  fout.close();

  delete [] tmp;

  m_headerBytes = 13; // to be used for applyMapping function
}

void
RemapImageVolume::savePvlHeader(QString pvlFilename,
				int d, int w, int h,
				QString voxelType,
				int slabSize)
{
  QString xmlfile = pvlFilename;

  QDomDocument doc("Drishti_Header");

  QDomElement topElement = doc.createElement("PvlDotNcFileHeader");
  doc.appendChild(topElement);

  {      
    QDomElement de0 = doc.createElement("gridsize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1 %2 %3").arg(d).arg(w).arg(h));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QString vstr = "no units";    
    QDomElement de0 = doc.createElement("voxelunit");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QString vstr = voxelType;    
    QDomElement de0 = doc.createElement("voxeltype");
    QDomText tn0;
    tn0 = doc.createTextNode(voxelType);
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QDomElement de0 = doc.createElement("voxelsize");
    QDomText tn0;
    tn0 = doc.createTextNode("1 1 1");
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
  
  {
    QDomElement de0 = doc.createElement("description");
    QDomText tn0;
    tn0 = doc.createTextNode("Colour volume");
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QDomElement de0 = doc.createElement("slabsize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(slabSize));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }  
  
  QFile f(xmlfile.toAscii().data());
  if (f.open(QIODevice::WriteOnly))
    {
      QTextStream out(&f);
      doc.save(out, 2);
      f.close();
    }
}

void
RemapImageVolume::saveTrimmedRGB(QString trimFile,
				 int dmin, int dmax,
				 int wmin, int wmax,
				 int hmin, int hmax)
{
  QStringList dtypes;
  dtypes << "No"
	 << "Yes";

  QString option = QInputDialog::getItem(0,
					 "Save Alpha Channel",
					 "Alpha Channel",
					 dtypes,
					 0,
					 false);
  
  bool saveAlpha = false;
  if (option == "Yes")
    saveAlpha = true;


  QProgressDialog progress("Saving trimmed RGB volume",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  int nX, nY, nZ;
  nX = m_depth;
  nY = m_width;
  nZ = m_height;

  int d, w, h;
  d = dmax-dmin+1;
  w = wmax-wmin+1;
  h = hmax-hmin+1;

  int nbytes = nY*nZ;
  uchar *tmpR = new uchar[nbytes];
  uchar *tmpG = new uchar[nbytes];
  uchar *tmpB = new uchar[nbytes];
  uchar *tmpA;
  if (saveAlpha)
    tmpA = new uchar[nbytes];


  QString voxelType = "RGB";
  if (saveAlpha) voxelType = "RGBA";
  
  //*** max 1Gb per slab
  int slabSize;
  slabSize = (1024*1024*1024)/(w*h);

  savePvlHeader(trimFile,
		d, w, h,
		voxelType,
		slabSize);			       

  VolumeFileManager rFileManager;
  VolumeFileManager gFileManager;
  VolumeFileManager bFileManager;
  VolumeFileManager aFileManager;

  QString pvlfile = trimFile;
  pvlfile.chop(6);

  QString rFilename = pvlfile + QString("red");
  QString gFilename = pvlfile + QString("green");
  QString bFilename = pvlfile + QString("blue");
  QString aFilename = pvlfile + QString("alpha");

  rFileManager.setBaseFilename(rFilename);
  rFileManager.setDepth(d);
  rFileManager.setWidth(w);
  rFileManager.setHeight(h);
  rFileManager.setVoxelType(0);
  rFileManager.setHeaderSize(13);
  rFileManager.createFile(true);
  rFileManager.createFile(true);

  gFileManager.setBaseFilename(gFilename);
  gFileManager.setDepth(d);
  gFileManager.setWidth(w);
  gFileManager.setHeight(h);
  gFileManager.setVoxelType(0);
  gFileManager.setHeaderSize(13);
  gFileManager.createFile(true);
  gFileManager.createFile(true);

  bFileManager.setBaseFilename(bFilename);
  bFileManager.setDepth(d);
  bFileManager.setWidth(w);
  bFileManager.setHeight(h);
  bFileManager.setVoxelType(0);
  bFileManager.setHeaderSize(13);
  bFileManager.createFile(true);
  bFileManager.createFile(true);

  if (saveAlpha)
    {
      aFileManager.setBaseFilename(aFilename);
      aFileManager.setDepth(d);
      aFileManager.setWidth(w);
      aFileManager.setHeight(h);
      aFileManager.setVoxelType(0);
      aFileManager.setHeaderSize(13);
      aFileManager.createFile(true);
      aFileManager.createFile(true);
    }


  for(int i=dmax; i>=dmin; i--)
    {

      QImage imgL = QImage(m_imageList[i]);
      if (imgL.format() != QImage::Format_ARGB32)
	imgL = imgL.convertToFormat(QImage::Format_ARGB32);
      
      uchar *imgbits = imgL.bits();
      for(uint j=0; j<m_width*m_height; j++)
	{
	  tmpR[j] = imgbits[4*j+2];
	  tmpG[j] = imgbits[4*j+1];
	  tmpB[j] = imgbits[4*j+0];
	}

      if (saveAlpha)
	{
	  for(uint j=0; j<m_width*m_height; j++)
	    tmpA[j] = imgbits[4*j+3];
	}

      for(uint j=wmin; j<=wmax; j++)
	memcpy(tmpR+(j-wmin)*h,
	       tmpR+(j*nZ + hmin),
	       h);

      for(uint j=wmin; j<=wmax; j++)
	memcpy(tmpG+(j-wmin)*h,
	       tmpG+(j*nZ + hmin),
	       h);

      for(uint j=wmin; j<=wmax; j++)
	memcpy(tmpB+(j-wmin)*h,
	       tmpB+(j*nZ + hmin),
	       h);

      if (saveAlpha)
	{
	  for(uint j=wmin; j<=wmax; j++)
	    memcpy(tmpA+(j-wmin)*h,
		   tmpA+(j*nZ + hmin),
		   h);
	}

      rFileManager.setSlice(dmax-i, tmpR);
      gFileManager.setSlice(dmax-i, tmpG);
      bFileManager.setSlice(dmax-i, tmpB);
      if (saveAlpha)
	aFileManager.setSlice(dmax-i, tmpA);

      progress.setValue((int)(100*(float)(dmax-i)/(float)d));
      qApp->processEvents();
    }


  delete [] tmpR;
  delete [] tmpG;
  delete [] tmpB;

  if (saveAlpha)
    delete [] tmpA;

  progress.setValue(100);
}

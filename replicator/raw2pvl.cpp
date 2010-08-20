#include "global.h"
#include "staticfunctions.h"
#include "raw2pvl.h"
#include <netcdfcpp.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include <QtXml>
#include <QFile>

#include "savepvldialog.h"
#include "volumefilemanager.h"
#include "blockfilewriter.h"

#ifdef Q_WS_WIN
#include <float.h>
#define ISNAN(v) _isnan(v)
#else
#define ISNAN(v) isnan(v)
#endif

#define REMAPVOLUME()							\
  {									\
    for(uint j=0; j<width*height; j++)					\
      {									\
	float v = ptr[j];						\
	int idx;							\
	float frc;							\
	if (v <= rawMap[0] || ISNAN(v))					\
	  {								\
	    idx = 0;							\
	    frc = 0;							\
	  }								\
	else if (v >= rawMap[rawSize])					\
	  {								\
	    idx = rawSize-1;						\
	    frc = 1;							\
	  }								\
	else								\
	  {								\
	    for(uint m=0; m<rawSize; m++)				\
	      {								\
		if (v >= rawMap[m] &&					\
		    v <= rawMap[m+1])					\
		  {							\
		    idx = m;						\
		    frc = ((float)v-rawMap[m])/				\
		      (rawMap[m+1]-rawMap[m]);				\
		  }							\
	      }								\
	  }								\
									\
	uchar pv = pvlMap[idx] + frc*(pvlMap[idx+1]-pvlMap[idx]);	\
	pvl[j] = pv;							\
      }									\
  }

void
Raw2Pvl::applyMapping(QString rawFilename,
		      QString mappedFilename,
		      int voxelType,
		      int headerBytes,
		      int depth, int width, int height,
		      QList<float> rawMap,
		      QList<uchar> pvlMap)
{
  QProgressDialog progress(QString("Remapping %1").arg(rawFilename),
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  QFile fout(mappedFilename);
  fout.open(QFile::WriteOnly);
  
  QFile fin(rawFilename);
  fin.open(QFile::ReadOnly);
  fin.seek(headerBytes);

  int bpv = 1;
  if (voxelType == _UChar) bpv = 1;
  else if (voxelType == _Char) bpv = 1;
  else if (voxelType == _UShort) bpv = 2;
  else if (voxelType == _Short) bpv = 2;
  else if (voxelType == _Int) bpv = 4;
  else if (voxelType == _Float) bpv = 4;

  int nbytes = width*height*bpv;
  uchar *raw = new uchar[nbytes];
  uchar *pvl = new uchar[width*height];

  int rawSize = rawMap.size()-1;
  for(uint d=0; d<depth; d++)
    {
      progress.setValue((int)(100.0*(float)d/(float)depth));
      qApp->processEvents();

      fin.read((char*)raw, nbytes);

      if (voxelType == _UChar)
	{
	  uchar *ptr = raw;
	  REMAPVOLUME();
	}
      else if (voxelType == _Char)
	{
	  char *ptr = (char*)raw;
	  REMAPVOLUME();
	}
      else if (voxelType == _UShort)
	{
	  ushort *ptr = (ushort*)raw;
	  REMAPVOLUME();
	}
      else if (voxelType == _Short)
	{
	  short *ptr = (short*)raw;
	  REMAPVOLUME();
	}
      else if (voxelType == _Int)
	{
	  int *ptr = (int*)raw;
	  REMAPVOLUME();
	}
      else if (voxelType == _Float)
	{
	  float *ptr = (float*)raw;
	  REMAPVOLUME();
	}

      fout.write((char*)pvl, width*height);
    }

  delete [] raw;
  delete [] pvl;

  fout.close();
  fin.close();

  progress.setValue(100);
  qApp->processEvents();
}


void
Raw2Pvl::applyRaw2PvlConversion(QString mappedFilename,
				QString pvlFilename,
				int depth,
				int width,
				int height)
{
  QProgressDialog progress("Processing volume",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);
      

  QFile fin(mappedFilename);
  fin.open(QFile::ReadOnly);

  progress.setValue(10);
  qApp->processEvents();

  NcFile pvlFile((char*)pvlFilename.toAscii().data(),
		 NcFile::Write);

  NcDim *gridX, *gridY, *gridZ;
  gridX = pvlFile.add_dim("gridX", depth);
  gridY = pvlFile.add_dim("gridY", width);
  gridZ = pvlFile.add_dim("gridZ", height);

  progress.setValue(20);
  qApp->processEvents();

  NcVar *pvol;
  pvol = pvlFile.add_var("ProcessedVolume",
			 ncByte,
			 gridX, gridY, gridZ);
  
  progress.setValue(50);
  qApp->processEvents();
  //--------------------------------------------------------
  // create space for 1D and 2D histograms in the netCDF file
  pvlFile.add_att("histogram_ok", (ncbyte)0);
  NcDim *hist1D;
  NcDim *hist2D;
  hist1D = pvlFile.add_dim("hist1D", 256);
  hist2D = pvlFile.add_dim("hist2D", 256*256);
  NcVar *hvol1D, *hvol2D;
  hvol1D = pvlFile.add_var("Histogram1D", ncInt, hist1D);
  hvol2D = pvlFile.add_var("Histogram2D", ncInt, hist2D);
  //--------------------------------------------------------
  progress.setValue(60);
  qApp->processEvents();

  uchar *rv = new uchar [width*height];
  for(uint d=0; d<depth; d++)
    {
      progress.setValue((int)(100*(float)d/(float)depth));

      fin.read((char*)rv, width*height);

      pvol->set_cur(d, 0, 0);
      pvol->put((const ncbyte*)rv, 1, width, height);
    }
  delete [] rv;
  fin.close();
  pvlFile.close();
  progress.setValue(100);
}

//-----------------------------------------
//-----------------------------------------
#define MEANFILTER()				\
  {						\
    for(int j=0; j<width; j++)			\
      for(int k=0; k<height; k++)		\
	{					\
	  int js = qMax(0, j-1);		\
	  int je = qMin(width-1, j+1);		\
	  int ks = qMax(0, k-1);		\
	  int ke = qMin(height-1, k+1);		\
						\
	  int dn = 0;				\
	  float avg = 0;			\
	  for(int j1=js; j1<=je; j1++)		\
	    for(int k1=ks; k1<=ke; k1++)	\
	      {					\
		int idx = j1*height+k1;		\
		float savg = (p0[idx] +		\
			      4*p1[idx] +	\
			      p2[idx]);		\
		dn += 6;			\
		if (j1 == j)			\
		  {				\
		    avg += 4*savg;		\
		    dn += 18;			\
		  }				\
		else				\
		  avg += savg;			\
	      }					\
	  avg = avg/dn;				\
	  p[j*height + k] = avg;		\
	}					\
  }

#define MEDIANFILTER()				\
  {						\
    for(int j=0; j<width; j++)			\
      for(int k=0; k<height; k++)		\
	{					\
	  int js = qMax(0, j-1);		\
	  int je = qMin(width-1, j+1);		\
	  int ks = qMax(0, k-1);		\
	  int ke = qMin(height-1, k+1);		\
	  					\
	  QList<float> vlist;			\
	  for(int j1=js; j1<=je; j1++)		\
	    for(int k1=ks; k1<=ke; k1++)	\
	      {					\
		int idx = j1*height+k1;		\
		vlist.append((float)p0[idx]);	\
		vlist.append((float)p1[idx]);	\
		vlist.append((float)p2[idx]);	\
	      }					\
	  qSort(vlist.begin(), vlist.end());	\
	  p[j*height + k] = vlist[vlist.size()/2];\
	}					\
  }

//-----------------------------------------
// sigma2 is used to define the domain filter
// it will decide the decay based on how far the
// value is from the central voxel value
#define BILATERALFILTER(dsigma)			\
  {						\
    float sigma2 = 2*dsigma*dsigma;		\
    for(int j=0; j<width; j++)			\
      for(int k=0; k<height; k++)		\
	{					\
	  int js = qMax(0, j-1);		\
	  int je = qMin(width-1, j+1);		\
	  int ks = qMax(0, k-1);		\
	  int ke = qMin(height-1, k+1);		\
						\
	  float cv = p1[j*height + k];		\
	  float dn = 0;				\
	  float avg = 0;			\
	  for(int j1=js; j1<=je; j1++)		\
	    for(int k1=ks; k1<=ke; k1++)	\
	      {					\
		int idx = j1*height+k1;		\
		float a0 = exp(-((p0[idx]-cv)*(p0[idx]-cv))/sigma2);\
		float a1 = exp(-((p1[idx]-cv)*(p1[idx]-cv))/sigma2);\
		float a2 = exp(-((p2[idx]-cv)*(p2[idx]-cv))/sigma2);\
		float savg = (a0*p0[idx] +			    \
			      4*a1*p1[idx] +			    \
			      a2*p2[idx]);			    \
		float ddn = a0+4*a1+a2;				    \
		if (j1 != j && k1 != k)			\
		  {					\
		    avg += savg;			\
		    dn += ddn;				\
		  }					\
		else					\
		  {					\
		    if (j1 == j)			\
		      {					\
			avg += 4*savg;			\
			dn += 4*ddn;			\
		      }					\
		    if (k1 == k)			\
		      {					\
			avg += 4*savg;			\
			dn += 4*ddn;			\
		      }					\
		  }					\
	      }					\
	  avg = avg/dn;				\
	  p[j*height + k] = avg;		\
	}					\
  }


void
Raw2Pvl::applyFilter(uchar *val0,
		     uchar *val1,
		     uchar *val2,
		     uchar *vg,
		     int voxelType,
		     int width, int height,
		     int filter)
{
  if (voxelType == _UChar)
    {
      uchar *p0 = val0;
      uchar *p1 = val1;
      uchar *p2 = val2;
      uchar *p  = vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
  else if (voxelType == _Char)
    {
      char *p0 = (char*)val0;
      char *p1 = (char*)val1;
      char *p2 = (char*)val2;
      char *p  = (char*)vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
  else if (voxelType == _UShort)
    {
      ushort *p0 = (ushort*)val0;
      ushort *p1 = (ushort*)val1;
      ushort *p2 = (ushort*)val2;
      ushort *p  = (ushort*)vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
  else if (voxelType == _Short)
    {
      short *p0 = (short*)val0;
      short *p1 = (short*)val1;
      short *p2 = (short*)val2;
      short *p  = (short*)vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
  else if (voxelType == _Int)
    {
      int *p0 = (int*)val0;
      int *p1 = (int*)val1;
      int *p2 = (int*)val2;
      int *p  = (int*)vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
  else if (voxelType == _Float)
    {
      float *p0 = (float*)val0;
      float *p1 = (float*)val1;
      float *p2 = (float*)val2;
      float *p  = (float*)vg;
      if (filter == _MeanFilter)
	MEANFILTER()
      else if (filter == _MedianFilter)
	MEDIANFILTER()
//      else if (filter == _BilateralFilter)
//	BILATERALFILTER(dsigma)
    }
}

void
Raw2Pvl::applyVolumeFilter(QString rawFile,
			   QString smoothFile,
			   int voxelType,
			   int headerBytes,
			   int depth,
			   int width,
			   int height,
			   int filter)
{
  QString ftxt;
//  if (filter == _BilateralFilter)
//    {
//      if (dsigma > 0.0)
//	ftxt = QString("Applying Bilateral Filter (sigma-D = %1)").arg(dsigma);
//      else
//	filter = _MeanFilter;
//    }

  if (filter == _MeanFilter)
    ftxt = "Applying Gaussian Filter";
  else if (filter == _MedianFilter)
    ftxt = "Applying Median Filter";
    
  QProgressDialog progress(ftxt,
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  QFile fin(rawFile);
  fin.open(QFile::ReadOnly);

  QFile fout(smoothFile);
  fout.open(QFile::WriteOnly);

  if (headerBytes > 0)
    {
      uchar *hb = new uchar[headerBytes];  
      fin.read((char*)hb, headerBytes);
      fout.write((char*)hb, headerBytes);
      delete [] hb;
    }

  int bpv = 1;
  if (voxelType == _UChar) bpv = 1;
  else if (voxelType == _Char) bpv = 1;
  else if (voxelType == _UShort) bpv = 2;
  else if (voxelType == _Short) bpv = 2;
  else if (voxelType == _Int) bpv = 4;
  else if (voxelType == _Float) bpv = 4;

  int nbytes = width*height*bpv;
  uchar *val0 = new uchar[nbytes];
  uchar *val1 = new uchar[nbytes];
  uchar *val2 = new uchar[nbytes];
  uchar *vg   = new uchar[nbytes];

  fin.read((char*)val1, nbytes);
  fin.read((char*)val2, nbytes);
  memcpy(val0, val1, nbytes);

  applyFilter(val0, val1, val2, vg,
	      voxelType, width, height,
	      filter);
  fout.write((char*)vg, nbytes);

  for(uint d=1; d<depth-1; d++)
    {
      progress.setValue((int)(100.0*(float)d/(float)depth));
      qApp->processEvents();

      // shift the planes
      uchar *tmp = val0;
      val0 = val1;
      val1 = val2;
      val2 = tmp;
    
      fin.read((char*)val2, nbytes);

      applyFilter(val0, val1, val2, vg,
		  voxelType, width, height,
		  filter);
      fout.write((char*)vg, nbytes);
    }
  fin.close();
  
  // shift the planes
  uchar *tmp = val0;
  val0 = val1;
  val1 = val2;
  val2 = tmp;
  memcpy(val2, val1, nbytes);

  applyFilter(val0, val1, val2, vg,
	      voxelType, width, height,
	      filter);
  fout.write((char*)vg, nbytes);

  fout.close();

  delete [] val0;
  delete [] val1;
  delete [] val2;
  delete [] vg;

  progress.setValue(100);
  qApp->processEvents();
}


#define MEANSLICEFILTER(T)						\
  {									\
    T *ptr = (T*)tmp;							\
    T *ptr1 = (T*)tmp1;							\
									\
    int ji=0;								\
    for(int j=0; j<sw; j++)						\
      {									\
	int y = j*subSamplingLevel;					\
	int loy = qMax(0, y-subSamplingLevel+1);			\
	int hiy = qMin(width-1, y+subSamplingLevel-1);			\
	for(int i=0; i<sh; i++)						\
	  {								\
	    int x = i*subSamplingLevel;					\
	    int lox = qMax(0, x-subSamplingLevel+1);			\
	    int hix = qMin(height-1, x+subSamplingLevel-1);		\
									\
	    int dn = 0;							\
	    float sum = 0;						\
	    for(int jy=loy; jy<=hiy; jy++)				\
	      {								\
		for(int ix=lox; ix<=hix; ix++)				\
		  {							\
		    int idx = jy*height+ix;				\
		    float v = ptr[idx];					\
		    int ddn = 1;					\
		    if (jy == y)					\
		      {							\
			v *= 4;						\
			ddn *= 4;					\
		      }							\
		    if (ix == x)					\
		      {							\
			v *= 4;						\
			ddn *= 4;					\
		      }							\
		    sum += v;						\
		    dn += ddn;						\
		  }							\
	      }								\
	    ptr1[ji] = sum/dn;						\
	    ji++;							\
	  }								\
      }									\
									\
    uchar *vptr = volX[0];						\
    for (int c=0; c<totcount-1; c++)					\
      volX[c] = volX[c+1];						\
    volX[totcount-1] = vptr;						\
									\
    memcpy(volX[totcount-1], tmp1, bpv*sw*sh);				\
  }


#define MEDIANSLICEFILTER(T)						\
  {									\
    T *ptr = (T*)tmp;							\
    T *ptr1 = (T*)tmp1;							\
									\
    int ji=0;								\
    for(int j=0; j<sw; j++)						\
      {									\
	int y = j*subSamplingLevel;					\
	int loy = qMax(0, y-subSamplingLevel+1);			\
	int hiy = qMin(width-1, y+subSamplingLevel-1);			\
	for(int i=0; i<sh; i++)						\
	  {								\
	    int x = i*subSamplingLevel;					\
	    int lox = qMax(0, x-subSamplingLevel+1);			\
	    int hix = qMin(height-1, x+subSamplingLevel-1);		\
									\
	    QList<T> vlist;						\
	    for(int jy=loy; jy<=hiy; jy++)				\
	      {								\
		for(int ix=lox; ix<=hix; ix++)				\
		  {							\
		    int idx = jy*height+ix;				\
		    vlist.append(ptr[idx]);				\
		  }							\
	      }								\
	    qSort(vlist.begin(), vlist.end());				\
	    ptr1[ji] = vlist[vlist.size()/2];				\
	    ji++;							\
	  }								\
      }									\
									\
    uchar *vptr = volX[0];						\
    for (int c=0; c<totcount-1; c++)					\
      volX[c] = volX[c+1];						\
    volX[totcount-1] = vptr;						\
									\
    memcpy(volX[totcount-1], tmp1, bpv*sw*sh);				\
  }


#define BILATERALSLICEFILTER(T)						\
  {									\
    T *ptr = (T*)tmp;							\
    T *ptr1 = (T*)tmp1;							\
									\
    int ji=0;								\
    for(int j=0; j<sw; j++)						\
      {									\
	int y = j*subSamplingLevel;					\
	int loy = qMax(0, y-subSamplingLevel+1);			\
	int hiy = qMin(width-1, y+subSamplingLevel-1);			\
	for(int i=0; i<sh; i++)						\
	  {								\
	    int x = i*subSamplingLevel;					\
	    int lox = qMax(0, x-subSamplingLevel+1);			\
	    int hix = qMin(height-1, x+subSamplingLevel-1);		\
									\
	    float vmin = ptr[loy*height+lox];				\
	    float vmax = ptr[loy*height+lox];				\
	    for(int jy=loy; jy<=hiy; jy++)				\
	      {								\
		for(int ix=lox; ix<=hix; ix++)				\
		  {							\
		    if (jy!=y || ix!=x)					\
		      {							\
			int idx = jy*height+ix;				\
			vmin = qMin(vmin, (float)ptr[idx]);		\
			vmax = qMax(vmax, (float)ptr[idx]);		\
		      }							\
		  }							\
	      }								\
	    int idx = y*height+x;					\
	    ptr1[ji] = qBound(vmin, (float)ptr[idx], vmax);		\
	    ji++;							\
	  }								\
      }									\
									\
    uchar *vptr = volX[0];						\
    for (int c=0; c<totcount-1; c++)					\
      volX[c] = volX[c+1];						\
    volX[totcount-1] = vptr;						\
									\
    memcpy(volX[totcount-1], tmp1, bpv*sw*sh);				\
  }

#define NOSLICEFILTER(T)						\
  {									\
    T *ptr = (T*)tmp;							\
    T *ptr1 = (T*)tmp1;							\
									\
    int ji=0;								\
    for(int j=0; j<sw; j++)						\
      {									\
	int y = j*subSamplingLevel;					\
	for(int i=0; i<sh; i++)						\
	  {								\
	    int x = i*subSamplingLevel;					\
	    int idx = y*height+x;					\
	    ptr1[ji] = ptr[idx];					\
	    ji++;							\
	  }								\
      }									\
									\
    uchar *vptr = volX[0];						\
    for (int c=0; c<totcount-1; c++)					\
      volX[c] = volX[c+1];						\
    volX[totcount-1] = vptr;						\
									\
    memcpy(volX[totcount-1], tmp1, bpv*sw*sh);				\
  }



#define MEANSUBSAMPLE(T)			\
  {						\
    memset(tmp1f, 0, 4*width*height);		\
    memset(tmp1, 0, bpv*sw*sh);			\
						\
    T *ptr1 = (T*)tmp1;				\
    T **volS;					\
    volS = new T*[totcount];			\
    for(uint i=0; i<totcount; i++)		\
      volS[i] = (T*)volX[i];			\
    						\
    for(int x=0; x<totcount; x++)		\
      for(int j=0; j<sw*sh; j++)		\
	tmp1f[j]+=volS[x][j];			\
    for(int j=0; j<sw*sh; j++)			\
      ptr1[j] = tmp1f[j]/totcount;		\
    						\
    delete [] volS;				\
  }


#define MEDIANSUBSAMPLE(T)			\
  {						\
    memset(tmp1, 0, bpv*sw*sh);			\
						\
    T *ptr1 = (T*)tmp1;				\
						\
    T **volS;					\
    volS = new T*[totcount];			\
    for(uint i=0; i<totcount; i++)		\
      volS[i] = (T*)volX[i];			\
						\
    for(int j=0; j<sw*sh; j++)			\
      {						\
	QList<T> vlist;				\
	for(int x=0; x<totcount; x++)		\
	  vlist.append(volS[x][j]);		\
	qSort(vlist.begin(), vlist.end());	\
	ptr1[j] = vlist[vlist.size()/2];	\
      }						\
    						\
    delete [] volS;				\
  }


#define BILATERALSUBSAMPLE(T)			  \
  {							  \
    memset(tmp1, 0, bpv*sw*sh);				  \
    T *ptr1 = (T*)tmp1;					  \
    T **volS;						  \
    volS = new T*[totcount];				  \
    for(uint i=0; i<totcount; i++)			  \
      volS[i] = (T*)volX[i];				  \
    							  \
    int tc2 = totcount/2;				  \
    for(int j=0; j<sw*sh; j++)				  \
      {							  \
	float vmin = volS[0][0];			  \
	float vmax = volS[0][0];			  \
	for(int x=0; x<tc2; x++)			  \
	  {						  \
	    vmin = qMin(vmin, (float)volS[x][j]);	  \
	    vmax = qMax(vmax, (float)volS[x][j]);	  \
	  }						  \
	for(int x=tc2+1; x<totcount; x++)		  \
	  {						  \
	    vmin = qMin(vmin, (float)volS[x][j]);	  \
	    vmax = qMax(vmax, (float)volS[x][j]);	  \
	  }						  \
	ptr1[j] = qBound(vmin, (float)volS[tc2][j], vmax);\
      }							  \
    							  \
    delete [] volS;					  \
  }


#define SUBSAMPLE(T)				\
  {						\
    int x = totcount/2;				\
    memcpy(tmp1, volX[x], bpv*sw*sh);		\
  }

void
Raw2Pvl::subsampleVolume(QString rawFilename,
			 QString subsampledFilename,
			 int voxelType, int headerBytes,
			 int depth, int width, int height,
			 int subSamplingLevel, int filter)
{
  QProgressDialog progress(QString("Subsampling %1").arg(rawFilename),
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);


  int sd = StaticFunctions::getScaledown(subSamplingLevel, depth);
  int sw = width/subSamplingLevel;
  int sh = height/subSamplingLevel;

  QFile fout(subsampledFilename);
  fout.open(QFile::WriteOnly);
  uchar svt = voxelType;

  if (svt == 5) // _Float
    svt = 8;

  fout.write((char*)&svt, 1);
  fout.write((char*)&sd, sizeof(int));
  fout.write((char*)&sw, sizeof(int));
  fout.write((char*)&sh, sizeof(int));

  QFile fin(rawFilename);
  fin.open(QFile::ReadOnly);
  fin.seek(headerBytes);

  int bpv = 1;
  if (voxelType == _UChar) bpv = 1;
  else if (voxelType == _Char) bpv = 1;
  else if (voxelType == _UShort) bpv = 2;
  else if (voxelType == _Short) bpv = 2;
  else if (voxelType == _Int) bpv = 4;
  else if (voxelType == _Float) bpv = 4;

  int nbytes = width*height*bpv;
  uchar *tmp = new uchar[nbytes];
  uchar *tmp1 = new uchar [nbytes];  
  float *tmp1f = new float [width*height];

  int totcount = 2*subSamplingLevel-1;
  uchar **volX;
  volX = 0;
  if (totcount > 1)
    {
      volX = new uchar*[totcount];
      for(int i=0; i<totcount; i++)
	volX[i] = new uchar[nbytes];
    }

  int count=0;
  int kslc=0;
  for(uint k=0; k<depth; k++)
    {
      progress.setValue((int)(100.0*(float)k/(float)depth));
      qApp->processEvents();
      
      fin.read((char*)tmp, nbytes);

      // apply filter and scaledown the slice
      if (voxelType == _UChar)
	{
	  if (filter == _NoFilter)
	      NOSLICEFILTER(uchar)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(uchar)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(uchar)
	  else
	    MEDIANSLICEFILTER(uchar)
	}
      else if (voxelType == _Char)
	{
	  if (filter == _NoFilter)
	    NOSLICEFILTER(char)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(char)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(char)
	  else
	    MEDIANSLICEFILTER(char)
	}
      else if (voxelType == _UShort)
	{
	  if (filter == _NoFilter)
	    NOSLICEFILTER(ushort)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(ushort)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(ushort)
	  else
	    MEDIANSLICEFILTER(ushort)
	}
      else if (voxelType == _Short)
	{
	  if (filter == _NoFilter)
	    NOSLICEFILTER(short)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(short)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(short)
	  else
	    MEDIANSLICEFILTER(short)
	}
      else if (voxelType == _Int)
	{
	  if (filter == _NoFilter)
	    NOSLICEFILTER(int)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(int)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(int)
	  else
	    MEDIANSLICEFILTER(int)
	}
      else if (voxelType == _Float)
	{
	  if (filter == _NoFilter)
	    NOSLICEFILTER(float)
	  else if (filter == _BilateralFilter)
	    BILATERALSLICEFILTER(float)
	  else if (filter == _MeanFilter)
	    MEANSLICEFILTER(float)
	  else
	    MEDIANSLICEFILTER(float)
	}

      count ++;

      if (count == totcount)
	{
	  if (voxelType == _UChar)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(uchar)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(uchar)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(uchar)	    
	      else
		MEDIANSUBSAMPLE(uchar)
          }
	  else if (voxelType == _Char)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(char)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(char)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(char)	    
	      else
		MEDIANSUBSAMPLE(char)
	    }
	  else if (voxelType == _UShort)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(ushort)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(ushort)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(ushort)	    
	      else
		MEDIANSUBSAMPLE(ushort)
	    }
	  else if (voxelType == _Short)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(short)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(short)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(short)	    
	      else
		MEDIANSUBSAMPLE(short)
	    }
	  else if (voxelType == _Int)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(int)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(int)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(int)	    
	      else
		MEDIANSUBSAMPLE(int)
	    }
	  else if (voxelType == _Float)
	    {
	      if (filter == _NoFilter)
		SUBSAMPLE(float)
	      else if (filter == _BilateralFilter)
		BILATERALSUBSAMPLE(float)
	      else if (filter == _MeanFilter)
		MEANSUBSAMPLE(float)	    
	      else
		MEDIANSUBSAMPLE(float)
	    }

	  count = totcount/2;
	  
	  // save subsampled slice
	  fout.write((char*)tmp1, bpv*sw*sh);

	  // increment slice number
	  kslc++;
	  if (kslc == sd)
	    break;
	}
    }

  delete [] tmp;
  delete [] tmp1;
  delete [] tmp1f;
  if (totcount > 1)
    {
      for(int i=0; i<totcount; i++)
	delete [] volX[i];
      delete [] volX;
    }

  fin.close();
  fout.close();

  progress.setValue(100);
}

//-----------------------------
QString
getPvlNcFilename()
{
  QString pvlFilename = QFileDialog::getSaveFileName(0,
					 "Save processed volume",
					 Global::previousDirectory(),
					 "NetCDF Files (*.bvf)");
  if (pvlFilename.isEmpty())
    return "";

  if (pvlFilename.endsWith(".bvf.bvf"))
    pvlFilename.chop(7);

  if(!pvlFilename.endsWith(".bvf"))
    pvlFilename += ".bvf";
  
  return pvlFilename;
}

int
getBlockSize()
{
  int blocksize = 32;
  
  bool ok = false;
  QStringList bsz;
  bsz << "32";
  bsz << "64";
  bsz << "128";
  QString option = QInputDialog::getItem(0,
		   "Block Size",
		   "Block size for preprocessed volume",
		   bsz,
		   0,
		   false,
		   &ok);
  if (ok)
    blocksize = option.toInt();

  QMessageBox::information(0, "Block Size", QString("Set block size to %1").arg(blocksize));

  return blocksize;
}

bool
getSaveRawFile()
{
  bool saveRawFile = false;
  bool ok = false;
  QStringList slevels;
  slevels << "Yes - save raw file";
  slevels << "No";  
  QString option = QInputDialog::getItem(0,
		   "Save Processed Volume",
		   "Save RAW file along with preprocessed volume ?",
		    slevels,
			  0,
		      false,
		       &ok);
  if (ok)
    {
      QStringList op = option.split(' ');
      if (op[0] == "Yes")
	saveRawFile = true;
    }
  else
    QMessageBox::information(0, "RAW Volume", "Will not save raw volume");

  return saveRawFile;
}

QString
getRawFilename(QString pvlFilename)
{
//  QString rawfile = QFileDialog::getSaveFileName(0,
//				 "Save processed volume",
//				 QFileInfo(pvlFilename).absolutePath(),
//				 "RAW Files (*.raw)");
  QString rawfile = pvlFilename;
  rawfile.chop(3);
  rawfile += "rawbvf";
  return rawfile;
}

int
getZSubsampling(int dsz, int wsz, int hsz)
{
  bool ok = false;
  QStringList slevels;

  slevels.clear();
  slevels << "No subsampling in Z";
  slevels << QString("2 [Z(%1) %2 %3]").arg(dsz/2).arg(wsz).arg(hsz);
  slevels << QString("3 [Z(%1) %2 %3]").arg(dsz/3).arg(wsz).arg(hsz);
  slevels << QString("4 [Z(%1) %2 %3]").arg(dsz/4).arg(wsz).arg(hsz);
  slevels << QString("5 [Z(%1) %2 %3]").arg(dsz/5).arg(wsz).arg(hsz);
  slevels << QString("6 [Z(%1) %2 %3]").arg(dsz/6).arg(wsz).arg(hsz);
  QString option = QInputDialog::getItem(0,
					 "Volume Size",
					 "Z subsampling",
					 slevels,
					 0,
					 false,
					 &ok);
  int svslz = 1;
  if (ok)
    {   
      QStringList op = option.split(' ');
      svslz = qMax(1, op[0].toInt());
    }
  return svslz;
}

int
getXYSubsampling(int svslz, int dsz, int wsz, int hsz)
{
  bool ok = false;
  QStringList slevels;

  slevels.clear();
  slevels << "No subsampling in XY";
  slevels << QString("2 [%1 Y(%2) X(%3)]").arg(dsz/svslz).arg(wsz/2).arg(hsz/2);
  slevels << QString("3 [%1 Y(%2) X(%3)]").arg(dsz/svslz).arg(wsz/3).arg(hsz/3);
  slevels << QString("4 [%1 Y(%2) X(%3)]").arg(dsz/svslz).arg(wsz/4).arg(hsz/4);
  slevels << QString("5 [%1 Y(%2) X(%3)]").arg(dsz/svslz).arg(wsz/5).arg(hsz/5);
  slevels << QString("6 [%1 Y(%2) X(%3)]").arg(dsz/svslz).arg(wsz/6).arg(hsz/6);
  QString option = QInputDialog::getItem(0,
					 "Volume Size",
					 "XY subsampling",
					 slevels,
					 0,
					 false,
					 &ok);
  int svsl = 1;
  if (ok)
    {   
      QStringList op = option.split(' ');
      svsl = qMax(1, op[0].toInt());
    }
  return svsl;
}

#define AVERAGEFILTER(n)			\
  {						\
    for(int j=0; j<width; j++)			\
      for(int k=0; k<height; k++)		\
	{					\
	  int js = qMax(0, j-n);		\
	  int je = qMin(width-n, j+n);		\
	  int ks = qMax(0, k-n);		\
	  int ke = qMin(height-n, k+n);		\
						\
	  int dn = 0;				\
	  float avg = 0;			\
	  for(int i=0; i<2*n+1; i++)		\
	    for(int j1=js; j1<=je; j1++)	\
	      for(int k1=ks; k1<=ke; k1++)	\
		{				\
		  int idx = j1*height+k1;	\
		  avg += pv[i][idx]; 		\
		  dn ++;			\
		}				\
	  avg = avg/dn;				\
	  p[j*height + k] = avg;		\
	}					\
  }

void
Raw2Pvl::applyMeanFilter(uchar **val, uchar *vg,
			 int voxelType,
			 int width, int height,
			 int spread)
{
  if (voxelType == _UChar)
    {
      uchar **pv = val;
      uchar *p  = vg;
      AVERAGEFILTER(spread)
    }
  else if (voxelType == _Char)
    {
      char **pv = (char**)val;
      char *p  = (char*)vg;
      AVERAGEFILTER(spread)
    }
  else if (voxelType == _UShort)
    {
      ushort **pv = (ushort**)val;
      ushort *p  = (ushort*)vg;
      AVERAGEFILTER(spread)
    }
  else if (voxelType == _Short)
    {
      short **pv = (short**)val;
      short *p  = (short*)vg;
      AVERAGEFILTER(spread)
    }
  else if (voxelType == _Int)
    {
      int **pv = (int**)val;
      int *p  = (int*)vg;
      AVERAGEFILTER(spread)
    }
  else if (voxelType == _Float)
    {
      float **pv = (float**)val;
      float *p  = (float*)vg;
      AVERAGEFILTER(spread)
    }
}


void
Raw2Pvl::savePvlHeader(QString pvlFilename,
		       bool saveRawFile, QString rawfile,
		       int voxelType, int voxelUnit,
		       int d, int w, int h,
		       float vx, float vy, float vz,
		       QList<float> rawMap, QList<uchar> pvlMap,
		       QString description,
		       long maxFileSize, int blockSize, int border)
{
  QString xmlfile = pvlFilename;

  QDomDocument doc("Drishti_Header");

  QDomElement topElement = doc.createElement("PvlDotNcFileHeader");
  doc.appendChild(topElement);

  {      
    QString vstr;
    if (saveRawFile)
      {
	// save relative path for the rawfile
	QFileInfo fileInfo(pvlFilename);
	QDir direc = fileInfo.absoluteDir();
	vstr = direc.relativeFilePath(rawfile);
      }
    else
      vstr = "";

    QDomElement de0 = doc.createElement("rawfile");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
      
  {      
    QString vstr;
    if (voxelType == Raw2Pvl::_UChar)      vstr = "unsigned char";
    else if (voxelType == Raw2Pvl::_Char)  vstr = "char";
    else if (voxelType == Raw2Pvl::_UShort)vstr = "unsigned short";
    else if (voxelType == Raw2Pvl::_Short) vstr = "short";
    else if (voxelType == Raw2Pvl::_Int)   vstr = "int";
    else if (voxelType == Raw2Pvl::_Float) vstr = "float";
    
    QDomElement de0 = doc.createElement("voxeltype");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }


  {      
    QDomElement de0 = doc.createElement("gridsize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1 %2 %3").arg(d).arg(w).arg(h));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QString vstr;
    if (voxelUnit == Raw2Pvl::_Nounit)         vstr = "no units";
    else if (voxelUnit == Raw2Pvl::_Angstrom)  vstr = "angstrom";
    else if (voxelUnit == Raw2Pvl::_Nanometer) vstr = "nanometer";
    else if (voxelUnit == Raw2Pvl::_Micron)    vstr = "micron";
    else if (voxelUnit == Raw2Pvl::_Millimeter)vstr = "millimeter";
    else if (voxelUnit == Raw2Pvl::_Centimeter)vstr = "centimeter";
    else if (voxelUnit == Raw2Pvl::_Meter)     vstr = "meter";
    else if (voxelUnit == Raw2Pvl::_Kilometer) vstr = "kilometer";
    else if (voxelUnit == Raw2Pvl::_Parsec)    vstr = "parsec";
    else if (voxelUnit == Raw2Pvl::_Kiloparsec)vstr = "kiloparsec";
    
    QDomElement de0 = doc.createElement("voxelunit");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QDomElement de0 = doc.createElement("voxelsize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1 %2 %3").arg(vx).arg(vy).arg(vz));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
  
  {
    QString vstr = description.trimmed();
    QDomElement de0 = doc.createElement("description");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QDomElement de0 = doc.createElement("maxfilesize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(maxFileSize));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
  {      
    QDomElement de0 = doc.createElement("blocksize");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(blockSize));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
  {      
    QDomElement de0 = doc.createElement("border");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(border));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }
  
  {      
    QString vstr;
    for(int i=0; i<rawMap.size(); i++)
      vstr += QString("%1 ").arg(rawMap[i]);
    
    QDomElement de0 = doc.createElement("rawmap");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
    de0.appendChild(tn0);
    topElement.appendChild(de0);
  }

  {      
    QString vstr;
    for(int i=0; i<pvlMap.size(); i++)
      vstr += QString("%1 ").arg(pvlMap[i]);
    
    QDomElement de0 = doc.createElement("pvlmap");
    QDomText tn0;
    tn0 = doc.createTextNode(QString("%1").arg(vstr));
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
Raw2Pvl::savePvl(AbstractRemapVolume* remapVolume,
		 int volumeType,
		 int dmin, int dmax,
		 int wmin, int wmax,
		 int hmin, int hmax,
		 QStringList timeseriesFiles)
{
  //------------------------------------------------------
  int rvdepth, rvwidth, rvheight;    
  remapVolume->gridSize(rvdepth, rvwidth, rvheight);

  int dsz=dmax-dmin+1;
  int wsz=wmax-wmin+1;
  int hsz=hmax-hmin+1;
  
  uchar voxelType = remapVolume->voxelType();  
  int headerBytes = remapVolume->headerBytes();

  int bpv = 1;
  if (voxelType == _UChar) bpv = 1;
  else if (voxelType == _Char) bpv = 1;
  else if (voxelType == _UShort) bpv = 2;
  else if (voxelType == _Short) bpv = 2;
  else if (voxelType == _Int) bpv = 4;
  else if (voxelType == _Float) bpv = 4;

  //*** max 1Gb per slab
//  int slabSize;
//  slabSize = (1024*1024*1024)/(bpv*wsz*hsz);
  //------------------------------------------------------

  QString pvlFilename = getPvlNcFilename();
  if (pvlFilename.count() < 5)
    {
      QMessageBox::information(0, "bvf", "No .bvf filename chosen.");
      return;
    }

  bool saveRawFile = getSaveRawFile();;
  QString rawfile;
  if (saveRawFile) rawfile = getRawFilename(pvlFilename);
  if (rawfile.isEmpty())
    saveRawFile = false;

  int svslz = getZSubsampling(dsz, wsz, hsz);
  int svsl = getXYSubsampling(svslz, dsz, wsz, hsz);

  int dsz2 = dsz/svslz;
  int wsz2 = wsz/svsl;
  int hsz2 = hsz/svsl;
  int svsl3 = svslz*svsl*svsl;
  //------------------------------------------------------

  //------------------------------------------------------
  QString vstr;

  // -- get saving parameters for processed file
  SavePvlDialog savePvlDialog;
  if (volumeType == 1)
    {
      float vx, vy, vz;
      remapVolume->voxelSize(vx, vy, vz);
      QString desc = remapVolume->description();
      savePvlDialog.setVoxelUnit(Raw2Pvl::_Micron);
      savePvlDialog.setVoxelSize(vx, vy, vz);
      savePvlDialog.setDescription(desc);
    }
  else if (volumeType == 2)
    {
      float vx, vy, vz;
      remapVolume->voxelSize(vx, vy, vz);
      savePvlDialog.setVoxelSize(vx, vy, vz);
    }

  savePvlDialog.exec();

  int spread = savePvlDialog.volumeFilter();
  int voxelUnit = savePvlDialog.voxelUnit();
  QString description = savePvlDialog.description();
  float vx, vy, vz;
  savePvlDialog.voxelSize(vx, vy, vz);

  QList<float> rawMap = remapVolume->rawMap();
  QList<uchar> pvlMap = remapVolume->pvlMap();

  int nbytes = rvwidth*rvheight*bpv;
  double *filtervol = new double[wsz2*hsz2];
  uchar *pvl = new uchar[wsz2*hsz2];
  uchar *raw = new uchar[nbytes];
  uchar *duppvl = new uchar[4*wsz2*hsz2];
  uchar **val;
  if (spread > 0)
    {
      val = new uchar*[2*spread+1];
      for (int i=0; i<2*spread+1; i++)
	val[i] = new uchar[nbytes];
    }
  int rawSize = rawMap.size()-1;
  int width = wsz2;
  int height = hsz2;
  bool subsample = (svsl > 1 || svslz > 1);
  bool trim = (dmin != 0 ||
	       wmin != 0 ||
	       hmin != 0 ||
	       dsz2 != rvdepth ||
	       wsz2 != rvwidth ||
	       hsz2 != rvheight);


  BlockFileWriter rawFileManager;
  BlockFileWriter pvlFileManager;

  QProgressDialog progress("Saving processed volume",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  //------------------------------------------------------
  int tsfcount = qMax(1, timeseriesFiles.count());
  for (int tsf=0; tsf<tsfcount; tsf++)
    {
      QString pvlflnm = pvlFilename;
      QString rawflnm = rawfile;

      if (tsfcount > 1)
	{
	  QFileInfo ftpvl(pvlFilename);
	  QFileInfo ftraw(timeseriesFiles[tsf]);
	  pvlflnm = QFileInfo(ftpvl.absolutePath(),
			      ftraw.completeBaseName() + ".bvf").absoluteFilePath();

	  rawflnm = QFileInfo(ftpvl.absolutePath(),
			      ftraw.completeBaseName() + ".raw").absoluteFilePath();

	  remapVolume->replaceFile(timeseriesFiles[tsf]);
	}

      int blocksize = getBlockSize();

      pvlFileManager.setBaseFilename(pvlflnm);
      pvlFileManager.setDepth(2*dsz2);
      pvlFileManager.setWidth(2*wsz2);
      pvlFileManager.setHeight(2*hsz2);
      pvlFileManager.setVoxelType(0); // uchar
      pvlFileManager.setHeaderSize(0);
      pvlFileManager.setBlockSize(blocksize);
      pvlFileManager.createFile(true);
      
      if (saveRawFile)
	{
	  rawFileManager.setBaseFilename(rawflnm);
	  rawFileManager.setDepth(dsz2);
	  rawFileManager.setWidth(wsz2);
	  rawFileManager.setHeight(hsz2);
	  rawFileManager.setVoxelType(voxelType);
	  rawFileManager.setHeaderSize(0);
	  rawFileManager.setBlockSize(blocksize);
	  rawFileManager.createFile(true);
	}
      //------------------------------------------------------


      savePvlHeader(pvlflnm,
		    saveRawFile, rawflnm,
		    voxelType, voxelUnit,
		    2*dsz2, 2*wsz2, 2*hsz2,
		    vx, vy, vz,
		    rawMap, pvlMap,
		    description,
		    pvlFileManager.maxFileSize(),
		    pvlFileManager.blockSize(),
		    0);
      
      pvlFileManager.startAddSlice();

      if (saveRawFile)
	{
	  savePvlHeader(rawflnm,
			false, "",
			voxelType, voxelUnit,
			dsz2, wsz2, hsz2,
			vx, vy, vz,
			rawMap, pvlMap,
			description,
			pvlFileManager.maxFileSize(),
			pvlFileManager.blockSize(),
			0);	  
	  rawFileManager.startAddSlice();
	}

      for(int dd2=0; dd2<2*dsz2; dd2++)
	{
	  int dd = (dd2 < dsz2) ? dd2 : dd2-dsz2;
	  int d0 = dmin + dd*svslz; 
	  int d1 = d0 + svslz-1;	
  
	  progress.setValue((int)(100*(float)dd2/(float)(2*dsz2)));
	  qApp->processEvents();
	  
	  memset(filtervol, 0, 8*wsz2*hsz2);
	  for (int d=d0; d<=d1; d++)
	    {
	      if (spread > 0)
		{
		  if (d == d0)
		    {
		      remapVolume->getDepthSlice(d, val[spread]);
		      for(int i=-spread; i<0; i++)
			{
			  if (d+i >= 0)
			    remapVolume->getDepthSlice(d+i, val[spread+i]);
			  else
			    remapVolume->getDepthSlice(0, val[spread+i]);
			}
		      
		      for(int i=1; i<=spread; i++)
			{
			  if (d+i < rvdepth)
			    remapVolume->getDepthSlice(d+i, val[spread+i]);
			  else
			    remapVolume->getDepthSlice(rvdepth-1, val[spread+i]);
			}
		    }
		  else if (d < rvdepth-spread)
		    remapVolume->getDepthSlice(d+spread, val[2*spread]);
		  else
		    remapVolume->getDepthSlice(rvdepth-1, val[2*spread]);
		}
	      else
		remapVolume->getDepthSlice(d, raw);
	      
	      if (spread > 0)
		{
		  applyMeanFilter(val, raw,
				  voxelType, rvwidth, rvheight,
				  spread);
		  
		  // now shift the planes
		  uchar *tmp = val[0];
		  for(int i=0; i<2*spread; i++)
		    val[i] = val[i+1];
		  val[2*spread] = tmp;
		}
	      
	      if (trim || subsample)
		{
		  int fi = 0;
		  for(int j=0; j<wsz2; j++)
		    {
		      int y0 = wmin+j*svsl;
		      int y1 = y0+svsl-1;
		      for(int i=0; i<hsz2; i++)
			{
			  int x0 = hmin+i*svsl;
			  int x1 = x0+svsl-1;
			  for(int y=y0; y<=y1; y++)
			    for(int x=x0; x<=x1; x++)
			      {
				if (voxelType == _UChar)
				  { uchar *ptr = raw; filtervol[fi] += ptr[y*rvheight+x]; }
				else if (voxelType == _Char)
				  { char *ptr = (char*)raw; filtervol[fi] += ptr[y*rvheight+x]; }
				else if (voxelType == _UShort)
				  { ushort *ptr = (ushort*)raw; filtervol[fi] += ptr[y*rvheight+x]; }
				else if (voxelType == _Short)
				  { short *ptr = (short*)raw; filtervol[fi] += ptr[y*rvheight+x]; }
				else if (voxelType == _Int)
				  { int *ptr = (int*)raw; filtervol[fi] += ptr[y*rvheight+x]; }
				else if (voxelType == _Float)
				  { float *ptr = (float*)raw; filtervol[fi] += ptr[y*rvheight+x]; }
			      }
			  fi++;
			}
		    }
		} // trim || subsample
	    }
	  
	  if (trim || subsample)
	    {
	      if (subsample)
		{
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    filtervol[fi] /= svsl3;
		}
	      
	      if (voxelType == _UChar)
		{
		  uchar *ptr = raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	      else if (voxelType == _Char)
		{
		  char *ptr = (char*)raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	      else if (voxelType == _UShort)
		{
		  ushort *ptr = (ushort*)raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	      else if (voxelType == _Short)
		{
		  short *ptr = (short*)raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	      else if (voxelType == _Int)
		{
		  int *ptr = (int*)raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	      else if (voxelType == _Float)
		{
		  float *ptr = (float*)raw;
		  for(int fi=0; fi<wsz2*hsz2; fi++)
		    ptr[fi] = filtervol[fi];
		}
	    } // trim || subsample
	  
	  if (saveRawFile && dd2 < dsz2)
	    rawFileManager.setSlice(dd, raw);
	  
	  
	  if (voxelType == _UChar)
	    {
	      uchar *ptr = raw;
	      REMAPVOLUME();
	    }
	  else if (voxelType == _Char)
	    {
	      char *ptr = (char*)raw;
	      REMAPVOLUME();
	    }
	  else if (voxelType == _UShort)
	    {
	      ushort *ptr = (ushort*)raw;
	      REMAPVOLUME();
	    }
	  else if (voxelType == _Short)
	    {
	      short *ptr = (short*)raw;
	      REMAPVOLUME();
	    }
	  else if (voxelType == _Int)
	    {
	      int *ptr = (int*)raw;
	      REMAPVOLUME();
	    }
	  else if (voxelType == _Float)
	    {
	      float *ptr = (float*)raw;
	      REMAPVOLUME();
	    }

	  //------------------
	  {
	    int ip = 0;
	    for(int iw=0; iw<wsz2; iw++)
	      for(int ih=0; ih<hsz2; ih++)
		{
		  duppvl[iw*(2*hsz2) + ih] = pvl[ip];
		  ip++;
		}
	    ip = 0;
	    for(int iw=wsz2; iw<2*wsz2; iw++)
	      for(int ih=0; ih<hsz2; ih++)
		{
		  duppvl[iw*(2*hsz2) + ih] = pvl[ip];
		  ip++;
		}
	    ip = 0;
	    for(int iw=wsz2; iw<2*wsz2; iw++)
	      for(int ih=hsz2; ih<2*hsz2; ih++)
		{
		  duppvl[iw*(2*hsz2) + ih] = pvl[ip];
		  ip++;
		}
	    ip = 0;
	    for(int iw=0; iw<wsz2; iw++)
	      for(int ih=hsz2; ih<2*hsz2; ih++)
		{
		  duppvl[iw*(2*hsz2) + ih] = pvl[ip];
		  ip++;
		}	  
	  }
	  //------------------
	  pvlFileManager.setSlice(dd2, duppvl);

	}

      pvlFileManager.endAddSlice();
      if (saveRawFile)
	rawFileManager.endAddSlice();
    }
  
  delete [] filtervol;
  delete [] pvl;
  delete [] duppvl;
  delete [] raw;
  if (spread > 0)
    {
      for (int i=0; i<2*spread+1; i++)
	delete [] val[i];
      delete [] val;
    }
  
  progress.setValue(100);

  QMessageBox::information(0, "Save", "-----Done-----");
}

void
Raw2Pvl::Old2New(QString oldflnm, QString direc)
{
  NcError err(NcError::verbose_nonfatal);

  NcFile pvlFile(oldflnm.toAscii().data(), NcFile::ReadOnly);

  if (!pvlFile.is_valid())
    {
      QMessageBox::information(0, "Error",
			       QString("%1 is not a valid preprocessed volume file").arg(oldflnm));
      return;
    }

  bool ok = true;
  NcVar *ncvar;
  ncvar = pvlFile.get_var("RVolume"); ok &= (ncvar != 0);
  ncvar = pvlFile.get_var("GVolume"); ok &= (ncvar != 0);
  ncvar = pvlFile.get_var("BVolume"); ok &= (ncvar != 0);  
  pvlFile.close();

  if (ok)
    Old2NewRGBA(oldflnm, direc);
  else
    Old2NewScalar(oldflnm, direc);
}

void
Raw2Pvl::Old2NewScalar(QString oldflnm, QString direc)
{
  NcError err(NcError::verbose_nonfatal);
  
  NcFile oldFile(oldflnm.toAscii().data(), NcFile::ReadOnly);
  if (!oldFile.is_valid())
    {
      QMessageBox::information(0, "Error",
	 QString("%1 is not a valid preprocessed volume file").arg(oldflnm));
      return;
    }

  QString rawFile;
  QString description;
  int voxelType;
  int voxelUnit;
  int dsz, wsz, hsz;
  float vx, vy, vz;
  QList<float> rawMap;
  QList<uchar> pvlMap;

  //--------------------------------
  {  
    int i;
    NcAtt *att;
    char *attval;
    QString pvalue;

    att = oldFile.get_att("rawfile");
    if (att)
      {
	attval = att->as_string(0);
	rawFile = attval;
	delete [] attval;
      }    
    
    att = oldFile.get_att("description");
    if (att)
      {
	attval = att->as_string(0);
	description = attval;
	delete [] attval;
      }

    att = oldFile.get_att("voxeltype");
    if (att)
      {
	attval = att->as_string(0);
	pvalue = attval;
      if (pvalue == "unsigned char")
	voxelType = Raw2Pvl::_UChar;
      if (pvalue == "char")
	voxelType = Raw2Pvl::_Char;
      if (pvalue == "unsigned short")
	voxelType = Raw2Pvl::_UShort;
      if (pvalue == "short")
	voxelType = Raw2Pvl::_Short;
      if (pvalue == "int")
	voxelType = Raw2Pvl::_Int;
      if (pvalue == "float")
	voxelType = Raw2Pvl::_Float;
      delete [] attval;
    }


  att = oldFile.get_att("voxelunit");
  if (att)
    { 
      attval = att->as_string(0);
      pvalue = attval;
      voxelUnit = Raw2Pvl::_Nounit;
      if (pvalue == "angstrom")
	voxelUnit = Raw2Pvl::_Angstrom;
      else if (pvalue == "nanometer")
	voxelUnit = Raw2Pvl::_Nanometer;
      else if (pvalue == "micron")
	voxelUnit = Raw2Pvl::_Micron;
      else if (pvalue == "millimeter")
	voxelUnit = Raw2Pvl::_Millimeter;
      else if (pvalue == "centimeter")
	voxelUnit = Raw2Pvl::_Centimeter;
      else if (pvalue == "meter")
	voxelUnit = Raw2Pvl::_Meter;
      else if (pvalue == "kilometer")
	voxelUnit = Raw2Pvl::_Kilometer;
      else if (pvalue == "parsec")
	voxelUnit = Raw2Pvl::_Parsec;
      else if (pvalue == "kiloparsec")
	voxelUnit = Raw2Pvl::_Kiloparsec;
      delete [] attval;
    }
  
  
  att = oldFile.get_att("gridsize");
  if (att)
    {
      dsz = att->as_int(0);
      wsz = att->as_int(1);
      hsz = att->as_int(2);
    }

  att = oldFile.get_att("voxelsize");
  if (att)
    {
      vx = att->as_float(0);
      vy = att->as_float(1);
      vz = att->as_float(2);
    }

  att = oldFile.get_att("mapraw");
  if (att)
    {
      for(i=0; i<att->num_vals(); i++)
	rawMap.append(att->as_float(i));
  
      att = oldFile.get_att("mappvl");
      for(i=0; i<att->num_vals(); i++)
	pvlMap.append(att->as_ncbyte(i));
    }
  }
  //--------------------------------


  bool ok = true;
  NcVar *ncvar;
  ncvar = oldFile.get_var("ProcessedVolume");
  if (!ok)
    {
      QMessageBox::information(0, "Converting",
			       "Error : Not an old-style processed volume file!");
      return;
    }

  int bpv = 1;
  if (voxelType == _UChar) bpv = 1;
  else if (voxelType == _Char) bpv = 1;
  else if (voxelType == _UShort) bpv = 2;
  else if (voxelType == _Short) bpv = 2;
  else if (voxelType == _Int) bpv = 4;
  else if (voxelType == _Float) bpv = 4;

  //--------------------------------------------------
  // -- create and start saving processed information
  //--------------------------------------------------

  QString pvlFilename;
  QFileInfo fi(oldflnm);
  QString olddir = fi.absolutePath();
  if (!direc.isEmpty() && olddir != direc)
    pvlFilename = direc + QDir::separator() + fi.fileName();
  else
    pvlFilename = olddir + QDir::separator() + "new_" + fi.fileName();

  //*** max 1Gb per slab
//  int slabSize;
//  slabSize = (1024*1024*1024)/(bpv*wsz*hsz);
  savePvlHeader(pvlFilename,
		false, "",
		voxelType, voxelUnit,
		dsz, wsz, hsz,
		vx, vy, vz,
		rawMap, pvlMap,
		description,
		1000, 32, 0);



  VolumeFileManager pvlFileManager;

  pvlFileManager.setBaseFilename(pvlFilename);
  pvlFileManager.setDepth(dsz);
  pvlFileManager.setWidth(wsz);
  pvlFileManager.setHeight(hsz);
  pvlFileManager.setVoxelType(0);
  pvlFileManager.setHeaderSize(13);
  pvlFileManager.createFile(true);

  uchar *slice = new uchar[2*wsz*hsz];

  QProgressDialog progress("Converting old-style to new-style",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  for(int d=0; d<dsz; d++)
    {
      progress.setValue((int)(100*(float)d/(float)dsz));
      qApp->processEvents();
      
      ncvar->set_cur(d, 0, 0);
      if (ncvar->type() == ncByte || ncvar->type() == ncChar)
	ncvar->get((ncbyte*)slice, 1, wsz, hsz);
      else if (ncvar->type() == ncShort)
	{
	  ncvar->get((short*)slice, 1, wsz, hsz);
	  for(int i=0; i<wsz*hsz; i++)
	    slice[i] = slice[2*i]; // throw away second byte
	}

      pvlFileManager.setSlice(d, slice);
    }
  delete [] slice;
  oldFile.close();

  progress.setValue(100);
}


void
Raw2Pvl::Old2NewRGBA(QString oldflnm, QString direc)
{
  NcError err(NcError::verbose_nonfatal);
  
  NcFile oldFile(oldflnm.toAscii().data(), NcFile::ReadOnly);
  if (!oldFile.is_valid())
    {
      QMessageBox::information(0, "Error",
	 QString("%1 is not a valid preprocessed volume file").arg(oldflnm));
      return;
    }

  QString description;
  int voxelUnit;
  int dsz, wsz, hsz;
  float vx, vy, vz;
  int nRGB = 3;

  NcVar *ncvar[4];

  //--------------------------------
  {  
    NcAtt *att;
    char *attval;
    QString pvalue;

    att = oldFile.get_att("description");
    if (att)
      {
	attval = att->as_string(0);
	description = attval;
	delete [] attval;
      }

    att = oldFile.get_att("voxelunit");
    if (att)
      { 
	attval = att->as_string(0);
	pvalue = attval;
	voxelUnit = Raw2Pvl::_Nounit;
	if (pvalue == "angstrom")
	  voxelUnit = Raw2Pvl::_Angstrom;
	else if (pvalue == "nanometer")
	  voxelUnit = Raw2Pvl::_Nanometer;
	else if (pvalue == "micron")
	  voxelUnit = Raw2Pvl::_Micron;
	else if (pvalue == "millimeter")
	  voxelUnit = Raw2Pvl::_Millimeter;
	else if (pvalue == "centimeter")
	  voxelUnit = Raw2Pvl::_Centimeter;
	else if (pvalue == "meter")
	  voxelUnit = Raw2Pvl::_Meter;
	else if (pvalue == "kilometer")
	  voxelUnit = Raw2Pvl::_Kilometer;
	else if (pvalue == "parsec")
	  voxelUnit = Raw2Pvl::_Parsec;
	else if (pvalue == "kiloparsec")
	  voxelUnit = Raw2Pvl::_Kiloparsec;
	delete [] attval;
      }
    
    
    att = oldFile.get_att("gridsize");
    if (att)
      {
	dsz = att->as_int(0);
	wsz = att->as_int(1);
	hsz = att->as_int(2);
      }
    
    att = oldFile.get_att("voxelsize");
    if (att)
      {
	vx = att->as_float(0);
	vy = att->as_float(1);
	vz = att->as_float(2);
      }

    ncvar[0] = oldFile.get_var("RVolume");
    ncvar[1] = oldFile.get_var("GVolume");
    ncvar[2] = oldFile.get_var("BVolume");
    ncvar[3] = oldFile.get_var("AVolume");
    if (ncvar[3] != 0) nRGB = 4;
  }

  //--------------------------------

  //--------------------------------------------------
  // -- create and start saving processed information
  //--------------------------------------------------

  QString pvlFilename;
  QFileInfo fi(oldflnm);
  QString olddir = fi.absolutePath();
  if (!direc.isEmpty() && olddir != direc)
    pvlFilename = direc + QDir::separator() + fi.fileName();
  else
    pvlFilename = olddir + QDir::separator() + "new_" + fi.fileName();

  int bpv = 1;

  //*** max 1Gb per slab
//  int slabSize;
//  slabSize = (1024*1024*1024)/(wsz*hsz);

  QString voxelType = "RGB";
  if (nRGB == 4) voxelType = "RGBA";

  if (nRGB == 3)
    RemapImageVolume::savePvlHeader(pvlFilename,
				    dsz, wsz, hsz,
				    "RGB",
				    1000);
  else
    RemapImageVolume::savePvlHeader(pvlFilename,
				    dsz, wsz, hsz,
				    "RGBA",
				    1000);

  VolumeFileManager rgbaFileManager[4];

  QString pvlfile = pvlFilename;
  pvlfile.chop(6);

  QString rFilename = pvlfile + QString("red");
  QString gFilename = pvlfile + QString("green");
  QString bFilename = pvlfile + QString("blue");
  QString aFilename = pvlfile + QString("alpha");

  rgbaFileManager[0].setBaseFilename(rFilename);
  rgbaFileManager[1].setBaseFilename(gFilename);
  rgbaFileManager[2].setBaseFilename(bFilename);
  rgbaFileManager[3].setBaseFilename(aFilename);
  for(int a=0; a<nRGB; a++)
    {
      rgbaFileManager[a].setDepth(dsz);
      rgbaFileManager[a].setWidth(wsz);
      rgbaFileManager[a].setHeight(hsz);
      rgbaFileManager[a].setVoxelType(0);
      rgbaFileManager[a].setHeaderSize(13);
      rgbaFileManager[a].createFile(true);
    }

  uchar *slice = new uchar[2*wsz*hsz];

  QProgressDialog progress("Converting old-style to new-style",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  for(int a=0; a<nRGB; a++)
    {
      for(int d=0; d<dsz; d++)
	{
	  progress.setValue((int)(100*(float)d/(float)dsz));
	  qApp->processEvents();
	  
	  ncvar[a]->set_cur(d, 0, 0);
	  ncvar[a]->get((ncbyte*)slice, 1, wsz, hsz);
	  
	  rgbaFileManager[a].setSlice(d, slice);
	}
    }

  delete [] slice;
  oldFile.close();

  progress.setValue(100);
}

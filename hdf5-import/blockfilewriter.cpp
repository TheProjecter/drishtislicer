#include <QtGui>

#include "common.h"
#include "blockfilewriter.h"

BlockFileWriter::BlockFileWriter()
{
  m_dumpSlice = false;
  m_slice = 0;
  m_sliceAcc = 0;
  m_minmaxvals = 0;
  m_localHistogram = 0;
  m_rmsValues = 0;

  m_maxFileSize = 1024*1024*1024;
  m_blockSize = 32;

  m_minLevel = 2;

  m_hdf5file = 0;

  reset();
}

BlockFileWriter::~BlockFileWriter() { reset(); }

void
BlockFileWriter::reset()
{
  if (m_hdf5file)
    m_hdf5file->close();

  m_baseFilename.clear();
  m_header = 0;
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;

  m_totBlocks = 0;
  m_dblocks = m_wblocks = m_hblocks = 0;

  m_dumpSlice = false;
  if (m_slice)
    delete [] m_slice;
  m_slice = 0;

  if (m_sliceAcc)
    delete [] m_sliceAcc;
  m_sliceAcc = 0;

  if (m_minmaxvals)
    delete [] m_minmaxvals;
  m_minmaxvals = 0;

  if (m_localHistogram)
    delete [] m_localHistogram;
  m_localHistogram = 0;

  if (m_rmsValues)
    delete [] m_rmsValues;
  m_rmsValues = 0;
}

QString BlockFileWriter::baseFilename() { return m_baseFilename; }
int BlockFileWriter::totalBlocks() { return m_totBlocks; }
qint64 BlockFileWriter::maxFileSize() { return m_maxFileSize; }
int BlockFileWriter::blockSize() { return m_blockSize; }
int BlockFileWriter::depth() { return m_depth; }
int BlockFileWriter::width() { return m_width; }
int BlockFileWriter::height() { return m_height; }
int BlockFileWriter::dblocks() { return m_dblocks; }
int BlockFileWriter::wblocks() { return m_wblocks; }
int BlockFileWriter::hblocks() { return m_hblocks; }


void BlockFileWriter::setBaseFilename(QString bfn) { m_baseFilename = bfn; }
void BlockFileWriter::setDepth(int d) { m_depth = d; }
void BlockFileWriter::setWidth(int w) { m_width = w; }
void BlockFileWriter::setHeight(int h) { m_height = h; }
void BlockFileWriter::setHeaderSize(int hs) { m_header = hs; }
void BlockFileWriter::setMaxFileSize(qint64 bpf) { m_maxFileSize = bpf; }
void BlockFileWriter::setBlockSize(int bs) { m_blockSize = bs; }
void BlockFileWriter::setVoxelType(int vt)
{
  m_voxelType = vt;
  m_bytesPerVoxel = 1;
  if (m_voxelType == _UChar) m_bytesPerVoxel = 1;
  else if (m_voxelType == _Char) m_bytesPerVoxel = 1;
  else if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Short) m_bytesPerVoxel = 2;
  else if (m_voxelType == _Int) m_bytesPerVoxel = 4;
  else if (m_voxelType == _Float) m_bytesPerVoxel = 4;
  else if (m_voxelType == _Rgb) m_bytesPerVoxel = 3;
  else if (m_voxelType == _Rgba) m_bytesPerVoxel = 4;
}

QString BlockFileWriter::fileName()
{
  QString flnm = m_baseFilename + ".h5";
  return flnm;
}

void
BlockFileWriter::createFile(bool writeHeader)
{
  m_dblocks = ((m_depth/m_blockSize) + (m_depth%m_blockSize > 0));
  m_wblocks = ((m_width/m_blockSize) + (m_width%m_blockSize > 0));
  m_hblocks = ((m_height/m_blockSize)+ (m_height%m_blockSize > 0));
  m_dblocks = qMax(1, m_dblocks);
  m_wblocks = qMax(1, m_wblocks);
  m_hblocks = qMax(1, m_hblocks);

  m_totBlocks = m_dblocks * m_wblocks * m_hblocks;

  qint64 vsize = m_depth;
  vsize *= m_width;
  vsize *= m_height;


  //int maxvsize = 128*1024*1024;
  int maxvsize = 32*1024*1024;
  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    maxvsize = 64*1024*1024;

  m_sslevel = 1;
  while (vsize > maxvsize)
    {
      m_sslevel *= 2;
      vsize /= 8;
    }

//  m_minLevel = 0;
//  if (m_sslevel > 1)
//    m_minLevel = logf(m_sslevel)/logf(2.0)-1;
  m_minLevel = 5;

  m_ssd = m_depth/m_sslevel + (m_depth%m_sslevel > 0);
  m_ssw = m_width/m_sslevel + (m_width%m_sslevel > 0);
  m_ssh = m_height/m_sslevel+ (m_height%m_sslevel > 0);
  m_ssvol = new uchar[m_ssd*m_ssw*m_ssh*m_bytesPerVoxel];
  memset(m_ssvol, 0, m_ssd*m_ssw*m_ssh*m_bytesPerVoxel);

  m_minmaxvals = new uchar[2*m_dblocks*m_wblocks*m_hblocks];
  memset(m_minmaxvals, 0, 2*m_dblocks*m_wblocks*m_hblocks);

  int midx = 0;
  for(int d=0; d<m_dblocks; d++)
    for(int w=0; w<m_wblocks; w++)
      for(int h=0; h<m_hblocks; h++)
	{
	  m_minmaxvals[2*midx + 0] = 255;
	  m_minmaxvals[2*midx + 1] = 0;
	  midx++;
	}

  m_rmsValues = new float[m_minLevel*m_dblocks*m_wblocks*m_hblocks];
  memset(m_rmsValues, 0,4*m_minLevel*m_dblocks*m_wblocks*m_hblocks);
}

void
BlockFileWriter::startSliceBlock()
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  if (m_slice)
    delete [] m_slice;
//  m_slice = new uchar[m_wblocks*m_hblocks*bpb];
//  memset(m_slice, 0, m_wblocks*m_hblocks*bpb);

  m_slice = new uchar[m_blockSize*m_width*m_height*m_bytesPerVoxel];
  memset(m_slice, 0, m_blockSize*m_width*m_height*m_bytesPerVoxel);

  if (m_sliceAcc)
    delete [] m_sliceAcc;

  m_sliceAcc = new uchar[m_blockSize*m_width*m_height*m_bytesPerVoxel];
  memset(m_sliceAcc, 0, m_blockSize*m_width*m_height*m_bytesPerVoxel);

  if (m_localHistogram)
    delete [] m_localHistogram;
  m_localHistogram = new ushort[m_wblocks*m_hblocks*256];
}

void
BlockFileWriter::endSliceBlock()
{
  if (m_slice)
    delete [] m_slice;
  m_slice = 0;

  if (m_sliceAcc)
    delete [] m_sliceAcc;
  m_sliceAcc = 0;

  if (m_localHistogram)
    delete [] m_localHistogram;
  m_localHistogram = 0;
}

void
BlockFileWriter::startAddSlice()
{
  startSliceBlock();

  m_dumpSlice = false;

  QString filename = m_baseFilename + ".h5";
  m_hdf5file = new H5File(filename.toAscii().data(),
			  H5F_ACC_TRUNC);

  if (m_voxelType == _UChar) m_dataType.copy(DataType(PredType::NATIVE_UCHAR));
  else if (m_voxelType == _Char) m_dataType.copy(DataType(PredType::NATIVE_CHAR));
  else if (m_voxelType == _UShort) m_dataType.copy(DataType(PredType::NATIVE_USHORT));
  else if (m_voxelType == _Short) m_dataType.copy(DataType(PredType::NATIVE_SHORT));
  else if (m_voxelType == _Int) m_dataType.copy(DataType(PredType::NATIVE_INT));
  else if (m_voxelType == _Float) m_dataType.copy(DataType(PredType::NATIVE_FLOAT));
  else if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    {
      size_t nRgb = 3;
      if (m_voxelType == _Rgba) nRgb = 4;

      typedef struct rgbstruct
      {
	uchar r, g, b;
      } rgb;
  
      //CompType rgbType(sizeof(rgbstruct));
      CompType rgbType(nRgb);
      rgbType.insertMember("r", 0, PredType::NATIVE_UCHAR);
      rgbType.insertMember("g", 1, PredType::NATIVE_UCHAR);
      rgbType.insertMember("b", 2, PredType::NATIVE_UCHAR);
      if (nRgb == 4)
	rgbType.insertMember("a", 3, PredType::NATIVE_UCHAR);
      m_dataType.copy(DataType(rgbType));
    }

//  if (m_voxelType != _Rgb && m_voxelType != _Rgba)
//    m_dataType.setOrder( H5T_ORDER_LE );

  hsize_t dimsf[3];              // dataset dimensions

  for(int ib=0; ib<=m_minLevel; ib++)
    {
      hsize_t bdim[3];              // block dimensions
      bdim[0] = m_blockSize/qPow(2, ib);
      bdim[1] = bdim[0];
      bdim[2] = bdim[0];
      
      //if (ib == 0)
	{
	  int lod2 = qPow(2, ib);
	  dimsf[0] = m_depth/lod2 + (m_depth%lod2 > 0);
	  dimsf[1] = m_width/lod2 + (m_width%lod2 > 0);
	  dimsf[2] = m_height/lod2+ (m_height%lod2 > 0);
	}
//      else
//	{
//	  dimsf[0] = m_dblocks*bdim[0];
//	  dimsf[1] = m_wblocks*bdim[1];
//	  dimsf[2] = m_hblocks*bdim[2];
//	}
      DataSpace dataspace( 3, dimsf );
      
      DSetCreatPropList cparms;
      cparms.setChunk(3, bdim);
      cparms.setDeflate(6);
      
      QString dataname = QString("lod-%1").arg(ib);
      m_hdf5dataset[ib] = m_hdf5file->createDataSet( dataname.toAscii().data(),
						     m_dataType,
						     dataspace,
						     cparms );  
    }

  {
    dimsf[0] = m_dblocks*m_wblocks*m_hblocks;
    dimsf[1] = 256;
    DataSpace dataspace( 2, dimsf );
    //for(int ib=0; ib<=m_minLevel; ib++)
    int ib = 0;
      {
	QString dataname = QString("localhistogram-%1").arg(ib);
	m_localHist[ib] = m_hdf5file->createDataSet(dataname.toAscii().data(),
						    DataType(PredType::NATIVE_USHORT),
						    dataspace);  
      }
  }

  {
    dimsf[0] = 2*m_dblocks*m_wblocks*m_hblocks;
    DataSpace dataspace(1, dimsf);
    m_minmaxdata = m_hdf5file->createDataSet( "minmax",
					      m_dataType,
					      dataspace);
  }

  {
    dimsf[0] = m_minLevel*m_dblocks*m_wblocks*m_hblocks;
    DataSpace dataspace(1, dimsf);
    m_rmsdata = m_hdf5file->createDataSet( "rms",
					   DataType(PredType::NATIVE_FLOAT),
					   dataspace);
  }

  {
    dimsf[0] = m_ssd;
    dimsf[1] = m_ssw;
    dimsf[2] = m_ssh;
    DataSpace dataspace(3, dimsf);

    m_lowres = m_hdf5file->createDataSet( "lowres",
					  m_dataType,
					  dataspace);

    dimsf[0] = 3;
    DataSpace gridspace(1, dimsf);
    Attribute attrib = m_hdf5dataset[0].createAttribute("gridsize",
							PredType::NATIVE_INT,
							gridspace); 
    int griddim[3];
    griddim[0] = m_depth;
    griddim[1] = m_width;
    griddim[2] = m_height;
    attrib.write(PredType::NATIVE_INT, griddim);
  }
}

void
BlockFileWriter::endAddSlice()
{
  int remainder = (m_depth-1)%m_blockSize;
  if (remainder > 0)
    {
      memcpy(m_slice, m_sliceAcc, m_blockSize*m_width*m_height*m_bytesPerVoxel);

      genZeroLevelSliceBlocks(m_depth-1);
      dumpSliceBlocks(0, m_depth-1);
      for(int ib=1; ib<=m_minLevel; ib++)
	{
	  genSliceBlocks(ib, m_depth-1);
	  dumpSliceBlocks(ib, m_depth-1);
	}
    }

  saveDict();
  m_hdf5file->close();

  endSliceBlock();
}

void
BlockFileWriter::saveDict()
{
  hsize_t dimsf[3];              // dataset dimensions
  dimsf[0] = m_ssd;
  dimsf[1] = m_ssw;
  dimsf[2] = m_ssh;
  DataSpace dataspace(3, dimsf);
  DataSpace memspace(3, dimsf);

  m_lowres.write(m_ssvol,
		 m_dataType,
		 memspace,
		 dataspace );
  
  Attribute attrib = m_lowres.createAttribute("sslevel",
					      PredType::NATIVE_UCHAR,
					      DataSpace(H5S_SCALAR)); 
  attrib.write(PredType::NATIVE_UCHAR, &m_sslevel);


  {
    dimsf[0] = 2*m_dblocks*m_wblocks*m_hblocks;
    DataSpace dataspace(1, dimsf);
    DataSpace memspace(1, dimsf);
    
    m_minmaxdata.write(m_minmaxvals,
		       m_dataType,
		       memspace,
		       dataspace );

  }

  {
    dimsf[0] = m_minLevel*m_dblocks*m_wblocks*m_hblocks;
    DataSpace dataspace(1, dimsf);
    DataSpace memspace(1, dimsf);
    
    m_rmsdata.write(m_rmsValues,
		    DataType(PredType::NATIVE_FLOAT),
		    memspace,
		    dataspace );

  }
}

void
BlockFileWriter::setSlice(int d, uchar *tmp)
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  int ldno = d%m_blockSize;
  memcpy(m_sliceAcc + ldno*m_width*m_height*m_bytesPerVoxel,
	 tmp,
	 m_width*m_height*m_bytesPerVoxel);

  if (ldno == m_blockSize-1)
    {
      memcpy(m_slice, m_sliceAcc, m_blockSize*m_width*m_height*m_bytesPerVoxel);

      genZeroLevelSliceBlocks(d);
      dumpSliceBlocks(0, d);
      for(int ib=1; ib<=m_minLevel; ib++)
	{
	  genSliceBlocks(ib, d);
	  dumpSliceBlocks(ib, d);
	}
      
      memset(m_slice, 0, m_blockSize*m_width*m_height*m_bytesPerVoxel);
      memset(m_sliceAcc, 0, m_blockSize*m_width*m_height*m_bytesPerVoxel);
    }

  // calculate subsampled volume
  if (d%m_sslevel == 0)
    {
      int dss = d/m_sslevel;
      int idx0 = dss*m_ssw*m_ssh*m_bytesPerVoxel;
      int idx = 0;
      if (m_bytesPerVoxel == 1)
	{
	  for(int w=0; w<m_ssw; w++)
	    for(int h=0; h<m_ssh; h++)
	      {
		m_ssvol[idx0+idx] = tmp[(w*m_height + h)*m_sslevel];
		idx ++;
	      }
	}
      else if (m_bytesPerVoxel == 2)
	{
	  for(int w=0; w<m_ssw; w++)
	    for(int h=0; h<m_ssh; h++)
	      {
		m_ssvol[idx0+2*idx+0] = tmp[2*(w*m_height + h)*m_sslevel + 0];
		m_ssvol[idx0+2*idx+1] = tmp[2*(w*m_height + h)*m_sslevel + 1];
		idx ++;
	      }
	}
      else if (m_bytesPerVoxel == 3)
	{
	  for(int w=0; w<m_ssw; w++)
	    for(int h=0; h<m_ssh; h++)
	      {
		m_ssvol[idx0+3*idx+0] = tmp[3*(w*m_height + h)*m_sslevel + 0];
		m_ssvol[idx0+3*idx+1] = tmp[3*(w*m_height + h)*m_sslevel + 1];
		m_ssvol[idx0+3*idx+2] = tmp[3*(w*m_height + h)*m_sslevel + 2];
		idx ++;
	      }
	}
      else if (m_bytesPerVoxel == 4)
	{
	  for(int w=0; w<m_ssw; w++)
	    for(int h=0; h<m_ssh; h++)
	      {
		m_ssvol[idx0+4*idx+0] = tmp[4*(w*m_height + h)*m_sslevel + 0];
		m_ssvol[idx0+4*idx+1] = tmp[4*(w*m_height + h)*m_sslevel + 1];
		m_ssvol[idx0+4*idx+2] = tmp[4*(w*m_height + h)*m_sslevel + 2];
		m_ssvol[idx0+4*idx+3] = tmp[4*(w*m_height + h)*m_sslevel + 3];
		idx ++;
	      }
	}
    }
}

void
BlockFileWriter::genZeroLevelSliceBlocks(int d)
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;
  int bps = bb*bb*m_bytesPerVoxel;

  int dno = d/m_blockSize;

  memset(m_localHistogram, 0, m_wblocks*m_hblocks*256*2);

  for (int ldno=0; ldno<m_blockSize; ldno++)
    {
      uchar *tmp = m_slice + ldno*m_width*m_height*m_bytesPerVoxel;
      int lbno = 0;
      for(int w=0; w<m_wblocks; w++)
	for(int h=0; h<m_hblocks; h++)
	  {
	    int lbidx = lbno*bpb + ldno*bps;

	    if (m_bytesPerVoxel == 1)
	      {
		// calculate minmax values for level-0
		uchar minval = 255;
		uchar maxval = 0;
		int idx = 0;
		for(int iw=0; iw<m_blockSize; iw++)
		  for(int ih=0; ih<m_blockSize; ih++)
		    {
		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
		      uchar val = tmp[jw*m_height + jh];

		      minval = qMin(minval, val);
		      maxval = qMax(maxval, val);

		      m_localHistogram[lbno*256 + val]++;

		      idx++;
		    }
		int midx = 2*(dno*m_wblocks*m_hblocks + w*m_hblocks + h);
		m_minmaxvals[midx + 0] = qMin(minval, m_minmaxvals[midx + 0]);
		m_minmaxvals[midx + 1] = qMax(maxval, m_minmaxvals[midx + 1]);
	      }
//	    else if (m_bytesPerVoxel == 2)
//	      {
//		int idx = 0;
//		for(int iw=0; iw<m_blockSize; iw++)
//		  for(int ih=0; ih<m_blockSize; ih++)
//		    {
//		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
//		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
//		      m_slice[lbidx + 2*idx+0] = tmp[2*(jw*m_height + jh) + 0];
//		      m_slice[lbidx + 2*idx+1] = tmp[2*(jw*m_height + jh) + 1];
//		      idx++;
//		    }
//	      }
//	    else if (m_bytesPerVoxel == 3)
//	      {
//		int idx = 0;
//		for(int iw=0; iw<m_blockSize; iw++)
//		  for(int ih=0; ih<m_blockSize; ih++)
//		    {
//		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
//		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
//		      m_slice[lbidx + 3*idx+0] = tmp[3*(jw*m_height + jh) + 0];
//		      m_slice[lbidx + 3*idx+1] = tmp[3*(jw*m_height + jh) + 1];
//		      m_slice[lbidx + 3*idx+2] = tmp[3*(jw*m_height + jh) + 2];
//		      idx++;
//		    }
//	      }
//	    else if (m_bytesPerVoxel == 4)
//	      {
//		int idx = 0;
//		for(int iw=0; iw<m_blockSize; iw++)
//		  for(int ih=0; ih<m_blockSize; ih++)
//		    {
//		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
//		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
//		      m_slice[lbidx + 4*idx+0] = tmp[4*(jw*m_height + jh) + 0];
//		      m_slice[lbidx + 4*idx+1] = tmp[4*(jw*m_height + jh) + 1];
//		      m_slice[lbidx + 4*idx+2] = tmp[4*(jw*m_height + jh) + 2];
//		      m_slice[lbidx + 4*idx+3] = tmp[4*(jw*m_height + jh) + 3];
//		      idx++;
//		    }
//	      }
	    lbno++;
	  }
    }
}

//-------
#define interpolateVoxel(T)						\
  T* slc1 = (T*)tmp;							\
  T* slc0 = (T*)slice0;							\
  int b = 0;								\
  float v = 0.0;							\
  float rms = 0.0;							\
  for(int is=0; is<bb0; is+=2)						\
    for(int iw=0; iw<bb0; iw+=2)					\
      for(int ih=0; ih<bb0; ih+=2)					\
	{								\
	  slc1[b] = (slc0[is*bb0*bb0+iw*bb0+ih] +			\
		     slc0[is*bb0*bb0+iw*bb0+(ih+1)] +			\
		     slc0[is*bb0*bb0+(iw+1)*bb0+ih] +			\
		     slc0[is*bb0*bb0+(iw+1)*bb0+(ih+1)] +		\
		     slc0[(is+1)*bb0*bb0+iw*bb0+ih] +			\
		     slc0[(is+1)*bb0*bb0+iw*bb0+(ih+1)] +		\
		     slc0[(is+1)*bb0*bb0+(iw+1)*bb0+ih] +		\
		     slc0[(is+1)*bb0*bb0+(iw+1)*bb0+(ih+1)])/8;		\
									\
	  v = slc1[b]-slc0[is*bb0*bb0+iw*bb0+ih];			\
	  rms += v*v;							\
	  v = slc1[b]-slc0[is*bb0*bb0+iw*bb0+(ih+1)];			\
	  rms += v*v;							\
	  v = slc1[b]-slc0[is*bb0*bb0+(iw+1)*bb0+ih];			\
	  rms += v*v;							\
	  v = slc1[b]-slc0[is*bb0*bb0+(iw+1)*bb0+(ih+1)];		\
	  rms += v*v;							\
	  v = slc1[b]-slc0[(is+1)*bb0*bb0+iw*bb0+ih];			\
	  rms += v*v;							\
	  v = slc1[b]-slc0[(is+1)*bb0*bb0+iw*bb0+(ih+1)];		\
	  rms += v*v;							\
	  v = slc1[b]-slc0[(is+1)*bb0*bb0+(iw+1)*bb0+ih];		\
	  rms += v*v;							\
	  v = slc1[b]-slc0[(is+1)*bb0*bb0+(iw+1)*bb0+(ih+1)];		\
	  rms += v*v;							\
									\
	  b++;								\
	}								\
  rms /= (bb0*bb0*bb0);						\
  rms = qSqrt(rms);							\
//-------

////-------
//#define interpolateVoxel(T)						\
//  T* slc1 = (T*)tmp;							\
//  T* slc0 = (T*)slice0;							\
//  int b = 0;								\
//  float v = 0.0;							\
//  float rms = 0.0;							\
//  for(int is=0; is<bb0; is+=2)						\
//    for(int iw=0; iw<bb0; iw+=2)					\
//      for(int ih=0; ih<bb0; ih+=2)					\
//	{								\
//	  slc1[b] = slc0[is*bb0*bb0+iw*bb0+ih];				\
//									\
//	  v = slc1[b]-slc0[is*bb0*bb0+iw*bb0+ih];			\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[is*bb0*bb0+iw*bb0+(ih+1)];			\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[is*bb0*bb0+(iw+1)*bb0+ih];			\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[is*bb0*bb0+(iw+1)*bb0+(ih+1)];		\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[(is+1)*bb0*bb0+iw*bb0+ih];			\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[(is+1)*bb0*bb0+iw*bb0+(ih+1)];		\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[(is+1)*bb0*bb0+(iw+1)*bb0+ih];		\
//	  rms += v*v;							\
//	  v = slc1[b]-slc0[(is+1)*bb0*bb0+(iw+1)*bb0+(ih+1)];		\
//	  rms += v*v;							\
//									\
//	  b++;								\
//	}								\
//  rms /= (bb0*bb0*bb0);						\
//  rms = qSqrt(rms);							\
////-------

void
BlockFileWriter::genSliceBlocks(int level, int d)
{
  int b3 = m_blockSize*m_blockSize*m_blockSize;

  int bb0 = m_blockSize/qPow(2, level-1);
  int bb1 = m_blockSize/qPow(2, level);
  int bpb0 = bb0*bb0*bb0*m_bytesPerVoxel;
  int bpb1 = bb1*bb1*bb1*m_bytesPerVoxel;

  int dno = d/m_blockSize;

  int lod0 = qPow(2, level-1);
  int wend0 = m_width/lod0 + (m_width%lod0 > 0);
  int hend0 = m_height/lod0 + (m_height%lod0 > 0);
  int dend0 = m_depth/lod0 + (m_depth%lod0 > 0);

  int lod1 = qPow(2, level);
  int wend1 = m_width/lod1 + (m_width%lod1 > 0);
  int hend1 = m_height/lod1 + (m_height%lod1 > 0);

  float v0, v, rms;
  for(int i=0; i<bb1; i++)
    for(int w=0; w<wend1; w++)
      for(int h=0; h<hend1; h++)
      {
	int dd0 = qMin(2*i,   dend0);
	int dd1 = qMin(2*i+1, dend0);
	int ww0 = qMin(2*w,   wend0);
	int ww1 = qMin(2*w+1, wend0);
	int hh0 = qMin(2*h,   hend0);
	int hh1 = qMin(2*h+1, hend0);

	v0 = (m_slice[dd0*wend0*hend0 + ww0*hend0 + hh0] +
	      m_slice[dd0*wend0*hend0 + ww0*hend0 + hh1] +
	      m_slice[dd0*wend0*hend0 + ww1*hend0 + hh0] +
	      m_slice[dd0*wend0*hend0 + ww1*hend0 + hh1] +
	      m_slice[dd1*wend0*hend0 + ww0*hend0 + hh0] +
	      m_slice[dd1*wend0*hend0 + ww0*hend0 + hh1] +
	      m_slice[dd1*wend0*hend0 + ww1*hend0 + hh0] +
	      m_slice[dd1*wend0*hend0 + ww1*hend0 + hh1])/8;
	
	m_slice[i*wend1*hend1 + w*hend1 + h] = v0;
      }

  for(int w=0; w<m_wblocks; w++)
    for(int h=0; h<m_hblocks; h++)
      {
	int wst0 = w*m_blockSize;
	int hst0 = h*m_blockSize;
	int wst1 = w*bb1;
	int hst1 = h*bb1;
	float rms = 0;

	int de = qMin(m_blockSize, m_depth-dno*m_blockSize);
	int we = qMin(m_blockSize, m_width-w*m_blockSize);
	int he = qMin(m_blockSize, m_height-h*m_blockSize);
	int nb = 0;
	for(int d0=0; d0<de; d0++)
	  for(int w0=0; w0<we; w0++)
	    for(int h0=0; h0<he; h0++)
	      {
		int idx0 = (d0*m_width*m_height +
			    (wst0+w0)*m_height +
			    (hst0+h0));
		int idx1 = (d0/lod1*wend1*hend1 +
			    (wst1+w0/lod1)*hend1 +
			    (hst1+h0/lod1));
		float v = m_sliceAcc[idx0] - m_slice[idx1];
		rms += v*v;
		nb ++;
	      }
	rms /= nb;
	rms = qSqrt(rms);
	int ridx = m_minLevel*(dno*m_wblocks*m_hblocks + w*m_hblocks + h);
	m_rmsValues[ridx + (level-1)] = rms;
      }

//  int lbno = 0;
//  for(int w=0; w<m_wblocks; w++)
//    for(int h=0; h<m_hblocks; h++)
//      {
//	int idx0 = lbno*bpb0;
//	int idx1 = lbno*bpb1;
//	uchar *slice1 = m_slice + idx1;
//	uchar *slice0 = m_slice + idx0;
//
//	if (m_voxelType == _UChar)
//	  {
//	    interpolateVoxel(uchar)
//
//	    int ridx = m_minLevel*(dno*m_wblocks*m_hblocks + w*m_hblocks + h);
//	    m_rmsValues[ridx + (level-1)] = rms;
//
////	    for(b=0; b<bpb1; b++)
////	      m_localHistogram[lbno*256 + tmp[b]]++;
//	  }
//	else if (m_voxelType == _Char)
//	  {
//	    interpolateVoxel(char)
//	  }
//	else if (m_voxelType == _UShort)
//	  {
//	    interpolateVoxel(ushort)
//	  }
//	else if (m_voxelType == _Short)
//	  {
//	    interpolateVoxel(short)
//	  }
//	else if (m_voxelType == _Int)
//	  {
//	    interpolateVoxel(int)
//	  }
//	else if (m_voxelType == _Float)
//	  {
//	    interpolateVoxel(float)
//	  }
//	else if (m_voxelType == _Rgb || m_voxelType == _Rgba)
//	  {
//	    int nv = 3;
//	    if (m_voxelType == _Rgba) nv = 4;
//	    int b = 0;
//	    for(int is=0; is<bb0; is+=2)
//	      for(int iw=0; iw<bb0; iw+=2)
//		for(int ih=0; ih<bb0; ih+=2)
//		  {
//		    for(int vi=0; vi<nv; vi++)
//		      tmp[nv*b+vi] = (*(slice0 + nv*(is*bb0*bb0+iw*bb0+ih)        +vi) +
//				      *(slice0 + nv*(is*bb0*bb0+iw*bb0+(ih+1))    +vi) +
//				      *(slice0 + nv*(is*bb0*bb0+(iw+1)*bb0+ih)    +vi) +
//				      *(slice0 + nv*(is*bb0*bb0+(iw+1)*bb0+(ih+1))+vi) +
//				      *(slice0 + nv*((is+1)*bb0*bb0+iw*bb0+ih)    +vi) +
//				      *(slice0 + nv*((is+1)*bb0*bb0+iw*bb0+(ih+1))+vi) +
//				      *(slice0 + nv*((is+1)*bb0*bb0+(iw+1)*bb0+ih)+vi) +
//				      *(slice0 + nv*((is+1)*bb0*bb0+(iw+1)*bb0+(ih+1))+vi))/8;
//		    b++;
//		  }
//	  }
//
//	memcpy(slice1, tmp, bpb1);	
//	lbno ++;
//      }
//  delete [] tmp;
    
}

void
BlockFileWriter::dumpSliceBlocks(int ib, int d)
{
  int lod2 = qPow(2, ib);

  int bb = m_blockSize/lod2;
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  int dno = d/m_blockSize;
  int blkno = dno*m_wblocks*m_hblocks;	  


  hsize_t offset[3];              // block dimensions
  offset[0] = (d/m_blockSize)*bb;
  offset[1] = 0;
  offset[2] = 0;

  hsize_t bdim[3];              // block dimensions
  int totslices = (m_depth/lod2 + (m_depth%lod2 > 0));
  int nslices = totslices - offset[0];
  bdim[0] = qMin(bb, nslices);
  bdim[1] = m_width/lod2 + (m_width%lod2 > 0);
  bdim[2] = m_height/lod2+ (m_height%lod2 > 0);
      
  hsize_t dimsf[3];              // dataset dimensions
  dimsf[0] = m_depth/lod2 + (m_depth%lod2 > 0);
  dimsf[1] = m_width/lod2 + (m_width%lod2 > 0);
  dimsf[2] = m_height/lod2+ (m_height%lod2 > 0);

//  //if (ib == 0)
//  QMessageBox::information(0, "", QString("%1 (%2) : %3 : %4").		\
//			   arg(ib).arg(d).arg(offset[0]).arg(bdim[0]));

  DataSpace memspace(3, bdim);
  DataSpace dspace(3, dimsf);
      
  dspace.selectHyperslab(H5S_SELECT_SET, bdim, offset);
      
  m_hdf5dataset[ib].write(m_slice,
			  m_dataType,
			  memspace, dspace );

  if (ib == 0)
    {
      hsize_t lmdim[2];
      lmdim[0] = m_wblocks*m_hblocks;
      lmdim[1] = 256;
      DataSpace lmemspace(2, lmdim);

      hsize_t loffset[2];
      loffset[0] = blkno;
      loffset[1] = 0;
      hsize_t lddim[2];
      lddim[0] = m_dblocks*m_wblocks*m_hblocks;
      lddim[1] = 256;
      DataSpace ldspace(2, lddim);
      ldspace.selectHyperslab(H5S_SELECT_SET, lmdim, loffset);
      
      m_localHist[ib].write(m_localHistogram,
			    DataType(PredType::NATIVE_USHORT),
			    lmemspace,
			    ldspace );
    }
}

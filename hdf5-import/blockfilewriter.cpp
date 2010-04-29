#include <QtGui>

#include "common.h"
#include "blockfilewriter.h"

BlockFileWriter::BlockFileWriter()
{
  m_dumpSlice = false;
  m_slice = 0;
  m_sliceAcc = 0;

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


  int maxvsize = 128*1024*1024;
  if (m_voxelType == _Rgb || m_voxelType == _Rgba)
    maxvsize = 64*1024*1024;

  m_sslevel = 1;
  while (vsize > maxvsize)
    {
      m_sslevel *= 2;
      vsize /= 8;
    }

  m_minLevel = 0;
  if (m_sslevel > 1)
    m_minLevel = logf(m_sslevel)/logf(2.0)-1;

  m_ssd = m_depth/m_sslevel + (m_depth%m_sslevel > 0);
  m_ssw = m_width/m_sslevel + (m_width%m_sslevel > 0);
  m_ssh = m_height/m_sslevel+ (m_height%m_sslevel > 0);
  m_ssvol = new uchar[m_ssd*m_ssw*m_ssh*m_bytesPerVoxel];
  memset(m_ssvol, 0, m_ssd*m_ssw*m_ssh*m_bytesPerVoxel);
}

void
BlockFileWriter::startSliceBlock()
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  if (m_slice)
    delete [] m_slice;

  m_slice = new uchar[m_wblocks*m_hblocks*bpb];
  memset(m_slice, 0, m_wblocks*m_hblocks*bpb);

  if (m_sliceAcc)
    delete [] m_sliceAcc;

  m_sliceAcc = new uchar[m_blockSize*m_width*m_height*m_bytesPerVoxel];
  memset(m_sliceAcc, 0, m_blockSize*m_width*m_height*m_bytesPerVoxel);
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

  for(int ib=0; ib<=m_minLevel; ib++)
    {
      hsize_t bdim[3];              // block dimensions
      bdim[0] = m_blockSize/qPow(2, ib);
      bdim[1] = bdim[0];
      bdim[2] = bdim[0];
      
      hsize_t dimsf[3];              // dataset dimensions
      dimsf[0] = m_dblocks*bdim[0];
      dimsf[1] = m_wblocks*bdim[1];
      dimsf[2] = m_hblocks*bdim[2];
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


  hsize_t dimsf[3];              // dataset dimensions
  dimsf[0] = m_ssd;
  dimsf[1] = m_ssw;
  dimsf[2] = m_ssh;
  DataSpace dataspace(3, dimsf);

  m_lowres = m_hdf5file->createDataSet( "lowres",
					m_dataType,
					dataspace);
}

void
BlockFileWriter::endAddSlice()
{
  genSliceBlocks();
  dumpSliceBlocks(0, m_depth-1);
  for(int ib=1; ib<=m_minLevel; ib++)
    {
      genSliceBlocks(ib);
      dumpSliceBlocks(ib, m_depth-1);
    }

  m_hdf5file->close();

  endSliceBlock();

  saveDict();
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
      genSliceBlocks();
      dumpSliceBlocks(0, d);
      for(int ib=1; ib<=m_minLevel; ib++)
	{
	  genSliceBlocks(ib);
	  dumpSliceBlocks(ib, d);
	}
      
      memset(m_slice, 0, m_wblocks*m_hblocks*bpb);
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
BlockFileWriter::genSliceBlocks()
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;
  int bps = bb*bb*m_bytesPerVoxel;

  for (int ldno=0; ldno<m_blockSize; ldno++)
    {
      uchar *tmp = m_sliceAcc + ldno*m_width*m_height*m_bytesPerVoxel;
      int lbno = 0;
      for(int w=0; w<m_wblocks; w++)
	for(int h=0; h<m_hblocks; h++)
	  {
	    int lbidx = lbno*bpb + ldno*bps;
	    lbno++;

	    if (m_bytesPerVoxel == 1)
	      {
		int idx = 0;
		for(int iw=0; iw<m_blockSize; iw++)
		  for(int ih=0; ih<m_blockSize; ih++)
		    {
		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
		      m_slice[lbidx + idx] = tmp[jw*m_height + jh];
		      idx++;
		    }
	      }
	    else if (m_bytesPerVoxel == 2)
	      {
		int idx = 0;
		for(int iw=0; iw<m_blockSize; iw++)
		  for(int ih=0; ih<m_blockSize; ih++)
		    {
		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
		      m_slice[lbidx + 2*idx+0] = tmp[2*(jw*m_height + jh) + 0];
		      m_slice[lbidx + 2*idx+1] = tmp[2*(jw*m_height + jh) + 1];
		      idx++;
		    }
	      }
	    else if (m_bytesPerVoxel == 3)
	      {
		int idx = 0;
		for(int iw=0; iw<m_blockSize; iw++)
		  for(int ih=0; ih<m_blockSize; ih++)
		    {
		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
		      m_slice[lbidx + 3*idx+0] = tmp[3*(jw*m_height + jh) + 0];
		      m_slice[lbidx + 3*idx+1] = tmp[3*(jw*m_height + jh) + 1];
		      m_slice[lbidx + 3*idx+2] = tmp[3*(jw*m_height + jh) + 2];
		      idx++;
		    }
	      }
	    else if (m_bytesPerVoxel == 4)
	      {
		int idx = 0;
		for(int iw=0; iw<m_blockSize; iw++)
		  for(int ih=0; ih<m_blockSize; ih++)
		    {
		      int jw = qMin(qMax(0, w*m_blockSize + iw), m_width-1);
		      int jh = qMin(qMax(0, h*m_blockSize + ih), m_height-1);
		      m_slice[lbidx + 4*idx+0] = tmp[4*(jw*m_height + jh) + 0];
		      m_slice[lbidx + 4*idx+1] = tmp[4*(jw*m_height + jh) + 1];
		      m_slice[lbidx + 4*idx+2] = tmp[4*(jw*m_height + jh) + 2];
		      m_slice[lbidx + 4*idx+3] = tmp[4*(jw*m_height + jh) + 3];
		      idx++;
		    }
	      }
	  }
    }
}

void
BlockFileWriter::genSliceBlocks(int level)
{
  int bb0 = m_blockSize/qPow(2, level-1);
  int bb1 = m_blockSize/qPow(2, level);
  int bpb0 = bb0*bb0*bb0*m_bytesPerVoxel;
  int bpb1 = bb1*bb1*bb1*m_bytesPerVoxel;

  int lbno = 0;
  for(int w=0; w<m_wblocks; w++)
    for(int h=0; h<m_hblocks; h++)
      {
	int idx0 = lbno*bpb0;
	int idx1 = lbno*bpb1;
	uchar *slice1 = m_slice + idx1;
	uchar *slice0 = m_slice + idx0;
	if (m_bytesPerVoxel == 1)
	  {
	    int b = 0;
	    for(int is=0; is<bb0; is+=2)
	      for(int iw=0; iw<bb0; iw+=2)
		for(int ih=0; ih<bb0; ih+=2)
		  {
		    *(slice1 + b) = *(slice0 + is*bb0*bb0+iw*bb0+ih);
		    b++;
		  }
	  }
	else if (m_bytesPerVoxel == 2)
	  {
	    int b = 0;
	    for(int is=0; is<bb0; is+=2)
	      for(int iw=0; iw<bb0; iw+=2)
		for(int ih=0; ih<bb0; ih+=2)
		  {
		    *(slice1 + 2*b+0) = *(slice0 + 2*(is*bb0*bb0+iw*bb0+ih) + 0);
		    *(slice1 + 2*b+1) = *(slice0 + 2*(is*bb0*bb0+iw*bb0+ih) + 1);
		    b++;
		  }
	  }
	else if (m_bytesPerVoxel == 3)
	  {
	    int b = 0;
	    for(int is=0; is<bb0; is+=2)
	      for(int iw=0; iw<bb0; iw+=2)
		for(int ih=0; ih<bb0; ih+=2)
		  {
		    *(slice1 + 3*b+0) = *(slice0 + 3*(is*bb0*bb0+iw*bb0+ih) + 0);
		    *(slice1 + 3*b+1) = *(slice0 + 3*(is*bb0*bb0+iw*bb0+ih) + 1);
		    *(slice1 + 3*b+2) = *(slice0 + 3*(is*bb0*bb0+iw*bb0+ih) + 2);
		    b++;
		  }
	  }
	else if (m_bytesPerVoxel == 4)
	  {
	    int b = 0;
	    for(int is=0; is<bb0; is+=2)
	      for(int iw=0; iw<bb0; iw+=2)
		for(int ih=0; ih<bb0; ih+=2)
		  {
		    *(slice1 + 4*b+0) = *(slice0 + 4*(is*bb0*bb0+iw*bb0+ih) + 0);
		    *(slice1 + 4*b+1) = *(slice0 + 4*(is*bb0*bb0+iw*bb0+ih) + 1);
		    *(slice1 + 4*b+2) = *(slice0 + 4*(is*bb0*bb0+iw*bb0+ih) + 2);
		    *(slice1 + 4*b+3) = *(slice0 + 4*(is*bb0*bb0+iw*bb0+ih) + 3);
		    b++;
		  }
	  }

	lbno ++;
      }
}

void
BlockFileWriter::dumpSliceBlocks(int ib, int d)
{
  int bb = m_blockSize/qPow(2, ib);
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  int dno = d/m_blockSize;
  int blkno = dno*m_wblocks*m_hblocks;	  

  int lbno = 0;
  for(int w=0; w<m_wblocks; w++)
    for(int h=0; h<m_hblocks; h++)
      {	
	hsize_t bdim[3];              // block dimensions
	bdim[0] = m_blockSize/qPow(2, ib);
	bdim[1] = bdim[0];
	bdim[2] = bdim[0];

	hsize_t offset[3];              // block dimensions
	offset[0] = dno*bb;
	offset[1] = w*bb;
	offset[2] = h*bb;

	hsize_t dimsf[3];              // dataset dimensions
	dimsf[0] = m_dblocks*bdim[0];
	dimsf[1] = m_wblocks*bdim[1];
	dimsf[2] = m_hblocks*bdim[2];

	DataSpace memspace(3, bdim);
	DataSpace dspace(3, dimsf);

	dspace.selectHyperslab(H5S_SELECT_SET, bdim, offset);

	m_hdf5dataset[ib].write(m_slice + lbno*bpb,
				m_dataType,
				memspace, dspace );

	lbno ++;
      }
}

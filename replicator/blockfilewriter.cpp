#include "blockfilewriter.h"
#include <QtGui>

BlockFileWriter::BlockFileWriter()
{
  m_dumpSlice = false;
  m_slice = 0;
  m_sliceAcc = 0;

  m_maxFileSize = 1024*1024*1024;
  m_blockSize = 32;

  m_minLevel = 2;

  reset();
}

BlockFileWriter::~BlockFileWriter() { reset(); }

void
BlockFileWriter::reset()
{
  for(int ib=0; ib<=m_minLevel; ib++)
    {
      m_prevfno[ib] = -1;
      if (m_qfile[ib].isOpen())
	m_qfile[ib].close();
      m_prevfno[ib] = -1;
      m_filename[ib].clear();
      m_uniform[ib].clear();
      m_fileBlocks[ib].clear();
      for(int i=0; i<m_blockOffset[ib].count(); i++)
	(m_blockOffset[ib])[i].clear();
      m_blockOffset[ib].clear();
      m_tmpOffset[ib].clear();
    }

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
  if (m_voxelType == _Char) m_bytesPerVoxel = 1;
  if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
  if (m_voxelType == _Short) m_bytesPerVoxel = 2;
  if (m_voxelType == _Int) m_bytesPerVoxel = 4;
  if (m_voxelType == _Float) m_bytesPerVoxel = 4;
}

QString BlockFileWriter::fileName() { return m_filename[0]; }

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

  m_sslevel = 1;
  m_minLevel = 0;
  while (vsize > 128*1024*1024)
    {
      m_minLevel++;
      m_sslevel *= 2;
      vsize /= 8;
    }

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

  for(int ib=0; ib<=m_minLevel; ib++)
    {
      m_uniform[ib].resize(m_totBlocks);
      m_uniform[ib].fill(false);

      m_fileBlocks[ib].clear();
      for(int i=0; i<m_blockOffset[ib].count(); i++)
	(m_blockOffset[ib])[i].clear();
      m_blockOffset[ib].clear();
      m_tmpOffset[ib].clear();
      m_prevfno[ib] = 0;
      m_filename[ib] = m_baseFilename +
	               QString(".%1").arg(m_blockSize/qPow(2, ib)) + 
	               QString(".%1").arg(m_prevfno[ib]+1, 3, 10, QChar('0'));
      m_qfile[ib].setFileName(m_filename[ib]);
      m_qfile[ib].open(QFile::ReadWrite);
      m_qfile[ib].seek(m_header);
    }
}

void
BlockFileWriter::endAddSlice()
{
  genSliceBlocks();
  dumpSliceBlocks(0, m_depth-1);
  for(int ib=1; ib<m_minLevel; ib++)
    {
      genSliceBlocks(ib);
      dumpSliceBlocks(ib, m_depth-1);
    }

  endSliceBlock();

  for(int ib=0; ib<=m_minLevel; ib++)
    {
      m_blockOffset[ib].append(m_tmpOffset[ib]);
      m_qfile[ib].close();
      m_fileBlocks[ib].clear();
      m_fileBlocks[ib].append((m_blockOffset[ib])[0].count());
      for(int fb=1; fb<m_blockOffset[ib].count(); fb++)
	m_fileBlocks[ib].append(m_fileBlocks[ib].at(fb-1) +
				(m_blockOffset[ib])[fb].count());
    }

  saveDict();
}

void
BlockFileWriter::saveDict()
{
  QString dictFile = m_baseFilename + ".dict";
  QFile dfile(dictFile);
  dfile.open(QIODevice::WriteOnly);
  QDataStream out(&dfile);
  out << m_minLevel;
  for(int ib=0; ib<=m_minLevel; ib++)
    {
      out << m_uniform[ib];
      out << m_fileBlocks[ib];
      out << m_blockOffset[ib];
    }
  dfile.close();

  QString lowresFile = m_baseFilename + ".lowres";
  QFile lfile(lowresFile);
  lfile.open(QIODevice::WriteOnly);
  lfile.write((char*)&m_sslevel, 1);
  lfile.write((char*)&m_ssd, 4);
  lfile.write((char*)&m_ssw, 4);
  lfile.write((char*)&m_ssh, 4);
  lfile.write((char*)m_ssvol, m_ssd*m_ssw*m_ssh*m_bytesPerVoxel);
  lfile.close();
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
      for(int ib=1; ib<m_minLevel; ib++)
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
  int bpb0 = bb0*bb0*bb0;
  int bpb1 = bb1*bb1*bb1;

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
	bool uniform = true;
	uchar v0 = m_slice[lbno*bpb];
	uchar *ptr = m_slice + lbno*bpb;
	for(int v=1; v<bpb; v++)
	  {
	    if (*(ptr + v) != v0)
	      {
		uniform = false;
		break;
	      }
	  }

	m_uniform[ib].setBit(blkno, uniform);

	int nblockbytes = bpb;
	if (m_uniform[ib].at(blkno))
	  nblockbytes = m_bytesPerVoxel;
	
	int fno = m_prevfno[ib];
	if(m_tmpOffset[ib].count() > 0)
	  {
	    if (m_tmpOffset[ib].last()+nblockbytes > m_maxFileSize)
	      fno = m_prevfno[ib] + 1;
	  }

	if (m_prevfno[ib] != fno)
	  {
	    m_blockOffset[ib].append(m_tmpOffset[ib]);
	    m_tmpOffset[ib].clear();

	    m_qfile[ib].close();
		  
	    m_prevfno[ib] = fno;
	    m_filename[ib] = m_baseFilename +
	                     QString(".%1").arg(m_blockSize/qPow(2, ib)) + 
	                     QString(".%1").arg(m_prevfno[ib]+1, 3, 10, QChar('0'));
	    m_qfile[ib].setFileName(m_filename[ib]);
	    m_qfile[ib].open(QFile::ReadWrite);
	    m_qfile[ib].seek(m_header);
	  }

	if(m_tmpOffset[ib].count() == 0)
	  m_tmpOffset[ib].append(nblockbytes);
	else
	  m_tmpOffset[ib].append(m_tmpOffset[ib].last() + nblockbytes);
	
	m_qfile[ib].write((char*)(m_slice + lbno*bpb), nblockbytes);

	lbno++;
	blkno++;
      }
}

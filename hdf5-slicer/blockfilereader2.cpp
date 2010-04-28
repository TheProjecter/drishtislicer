#include "blockfilereader.h"
#include <QtGui>

#define LOWRESWIDTHSLICE(T)		  \
{					  \
  T* block = (T*)m_ssvol;		  \
  T* slice = (T*)m_lowresSlice;		  \
  int iss = 0;				  \
  for(int i=0; i<m_ssd; i++)		  \
    for(int j=0; j<m_ssh; j++)		  \
      {								 \
	slice[iss] = (1.0-frc)*block[i*m_ssw*m_ssh + idx0 + j] + \
	                   frc*block[i*m_ssw*m_ssh + idx1 + j];  \
	iss++;							 \
      }								 \
}


uchar*
BlockFileReader::getLowresWidthSlice(int w, int& ssd, int& ssh)
{
  memset(m_lowresSlice, 0, m_ssd*m_ssh*m_bytesPerVoxel);
 
  ssd = m_ssd;
  ssh = m_ssh;

  int ws = w/m_sslevel;

  if (ws >= m_ssw-1)
    return m_lowresSlice;

  float frc = (w-ws*m_sslevel)*1.0/m_sslevel;
  int idx0 = ws*m_ssh;
  int idx1 = (ws+1)*m_ssh;
  if (m_sslevel == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }

    if (m_voxelType == _UChar)
      LOWRESWIDTHSLICE(uchar)
    else if (m_voxelType == _Char)
      LOWRESWIDTHSLICE(char)		    
    else if (m_voxelType == _UShort)
      LOWRESWIDTHSLICE(ushort)		    
    else if (m_voxelType == _Short)
      LOWRESWIDTHSLICE(short)		    
    else if (m_voxelType == _Int)
      LOWRESWIDTHSLICE(int)		    
    else if (m_voxelType == _Float)
      LOWRESWIDTHSLICE(float)		    
    else if (m_voxelType == _Rgb || m_voxelType == _Rgba)
      {
	int nRgb = 3;
	if (m_voxelType == _Rgba)
	  nRgb = 4;
	int iss = 0;
	for(int i=0; i<m_ssd; i++)
	  for(int j=0; j<m_ssh; j++)
	    {
	      for(int a=0; a<nRgb; a++)
		m_lowresSlice[nRgb*iss+a] = (1.0-frc)*m_ssvol[nRgb*(i*m_ssw*m_ssh + idx0 + j)+a] +
		                                  frc*m_ssvol[nRgb*(i*m_ssw*m_ssh + idx1 + j)+a];
	      iss++;
	    }
      }

  return m_lowresSlice;
}

bool
BlockFileReader::widthBlocksPresent(int level, int w,
				    int dstart, int dend,
				    int hstart, int hend)
{
  if (level < 0)
    return true;

  m_startdblocks = dstart/m_blockSize;
  m_enddblocks = qMin(m_dblocks, dend/m_blockSize + 1);

  m_starthblocks = hstart/m_blockSize;
  m_endhblocks = qMin(m_hblocks, hend/m_blockSize + 1);

  m_minhblocks = m_starthblocks;
  m_maxhblocks = qMin(m_minhblocks + 700, m_endhblocks);

  int ndblocks = qMax(700/(m_maxhblocks-m_minhblocks+1), 1);
  m_mindblocks = m_startdblocks;
  m_maxdblocks = qMin(m_mindblocks + ndblocks, m_enddblocks);

  int wno = w/m_blockSize;
  int wno1 = wno;
  int p2 = qPow(2, m_level);
  int localSlice = (w%m_blockSize)/p2;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      wno1 = wno+1;
      nextslab = true;
    }

  for(int d=m_mindblocks; d<m_maxdblocks; d++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (d*m_wblocks*m_hblocks +
		     wno*m_hblocks + h);

	if (! m_blockCache[m_level].contains(blkno))
	  return false;

	if (nextslab)
	  {
	    int blkno1 = (d*m_wblocks*m_hblocks +
			 wno1*m_hblocks + h);

	    if (! m_blockCache[m_level].contains(blkno1))
	      return false;
	  }
      }
  return true;
}

void
BlockFileReader::getWidthSlice(int level, int w,
				int dstart, int dend,
				int hstart, int hend)
{
  if (level < 0)
    return;

  m_level = level;
  m_currentAxis = 1;
  m_currentSlice = w;

  memset(m_widthSlice, 0, m_depth*m_height*m_bytesPerVoxel);

  m_startdblocks = dstart/m_blockSize;
  m_enddblocks = qMin(m_dblocks, dend/m_blockSize + 1);

  m_starthblocks = hstart/m_blockSize;
  m_endhblocks = qMin(m_hblocks, hend/m_blockSize + 1);

  m_minhblocks = m_starthblocks;
  m_maxhblocks = qMin(m_minhblocks + 700, m_endhblocks);

  int ndblocks = qMax(700/(m_maxhblocks-m_minhblocks+1), 1);
  m_mindblocks = m_startdblocks;
  m_maxdblocks = qMin(m_mindblocks + ndblocks, m_enddblocks);

  calculateBlocksForWidthSlice(w);
}

void
BlockFileReader::calculateBlocksForWidthSlice(int w)
{
  QVector<int> blockNumbers;

  int wno = w/m_blockSize;

  int wno1 = wno;
  int p2 = qPow(2, m_level);
  int localSlice = (w%m_blockSize)/p2;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      wno1 = wno+1;
      nextslab = true;
    }

  for(int d=m_mindblocks; d<m_maxdblocks; d++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (d*m_wblocks*m_hblocks +
		     wno*m_hblocks + h);

	blockNumbers.append(blkno);

	if (nextslab)
	  {
	    int blkno1 = (d*m_wblocks*m_hblocks +
			 wno1*m_hblocks + h);

	    blockNumbers.append(blkno1);
	  }

      }

  m_blockReader.loadBlocks(m_level, blockNumbers);
}



#define WIDTHSLICE_1(T)			\
{					\
  T* block = (T*)blockData;		\
  T* slice = (T*)m_widthSlice;		\
  for(int id=0; id<idend; id++)		\
    for(int ih=0; ih<ihend; ih++)       \
      slice[(d*bb +id)*newht + h*bb + ih] =     \
	(1.0-frc)*block[id*bb*bb + idx0 + ih] + \
	      frc*block[id*bb*bb + idx1 + ih];  \
}

#define WIDTHSLICE_2(T)			\
{					\
  T* block = (T*)blockData;		\
  T* block1 = (T*)block1Data;		\
  T* slice = (T*)m_widthSlice;		\
  for(int id=0; id<idend; id++)		\
    for(int ih=0; ih<ihend; ih++)       \
      slice[(d*bb +id)*newht + h*bb + ih] =     \
	(1.0-frc)*block[id*bb*bb + idx0 + ih] + \
	     frc*block1[id*bb*bb + idx1 + ih];  \
}

void
BlockFileReader::widthSliceDone()
{
  int w = m_currentSlice;

  int p2 = qPow(2, m_level);
  int bb = m_blockSize/p2;
  int newht = m_height/p2;
  int newdp = m_depth/p2;

  int wno = w/m_blockSize;
  int localSlice = (w%m_blockSize)/p2;

  //----------------------
  // for interpolation between slices for m_level > 0
  int ws = w%m_blockSize;
  float frc = (ws-localSlice*p2)*1.0/p2;
  int idx0 = localSlice*bb;
  int idx1 = (localSlice+1)*bb;
  if (p2 == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }
  int wno1 = wno;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      idx1 = 0;
      wno1 = wno+1;
      nextslab = true;
    }
  //----------------------

  bool doagain = false;
  for(int d=m_mindblocks; d<m_maxdblocks; d++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (d*m_wblocks*m_hblocks +
		     wno*m_hblocks + h);
	int blkno1 = (d*m_wblocks*m_hblocks +
		      wno1*m_hblocks + h);

	if (m_blockCache[m_level].contains(blkno) &&
	    m_blockCache[m_level].contains(blkno1))
	  {
	    uchar *blockData = (m_blockCache[m_level])[blkno];	
	    int idend = qMin(bb, newdp-d*bb);
	    int ihend = qMin(bb, newht-h*bb);

	    if (p2 == 1)
	      {
		for(int id=0; id<idend; id++)
		  memcpy(m_widthSlice + ((d*bb +id)*newht + h*bb)*m_bytesPerVoxel,
			 blockData + (id*bb*bb + localSlice*bb)*m_bytesPerVoxel,
			 ihend*m_bytesPerVoxel);
	      }
	    else
	      {
		if (!nextslab)
		  {
		    if (m_voxelType == _UChar)
		      WIDTHSLICE_1(uchar)
		    else if (m_voxelType == _Char)
		      WIDTHSLICE_1(char)		    
		    else if (m_voxelType == _UShort)
		      WIDTHSLICE_1(ushort)		    
		    else if (m_voxelType == _Short)
		      WIDTHSLICE_1(short)		    
		    else if (m_voxelType == _Int)
		      WIDTHSLICE_1(int)		    
		    else if (m_voxelType == _Float)
		      WIDTHSLICE_1(float)		    
		    else if (m_voxelType == _Rgb || m_voxelType == _Rgba)
		      {
			int nRgb = 3;
			if (m_voxelType == _Rgba)
			  nRgb = 4;
			for(int id=0; id<idend; id++)
			  for(int ih=0; ih<ihend; ih++)
			    for(int a=0; a<nRgb; a++)
			      m_widthSlice[nRgb*((d*bb +id)*newht + h*bb + ih)+a] =
				               (1.0-frc)*blockData[nRgb*(id*bb*bb + idx0 + ih)+a] +
				                     frc*blockData[nRgb*(id*bb*bb + idx1 + ih)+a];
		      }
		  }
		else
		  {
		    uchar *block1Data = (m_blockCache[m_level])[blkno1];

		    if (m_voxelType == _UChar)
		      WIDTHSLICE_2(uchar)
		    else if (m_voxelType == _Char)
		      WIDTHSLICE_2(char) 
		    else if (m_voxelType == _UShort)
		      WIDTHSLICE_2(ushort)		    
		    else if (m_voxelType == _Short)
		      WIDTHSLICE_2(short)		    
		    else if (m_voxelType == _Int)
		      WIDTHSLICE_2(int)		    
		    else if (m_voxelType == _Float)
		      WIDTHSLICE_2(float)		    
		    else if (m_voxelType == _Rgb || m_voxelType == _Rgba)
		      {
			int nRgb = 3;
			if (m_voxelType == _Rgba)
			  nRgb = 4;
			for(int id=0; id<idend; id++)
			  for(int ih=0; ih<ihend; ih++)
			    for(int a=0; a<nRgb; a++)
			      m_widthSlice[nRgb*((d*bb +id)*newht + h*bb + ih)+a] =
				               (1.0-frc)*blockData[nRgb*(id*bb*bb + idx0 + ih)+a] +
				                     frc*block1Data[nRgb*(id*bb*bb + idx1 + ih)+a];
		      }

		  }
	      }
	  }
	else
	  {
	    doagain = true;
	    break;
	  }
      }

  if (doagain)
    {
      calculateBlocksForWidthSlice(w);
      return;
    }

  int mind = m_startdblocks*bb;
  int maxd = qMin(newdp-1, m_maxdblocks*bb-1);

  int minh = m_starthblocks*bb;
  int maxh = qMin(newht-1, m_maxhblocks*bb-1);

  QPair<int, int> hlimits = qMakePair(minh, maxh);
  QPair<int, int> dlimits = qMakePair(mind, maxd);

  if (m_maxdblocks < m_enddblocks)
    {
      emit widthSlice(w, m_level, true, dlimits, hlimits);

      m_minhblocks = m_maxhblocks;
      m_maxhblocks = qMin(m_minhblocks + 700, m_endhblocks);

      if (m_minhblocks >= m_endhblocks)
	{
	  m_minhblocks = m_starthblocks;
	  m_maxhblocks = qMin(m_minhblocks + 700, m_endhblocks);

	  int ndblocks = qMax(700/(m_maxhblocks-m_minhblocks+1), 1);
	  m_mindblocks = m_maxdblocks;
	  m_maxdblocks = qMin(m_mindblocks + ndblocks, m_enddblocks);
	}

      calculateBlocksForWidthSlice(w);
    }
  else
    emit widthSlice(w, m_level, false, dlimits, hlimits);
}

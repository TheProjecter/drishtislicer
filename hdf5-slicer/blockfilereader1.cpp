#include "blockfilereader.h"
#include <QtGui>

#define LOWRESDEPTHSLICE(T)		\
{					\
  T* block = (T*)m_ssvol;		\
  T* slice = (T*)m_lowresSlice;		\
  for (int iss=0; iss<m_ssw*m_ssh; iss++)   \
    slice[iss] = (1.0-frc)*block[idx0+iss] +\
                       frc*block[idx1+iss]; \
}


uchar*
BlockFileReader::getLowresDepthSlice(int d, int& ssw, int& ssh)
{
  memset(m_lowresSlice, 0, m_ssw*m_ssh*m_bytesPerVoxel); 

  ssw = m_ssw;
  ssh = m_ssh;

  int ds = d/m_sslevel;

  if (ds >= m_ssd-1)
    return m_lowresSlice;

  float frc = (d-ds*m_sslevel)*1.0/m_sslevel;
  int idx0 = ds*m_ssw*m_ssh;
  int idx1 = (ds+1)*m_ssw*m_ssh;
  if (m_sslevel == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }

    if (m_voxelType == _UChar)
      LOWRESDEPTHSLICE(uchar)
    else if (m_voxelType == _Char)
      LOWRESDEPTHSLICE(char)		    
    else if (m_voxelType == _UShort)
      LOWRESDEPTHSLICE(ushort)		    
    else if (m_voxelType == _Short)
      LOWRESDEPTHSLICE(short)		    
    else if (m_voxelType == _Int)
      LOWRESDEPTHSLICE(int)		    
    else if (m_voxelType == _Float)
      LOWRESDEPTHSLICE(float)		    

  return m_lowresSlice;
}

bool
BlockFileReader::depthBlocksPresent(int level, int d,
				    int wstart, int wend,
				    int hstart, int hend)
{
  if (level < 0)
    return true;

  m_startwblocks = wstart/m_blockSize;
  m_endwblocks = qMin(m_wblocks, wend/m_blockSize + 1);

  m_starthblocks = hstart/m_blockSize;
  m_endhblocks = qMin(m_hblocks, hend/m_blockSize + 1);

  m_minhblocks = m_starthblocks;
  m_maxhblocks = qMin(m_minhblocks + 1000, m_endhblocks);

  int nwblocks = qMax(1000/(m_maxhblocks-m_minhblocks+1), 1);
  m_minwblocks = m_startwblocks;
  m_maxwblocks = qMin(m_minwblocks + nwblocks, m_endwblocks);


  int dno = d/m_blockSize;
  int dno1 = dno+1;
  int p2 = qPow(2, level);
  int localSlice = (d%m_blockSize)/p2;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    nextslab = true;

  for(int w=m_minwblocks; w<m_maxwblocks; w++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (dno*m_wblocks*m_hblocks +
		     w*m_hblocks + h);

	if (! m_blockCache[level].contains(blkno))
	  return false;

	if (nextslab)
	  {
	    int blkno1 = (dno1*m_wblocks*m_hblocks +
			 w*m_hblocks + h);

	    if (! m_blockCache[level].contains(blkno1))
	      return false;
	  }
      }
  return true;
}

void
BlockFileReader::getDepthSlice(int level, int d,
				int wstart, int wend,
				int hstart, int hend)
{
  if (level < 0)
    return;

  m_level = level;
  m_currentAxis = 0;
  m_currentSlice = d;

  memset(m_depthSlice, 0, m_width*m_height*m_bytesPerVoxel);

  m_startwblocks = wstart/m_blockSize;
  m_endwblocks = qMin(m_wblocks, wend/m_blockSize + 1);

  m_starthblocks = hstart/m_blockSize;
  m_endhblocks = qMin(m_hblocks, hend/m_blockSize + 1);

  m_minhblocks = m_starthblocks;
  m_maxhblocks = qMin(m_minhblocks + 1000, m_endhblocks);

  int nwblocks = qMax(1000/(m_maxhblocks-m_minhblocks+1), 1);
  m_minwblocks = m_startwblocks;
  m_maxwblocks = qMin(m_minwblocks + nwblocks, m_endwblocks);

  calculateBlocksForDepthSlice(d);
}

void
BlockFileReader::calculateBlocksForDepthSlice(int d)
{
  QVector<int> blockNumbers;

  int dno = d/m_blockSize;

  int dno1 = dno+1;
  int p2 = qPow(2, m_level);
  int localSlice = (d%m_blockSize)/p2;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    nextslab = true;

  for(int w=m_minwblocks; w<m_maxwblocks; w++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (dno*m_wblocks*m_hblocks +
		     w*m_hblocks + h);

	blockNumbers.append(blkno);

	if (nextslab)
	  {
	    int blkno1 = (dno1*m_wblocks*m_hblocks +
			 w*m_hblocks + h);

	    blockNumbers.append(blkno1);
	  }
      }

  //  QMessageBox::information(0, "", QString("level - %1").arg(m_level));

  m_blockReader.loadBlocks(m_level, blockNumbers);
}


#define DEPTHSLICE_1(T)			\
{					\
  T* block = (T*)blockData;		\
  T* interp = (T*)interpData;		\
  for (int iss=0; iss<bb*bb; iss++)	\
  interp[iss] = (1.0-frc)*block[idx0+iss] +\
                      frc*block[idx1+iss]; \
}

#define DEPTHSLICE_2(T)			\
{					\
  T* block = (T*)blockData;		\
  T* block1 = (T*)block1Data;		\
  T* interp = (T*)interpData;		\
  for (int iss=0; iss<bb*bb; iss++)	\
    interp[iss] = (1.0-frc)*block[idx0+iss] + \
                        frc*block1[idx1+iss]; \
}

void
BlockFileReader::depthSliceDone()
{
  if (m_blockCache[m_level].count() == 0)
    return;

  int d = m_currentSlice;

  int p2 = qPow(2, m_level);
  int bb = m_blockSize/p2;
  int newht = m_height/p2;
  int newwd = m_width/p2;

  int dno = d/m_blockSize;
  int localSlice = (d%m_blockSize)/p2;
  

  //----------------------
  // for interpolation between slices for m_level > 0
  int ds = d%m_blockSize;
  uchar *interpData = new uchar[bb*bb*m_bytesPerVoxel];
  float frc = (ds-localSlice*p2)*1.0/p2;
  int idx0 = localSlice*bb*bb;
  int idx1 = (localSlice+1)*bb*bb;
  if (p2 == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }
  int dno1 = dno;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      idx1 = 0;
      dno1 = dno+1;
      nextslab = true;
    }
  //----------------------

  bool doagain = false;
  for(int w=m_minwblocks; w<m_maxwblocks; w++)
    for(int h=m_minhblocks; h<m_maxhblocks; h++)
      {
	int blkno = (dno*m_wblocks*m_hblocks +
		     w*m_hblocks + h);

	int blkno1 = (dno1*m_wblocks*m_hblocks +
		      w*m_hblocks + h);
	
	if (m_blockCache[m_level].contains(blkno) &&
	    m_blockCache[m_level].contains(blkno1) )
	  {
	    uchar *blockData = (m_blockCache[m_level])[blkno];
	    int iwend = qMin(bb, newwd-w*bb);
	    int ihend = qMin(bb, newht-h*bb);
	    if (p2 == 1)
	      {
		for(int iw=0; iw<iwend; iw++)
		  memcpy(m_depthSlice + ((w*bb +iw)*newht + h*bb)*m_bytesPerVoxel,
			 blockData + (localSlice*bb*bb + iw*bb)*m_bytesPerVoxel,
			 ihend*m_bytesPerVoxel);
	      }
	    else
	      {
		if (!nextslab)
		  {
		    if (m_voxelType == _UChar)
		      DEPTHSLICE_1(uchar)
		    else if (m_voxelType == _Char)
		      DEPTHSLICE_1(char)		    
		    else if (m_voxelType == _UShort)
		      DEPTHSLICE_1(ushort)		    
		    else if (m_voxelType == _Short)
		      DEPTHSLICE_1(short)		    
		    else if (m_voxelType == _Int)
		      DEPTHSLICE_1(int)		    
		    else if (m_voxelType == _Float)
		      DEPTHSLICE_1(float)		    
		  }
		else
		  {
		    uchar *block1Data = (m_blockCache[m_level])[blkno1];

		    if (m_voxelType == _UChar)
		      DEPTHSLICE_2(uchar)
		    else if (m_voxelType == _Char)
		      DEPTHSLICE_2(char)		    
		    else if (m_voxelType == _UShort)
		      DEPTHSLICE_2(ushort)		    
		    else if (m_voxelType == _Short)
		      DEPTHSLICE_2(short)		    
		    else if (m_voxelType == _Int)
		      DEPTHSLICE_2(int)		    
		    else if (m_voxelType == _Float)
		      DEPTHSLICE_2(float)		    
		  }

		for(int iw=0; iw<iwend; iw++)
		  memcpy(m_depthSlice + ((w*bb +iw)*newht + h*bb)*m_bytesPerVoxel,
			 interpData + (iw*bb)*m_bytesPerVoxel,
			 ihend*m_bytesPerVoxel);
	      }
	  } // if block present
	else
	  {
	    doagain = true;
	    break;
	  }
      }

  delete [] interpData;

  if (doagain)
    {
      calculateBlocksForDepthSlice(d);
      return;
    }

  int minw = m_startwblocks*bb;
  int maxw = qMin(newwd-1, m_maxwblocks*bb-1);

  int minh = m_starthblocks*bb;
  int maxh = qMin(newht-1, m_maxhblocks*bb-1);

  QPair<int, int> hlimits = qMakePair(minh, maxh);
  QPair<int, int> wlimits = qMakePair(minw, maxw);

  if (m_maxwblocks < m_endwblocks)
    {
      emit depthSlice(d, m_level, true, wlimits, hlimits);

      m_minhblocks = m_maxhblocks;
      m_maxhblocks = qMin(m_minhblocks + 1000, m_endhblocks);

      if (m_minhblocks >= m_endhblocks)
	{
	  m_minhblocks = m_starthblocks;
	  m_maxhblocks = qMin(m_minhblocks + 1000, m_endhblocks);

	  int nwblocks = qMax(1000/(m_maxhblocks-m_minhblocks+1), 1);
	  m_minwblocks = m_maxwblocks;
	  m_maxwblocks = qMin(m_minwblocks + nwblocks, m_endwblocks);
	}

      calculateBlocksForDepthSlice(d);
    }
  else
    emit depthSlice(d, m_level, false, wlimits, hlimits);
}

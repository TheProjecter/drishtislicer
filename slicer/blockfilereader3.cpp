#include "blockfilereader.h"
#include <QtGui>

#define LOWRESHEIGHTSLICE(T)		  \
{					  \
  T* block = (T*)m_ssvol;		  \
  T* slice = (T*)m_lowresSlice;		  \
  int iss = 0;				  \
  for(int i=0; i<m_ssd; i++)		  \
    for(int j=0; j<m_ssw; j++)		  \
      {					  \
	slice[iss] = (1.0-frc)*block[i*m_ssw*m_ssh + j*m_ssh + idx0] +\
	                   frc*block[i*m_ssw*m_ssh + j*m_ssh + idx1]; \
	iss++;							      \
      }								      \
}

uchar*
BlockFileReader::getLowresHeightSlice(int h, int& ssd, int& ssw)
{
  memset(m_lowresSlice, 0, m_ssd*m_ssw*m_bytesPerVoxel);
 
  ssd = m_ssd;
  ssw = m_ssw;

  int hs = h/m_sslevel;

  if (hs >= m_ssh-1)
    return m_lowresSlice;

  float frc = (h-hs*m_sslevel)*1.0/m_sslevel;
  int idx0 = hs;
  int idx1 = hs+1;
  if (m_sslevel == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }

    if (m_voxelType == _UChar)
      LOWRESHEIGHTSLICE(uchar)
    else if (m_voxelType == _Char)
      LOWRESHEIGHTSLICE(char)		    
    else if (m_voxelType == _UShort)
      LOWRESHEIGHTSLICE(ushort)		    
    else if (m_voxelType == _Short)
      LOWRESHEIGHTSLICE(short)		    
    else if (m_voxelType == _Int)
      LOWRESHEIGHTSLICE(int)		    
    else if (m_voxelType == _Float)
      LOWRESHEIGHTSLICE(float)		    

  return m_lowresSlice;
}

void
BlockFileReader::getHeightSlice(int level, int h,
				 int dstart, int dend,
				 int wstart, int wend)
{
  if (level < 0)
    return;

  m_level = level;
  m_currentAxis = 2;
  m_currentSlice = h;

  memset(m_heightSlice, 0, m_depth*m_width*m_bytesPerVoxel);

  m_startdblocks = dstart/m_blockSize;
  m_enddblocks = qMin(m_dblocks, dend/m_blockSize + 1);

  m_startwblocks = wstart/m_blockSize;
  m_endwblocks = qMin(m_wblocks, wend/m_blockSize + 1);

  m_minwblocks = m_startwblocks;
  m_maxwblocks = qMin(m_minwblocks + 400, m_endwblocks);

  int ndblocks = qMax(400/(m_maxwblocks-m_minwblocks+1), 1);
  m_mindblocks = m_startdblocks;
  m_maxdblocks = qMin(m_mindblocks + ndblocks, m_enddblocks);

  calculateBlocksForHeightSlice(h);
}

void
BlockFileReader::calculateBlocksForHeightSlice(int h)
{
  QVector<int> blockNumbers;

  int hno = h/m_blockSize;

  int hno1 = hno;
  int p2 = qPow(2, m_level);
  int localSlice = (h%m_blockSize)/p2;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      hno1 = hno+1;
      nextslab = true;
    }

  for(int d=m_mindblocks; d<m_maxdblocks; d++)
    for(int w=m_minwblocks; w<m_maxwblocks; w++)
      {
	int blkno = (d*m_wblocks*m_hblocks +
		     w*m_hblocks + hno);

	blockNumbers.append(blkno);

	if (nextslab)
	  {
	    int blkno1 = (d*m_wblocks*m_hblocks +
			 w*m_hblocks + hno1);

	    blockNumbers.append(blkno1);
	  }
      }

  m_blockReader.loadBlocks(m_level, blockNumbers);
}


#define HEIGHTSLICE_1(T)		\
{					\
  T* block = (T*)blockData;		\
  T* slice = (T*)m_heightSlice;		\
  for(int id=0; id<idend; id++)		\
    for(int iw=0; iw<iwend; iw++)	\
      {					\
	slice[(d*bb+id)*newwd + (w*bb+iw)] =	\
	  block[id*bb*bb + iw*bb + localSlice];	\
      }					\
}

#define HEIGHTSLICE_2(T)		\
{					\
  T* block = (T*)blockData;		\
  T* slice = (T*)m_heightSlice;		\
  for(int id=0; id<idend; id++)		\
    for(int iw=0; iw<iwend; iw++)	\
      {					\
	slice[(d*bb+id)*newwd + (w*bb+iw)] =	     \
	  (1.0-frc)*block[id*bb*bb + iw*bb + idx0] + \
	        frc*block[id*bb*bb + iw*bb + idx1];  \
      }					\
}

#define HEIGHTSLICE_3(T)		\
{					\
  T* block = (T*)blockData;		\
  T* block1 = (T*)block1Data;		\
  T* slice = (T*)m_heightSlice;		\
  for(int id=0; id<idend; id++)		\
    for(int iw=0; iw<iwend; iw++)	\
      {					\
	slice[(d*bb+id)*newwd + (w*bb+iw)] =	     \
	  (1.0-frc)*block[id*bb*bb + iw*bb + idx0] + \
	       frc*block1[id*bb*bb + iw*bb + idx1];  \
      }					\
}

void
BlockFileReader::heightSliceDone()
{
  int h = m_currentSlice;

  int p2 = qPow(2, m_level);
  int bb = m_blockSize/p2;
  int newwd = m_width/p2;
  int newdp = m_depth/p2;

  int hno = h/m_blockSize;
  int localSlice = (h%m_blockSize)/p2;

  //----------------------
  // for interpolation between slices for m_level > 0
  int hs = h%m_blockSize;
  float frc = (hs-localSlice*p2)*1.0/p2;
  int idx0 = localSlice;
  int idx1 = localSlice+1;
  if (p2 == 1)
    {
      idx1 = idx0;
      frc = 0.0;
    }
  int hno1 = hno;
  bool nextslab = false;
  if (p2*(localSlice+1)>=m_blockSize)
    {
      idx1 = 0;
      hno1 = hno+1;
      nextslab = true;
    }
  //----------------------


  bool doagain = false;
  for(int d=m_mindblocks; d<m_maxdblocks; d++)
    for(int w=m_minwblocks; w<m_maxwblocks; w++)
      {
	int blkno = (d*m_wblocks*m_hblocks +
		     w*m_hblocks + hno);
	int blkno1 = (d*m_wblocks*m_hblocks +
		      w*m_hblocks + hno1);

	if (m_blockCache[m_level].contains(blkno) &&
	    m_blockCache[m_level].contains(blkno1))
	  {
	    uchar *blockData = (m_blockCache[m_level])[blkno];
	    
	    int idend = qMin(bb, newdp-d*bb);
	    int iwend = qMin(bb, newwd-w*bb);

	    if (p2 == 1)
	      {
		if (m_voxelType == _UChar)
		  HEIGHTSLICE_1(uchar)
		else if (m_voxelType == _Char)
		  HEIGHTSLICE_1(char)		    
		else if (m_voxelType == _UShort)
		  HEIGHTSLICE_1(ushort)		    
		else if (m_voxelType == _Short)
		  HEIGHTSLICE_1(short)		    
		else if (m_voxelType == _Int)
		  HEIGHTSLICE_1(int)		    
		else if (m_voxelType == _Float)
		  HEIGHTSLICE_1(float)		    

//		for(int id=0; id<idend; id++)
//		  for(int iw=0; iw<iwend; iw++)
//		    {
//		      m_heightSlice[(d*bb+id)*newwd +
//				    (w*bb+iw)] = 
//			block[id*bb*bb +
//			      iw*bb + localSlice];
//		    }
	      }
	    else
	      {
		if (!nextslab)
		  {
		    if (m_voxelType == _UChar)
		      HEIGHTSLICE_2(uchar)
		    else if (m_voxelType == _Char)
		      HEIGHTSLICE_2(char)		    
		    else if (m_voxelType == _UShort)
		      HEIGHTSLICE_2(ushort)		    
		    else if (m_voxelType == _Short)
		      HEIGHTSLICE_2(short)		    
		    else if (m_voxelType == _Int)
		      HEIGHTSLICE_2(int)		    
		    else if (m_voxelType == _Float)
		      HEIGHTSLICE_2(float)		    

//		    for(int id=0; id<idend; id++)
//		      for(int iw=0; iw<iwend; iw++)
//			{
//			  m_heightSlice[(d*bb+id)*newwd +
//					(w*bb+iw)] = 
//			    (1.0-frc)*block[id*bb*bb +
//					    iw*bb + idx0] +
//			          frc*block[id*bb*bb +
//					    iw*bb + idx1];
//			}
		  }
		else
		  {
		    uchar *block1Data = (m_blockCache[m_level])[blkno1];

		    if (m_voxelType == _UChar)
		      HEIGHTSLICE_3(uchar)
		    else if (m_voxelType == _Char)
		      HEIGHTSLICE_3(char)		    
		    else if (m_voxelType == _UShort)
		      HEIGHTSLICE_3(ushort)		    
		    else if (m_voxelType == _Short)
		      HEIGHTSLICE_3(short)		    
		    else if (m_voxelType == _Int)
		      HEIGHTSLICE_3(int)		    
		    else if (m_voxelType == _Float)
		      HEIGHTSLICE_3(float)		    

//		    for(int id=0; id<idend; id++)
//		      for(int iw=0; iw<iwend; iw++)
//			{
//			  m_heightSlice[(d*bb+id)*newwd +
//					(w*bb+iw)] = 
//			    (1.0-frc)*block[id*bb*bb +
//					    iw*bb + idx0] +
//			          frc*block1[id*bb*bb +
//					     iw*bb + idx1];
//			}
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
      calculateBlocksForHeightSlice(h);
      return;
    }

  int mind = m_startdblocks*bb;
  int maxd = qMin(newdp-1, m_maxdblocks*bb-1);

  int minw = m_startwblocks*bb;
  int maxw = qMin(newwd-1, m_maxwblocks*bb-1);

  QPair<int, int> wlimits = qMakePair(minw, maxw);
  QPair<int, int> dlimits = qMakePair(mind, maxd);

  if (m_maxdblocks < m_enddblocks)
    {
      emit heightSlice(h, m_level, true, dlimits, wlimits);

      m_minwblocks = m_maxwblocks;
      m_maxwblocks = qMin(m_minwblocks + 400, m_endwblocks);

      if (m_minwblocks >= m_endwblocks)
	{
	  m_minwblocks = m_startwblocks;
	  m_maxwblocks = qMin(m_minwblocks + 400, m_endwblocks);

	  int ndblocks = qMax(400/(m_maxwblocks-m_minwblocks+1), 1);
	  m_mindblocks = m_maxdblocks;
	  m_maxdblocks = qMin(m_mindblocks + ndblocks, m_enddblocks);
	}

      calculateBlocksForHeightSlice(h);
    }
  else
    emit heightSlice(h, m_level, false, dlimits, wlimits);
}


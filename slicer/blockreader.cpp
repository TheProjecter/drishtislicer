#include <blockreader.h>

BlockReader::BlockReader(QObject *parent) : QThread(parent)
{
  m_level = 0;
  m_maxCacheSize = 100;
  m_blockSize = 32;
  m_bytesPerVoxel = 1;
  m_baseFilename.clear();
  m_dblocks = m_wblocks = m_hblocks = 0;
  m_blkno.clear();

  for (int il=0; il<10; il++)
    {
      m_prevfno[il] = -1;
      m_blockCache[il] = 0;
      m_lru[il].clear();
      m_uniform[il].clear();
      m_fileBlocks[il].clear();
      m_blockOffset[il].clear();
    }

  m_interrupt = false;  
}

BlockReader::~BlockReader()
{
  if (isRunning())
    {
      m_mutex.lock();
      m_interrupt = true;
      m_condition.wakeOne();
      m_mutex.unlock();
    }

  m_baseFilename.clear();

  for (int il=0; il<10; il++)
    {
      if (m_qfile[il].isOpen())
	m_qfile[il].close();

      m_lru[il].clear();
      m_uniform[il].clear();
      m_fileBlocks[il].clear();
      for(int i=0; i<m_blockOffset[il].count(); i++)
	(m_blockOffset[il])[i].clear();
      m_blockOffset[il].clear();
    }
}

void BlockReader::setMaxCacheSize(int mcs) { m_maxCacheSize = mcs; }
void BlockReader::setBlockSize(int bs) { m_blockSize = bs; }
void BlockReader::setBytesPerVoxel(int bpv) { m_bytesPerVoxel = bpv; }
void BlockReader::setBaseFilename(QString bf) { m_baseFilename = bf; }
void BlockReader::setBlockGridSize(int dno, int wno, int hno)
{
  m_dblocks = dno;
  m_wblocks = wno;
  m_hblocks = hno;
}

void BlockReader::setUniform(int l, QBitArray u) { m_uniform[l] = u; }
void BlockReader::setFileBlocks(int l, QVector<int> fb) { m_fileBlocks[l] = fb; }
void BlockReader::setBlockOffset(int l, QList< QVector<qint64> > bo) { m_blockOffset[l] = bo; }
void BlockReader::setBlockCache(int l, QHash<long, uchar*> *bc) { m_blockCache[l] = bc; }

void
BlockReader::getBlock(int level, int blkno, uchar* block)
{
  int bno = -1;
  int fno = -1;

  if (blkno < m_fileBlocks[level].at(0))
    {
      fno = 0;
      bno = blkno;
    }
  else
    {
      for (int fb=1; fb<m_fileBlocks[level].count(); fb++)
	{
	  if (blkno < m_fileBlocks[level].at(fb))
	    {
	      fno = fb;
	      bno = blkno - m_fileBlocks[level].at(fb-1);
	      break;
	    }
	}
    }


  qint64 seekblock;
  if (bno > 0)
    seekblock = m_blockOffset[level][fno].at(bno-1);
  else
    seekblock = 0;


  if (m_prevfno[level] != fno)
    {
      if (m_prevfno[level] != -1)
	m_qfile[level].close();

      m_prevfno[level] = fno;

      QString filename;
      filename = m_baseFilename +
	         QString(".%1").arg(m_blockSize/qPow(2, level)) +
	         QString(".%1").arg(m_prevfno[level]+1, 3, 10, QChar('0'));

      m_qfile[level].setFileName(filename);
      m_qfile[level].open(QFile::ReadOnly);
    }
  m_qfile[level].seek(seekblock);
  if (!m_uniform[level].at(blkno))
    m_qfile[level].read((char*)block, m_bpb);
  else
    {
      uchar v0[4];
      m_qfile[level].read((char*)&v0, m_bytesPerVoxel);
      if (m_bytesPerVoxel == 1)
	memset(block, v0[0], m_bpb);
      else if (m_bytesPerVoxel == 2)
	{
	  for(int b=0; b<m_bpb/m_bytesPerVoxel; b++)
	    {
	      block[2*b+0] = v0[0];
	      block[2*b+1] = v0[1];
	    }
	}
      else if (m_bytesPerVoxel == 4)
	{
	  for(int b=0; b<m_bpb/m_bytesPerVoxel; b++)
	    {
	      block[4*b+0] = v0[0];
	      block[4*b+1] = v0[1];
	      block[4*b+2] = v0[2];
	      block[4*b+3] = v0[3];
	    }
	}
    }
}


void
BlockReader::loadBlocks(int level, QVector<int> blkno)
{
  QMutexLocker locker(&m_mutex);

  m_level = level;
  m_blkno = blkno;

  if (!isRunning())
    {
      start();
    }
  else
    {
      m_interrupt = true;
      m_condition.wakeOne();
    }
}

void
BlockReader::stopLoading()
{
  if (isRunning())
    {
      m_interrupt = true;
      m_condition.wakeOne();
    }
}

void
BlockReader::run()
{
  forever
    {
      loadLevel(m_level);

      m_mutex.lock();

      if (!m_interrupt)
	{
	  emit readDone();
	  m_condition.wait(&m_mutex);
	}

      m_interrupt = false;

      m_mutex.unlock();
    }
}

void
BlockReader::loadLevel(int level)
{
  //QMutexLocker locker(&m_mutex);

  QHash<long, uchar*> *blockCache = m_blockCache[level];

  int bb = m_blockSize/qPow(2, level);
  m_bpb = bb*bb*bb*m_bytesPerVoxel;

  for(int b=0; b<m_blkno.count(); b++)
    {
      if (m_interrupt)
	break;
	  
      int blkno = m_blkno.at(b);

      if (m_lru[level].contains(blkno))
	m_lru[level].removeOne(blkno);

      if (! blockCache->contains(blkno))
	{
	  uchar* block;

	  if (blockCache->size() < m_maxCacheSize)	      
	    block = new uchar[m_bpb];
	  else
	    {
	      block = (*blockCache)[m_lru[level][0]];
	      blockCache->remove(m_lru[level][0]);
	      m_lru[level].removeFirst();
	    }
	  
	  getBlock(level, blkno, block);
	  
	  blockCache->insert(blkno, block);
	}
      
      m_lru[level].append(blkno);
    }      
}

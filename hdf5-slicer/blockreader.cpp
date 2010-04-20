#include <blockreader.h>

BlockReader::BlockReader(QObject *parent) : QThread(parent)
{
  m_level = 0;
  m_maxCacheSize = 100;
  m_blockSize = 32;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;
  m_baseFilename.clear();
  m_dblocks = m_wblocks = m_hblocks = 0;
  m_blkno.clear();

  m_hdf5file = 0;
  for (int il=0; il<10; il++)
    {
      m_prevfno[il] = -1;
      m_blockCache[il] = 0;
      m_lru[il].clear();
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

  if (m_hdf5file)
    m_hdf5file->close();

  for (int il=0; il<10; il++)
    m_lru[il].clear();

}

void BlockReader::setMaxCacheSize(int mcs) { m_maxCacheSize = mcs; }
void BlockReader::setBlockSize(int bs) { m_blockSize = bs; }
void BlockReader::setVoxelType(int vt) { m_voxelType = vt; }
void BlockReader::setBytesPerVoxel(int bpv) { m_bytesPerVoxel = bpv; }
void BlockReader::setBaseFilename(QString bf) { m_baseFilename = bf; }
void BlockReader::setMinLevel(int ml) { m_minLevel = ml; }
void BlockReader::setBlockGridSize(int dno, int wno, int hno)
{
  m_dblocks = dno;
  m_wblocks = wno;
  m_hblocks = hno;
}

void BlockReader::setBlockCache(int l, QHash<long, uchar*> *bc) { m_blockCache[l] = bc; }

void
BlockReader::getBlock(int level, int blkno, uchar* block)
{
  int bb = m_blockSize/qPow(2, level);

  int dno = blkno/(m_wblocks*m_hblocks);
  int wno = (blkno - dno*m_wblocks*m_hblocks)/m_hblocks;
  int hno = blkno - dno*m_wblocks*m_hblocks - wno*m_hblocks;

  if (! m_hdf5file)
    {
      QString filename;
      filename = m_baseFilename + ".h5";

      m_hdf5file = new H5File(filename.toAscii().data(),
			      H5F_ACC_RDONLY);

      for(int il=0; il<m_minLevel; il++)
	{
	  QString dataname = QString("lod-%1").arg(il);
	  m_hdf5dataset[il] = m_hdf5file->openDataSet(dataname.toAscii().data());
	}
    }

  DataSpace dataspace = m_hdf5dataset[level].getSpace();

  hsize_t offset[3], count[3];
  offset[0] = dno*bb;
  offset[1] = wno*bb;
  offset[2] = hno*bb;
  count[0] = count[1] = count[2] = bb;

  dataspace.selectHyperslab( H5S_SELECT_SET, count, offset );

  hsize_t dimsm[3];
  dimsm[0] = bb;
  dimsm[1] = bb;
  dimsm[2] = bb;
  DataSpace memspace( 3, dimsm );

  if (m_voxelType == _UChar)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_UCHAR,
			       memspace,
			       dataspace );
  else if (m_voxelType == _Char)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_CHAR,
			       memspace,
			       dataspace );
  else if (m_voxelType == _UShort)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_USHORT,
			       memspace,
			       dataspace );
  else if (m_voxelType == _Short)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_SHORT,
			       memspace,
			       dataspace );
  else if (m_voxelType == _Int)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_INT,
			       memspace,
			       dataspace );
  else if (m_voxelType == _Float)
    m_hdf5dataset[level].read( block,
			       PredType::NATIVE_FLOAT,
			       memspace,
			       dataspace );
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
	  
	  //memset(block, 120, m_bpb);
	  getBlock(level, blkno, block);
	  
	  blockCache->insert(blkno, block);
	}
      
      m_lru[level].append(blkno);
    }      
}

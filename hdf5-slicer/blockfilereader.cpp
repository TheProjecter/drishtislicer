#include "blockfilereader.h"
#include <QtGui>

#include "H5Cpp.h"
using namespace H5;

BlockFileReader::BlockFileReader()
{
  m_dumpSlice = false;
  m_lowresSlice = 0;
  m_slice = 0;

  m_depthSlice = 0;
  m_widthSlice = 0;
  m_heightSlice = 0;

  m_maxFileSize = 1024*1024*1024;
  m_blockSize = 32;
  m_maxCacheSize = 1000;

  m_prevfno = -1;
  m_minLevel = 0;

  reset();

  m_currentAxis = m_currentSlice = -1;

  connect(&m_blockReader, SIGNAL(readDone()),
	  this, SLOT(readDone()));
}

BlockFileReader::~BlockFileReader()
{
  m_blockReader.terminate();
  m_blockReader.wait();

  reset();
}

void BlockFileReader::setMaxCacheSize(int mcs) { m_maxCacheSize = mcs; }
int BlockFileReader::maxCacheSize() { return m_maxCacheSize; }
int BlockFileReader::minLevel() { return m_minLevel; }

uchar* BlockFileReader::depthSlice() { return m_depthSlice;}
uchar* BlockFileReader::widthSlice() { return m_widthSlice;}
uchar* BlockFileReader::heightSlice() { return m_heightSlice;}

void
BlockFileReader::reset()
{
  if (m_qfile.isOpen())
    m_qfile.close();

  m_prevfno = -1;

  m_filename.clear();
  m_baseFilename.clear();
  m_header = 0;
  m_depth = m_width = m_height = 0;
  m_voxelType = _UChar;
  m_bytesPerVoxel = 1;

  m_totBlocks = 0;
  m_dblocks = m_wblocks = m_hblocks = 0;

  clearCache();

  for (int ib=0; ib<10; ib++)
    m_blockCache[ib].clear();  

  m_dumpSlice = false;

  if (m_lowresSlice)
    delete [] m_lowresSlice;
  m_lowresSlice = 0;

  if (m_slice)
    delete [] m_slice;
  m_slice = 0;

  if (m_depthSlice)
    delete [] m_depthSlice;
  m_depthSlice = 0;

  if (m_widthSlice)
    delete [] m_widthSlice;
  m_widthSlice = 0;

  if (m_heightSlice)
    delete [] m_heightSlice;
  m_heightSlice = 0;

  m_prevfno = -1;
}

void
BlockFileReader::clearCache()
{
  for(int ib=0; ib<10; ib++)
    {
      QList<uchar*> blocks = m_blockCache[ib].values();
      for(int i=0; i<blocks.count(); i++)
	delete [] blocks[i];

      m_blockCache[ib].clear();
    }
}

QString BlockFileReader::baseFilename() { return m_baseFilename; }
int BlockFileReader::totalBlocks() { return m_totBlocks; }
qint64 BlockFileReader::maxFileSize() { return m_maxFileSize; }
int BlockFileReader::blockSize() { return m_blockSize; }
int BlockFileReader::depth() { return m_depth; }
int BlockFileReader::width() { return m_width; }
int BlockFileReader::height() { return m_height; }
int BlockFileReader::dblocks() { return m_dblocks; }
int BlockFileReader::wblocks() { return m_wblocks; }
int BlockFileReader::hblocks() { return m_hblocks; }
void BlockFileReader::getLowresGrid(int &sslevel,
				     int& ssd, int& ssw, int& ssh)
{
  ssd = m_ssd;
  ssw = m_ssw;
  ssh = m_ssh;
  sslevel = m_sslevel;
}

void BlockFileReader::setBaseFilename(QString bfn) { m_baseFilename = bfn; }
void BlockFileReader::setDepth(int d) { m_depth = d; }
void BlockFileReader::setWidth(int w) { m_width = w; }
void BlockFileReader::setHeight(int h) { m_height = h; }
void BlockFileReader::setHeaderSize(int hs) { m_header = hs; }
void BlockFileReader::setMaxFileSize(qint64 bpf) { m_maxFileSize = bpf; }
void BlockFileReader::setBlockSize(int bs) { m_blockSize = bs; }
void BlockFileReader::setVoxelType(int vt)
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

QString BlockFileReader::fileName() { return m_filename; }

bool
BlockFileReader::exists()
{
  if (!m_depthSlice)
    m_depthSlice = new uchar[m_width*m_height*m_bytesPerVoxel];
  if (!m_widthSlice)
    m_widthSlice = new uchar[m_depth*m_height*m_bytesPerVoxel];
  if (!m_heightSlice)
    m_heightSlice = new uchar[m_depth*m_width*m_bytesPerVoxel];

  m_dblocks = ((m_depth/m_blockSize) + (m_depth%m_blockSize > 0));
  m_wblocks = ((m_width/m_blockSize) + (m_width%m_blockSize > 0));
  m_hblocks = ((m_height/m_blockSize)+ (m_height%m_blockSize > 0));
  m_dblocks = qMax(1, m_dblocks);
  m_wblocks = qMax(1, m_wblocks);
  m_hblocks = qMax(1, m_hblocks);

  m_totBlocks = m_dblocks * m_wblocks * m_hblocks;

//  QMessageBox::information(0, "", QString("%1 %2 %3").\
//			   arg(m_dblocks).arg(m_wblocks).arg(m_hblocks));

  int ld = qMax(m_depth, qMax(m_width, m_height));
  m_lowresSlice = new uchar[ld*ld*m_bytesPerVoxel];

  loadDict();

  return true;
}

void
BlockFileReader::startSliceBlock()
{
  int bb = m_blockSize;
  int bpb = bb*bb*bb*m_bytesPerVoxel;

  if (m_slice)
    delete [] m_slice;

  m_slice = new uchar[m_wblocks*m_hblocks*bpb];
  memset(m_slice, 0, m_wblocks*m_hblocks*bpb);
}

void
BlockFileReader::endSliceBlock()
{
  if (m_slice)
    delete [] m_slice;
  m_slice = 0;
}

uchar*
BlockFileReader::getSliceBlock(int slc)
{
  return m_slice;
}

void
BlockFileReader::getBlock(int dno, int wno, int hno,
			   uchar* block)
{
}

void
BlockFileReader::loadDict()
{
  QString flnm = m_baseFilename + ".h5";
  H5File h5file(flnm.toAscii().data(),
		H5F_ACC_RDONLY);

  DataSet dataset = h5file.openDataSet("lowres");

  IntType datatype;
  datatype.copy(dataset);

  Attribute attrib = dataset.openAttribute("sslevel");
  attrib.read(PredType::NATIVE_UCHAR, &m_sslevel);

  DataSpace dataspace = dataset.getSpace();
  hsize_t dims[3];
  int ndims = dataspace.getSimpleExtentDims(dims, NULL);
  m_ssd = dims[0];
  m_ssw = dims[1];
  m_ssh = dims[2];

  DataSpace memspace(3, dims);

  m_ssvol = new uchar[m_ssd*m_ssw*m_ssh*m_bytesPerVoxel];
  dataset.read( m_ssvol,
		datatype,
		memspace,
		dataspace );

  h5file.close();

  m_minLevel = 0;
  if (m_sslevel > 0)
    m_minLevel = logf(m_sslevel)/logf(2.0);

  //  QMessageBox::information(0, "", QString("%1 %2").arg(m_minLevel).arg(m_sslevel));

  initializeBlockReader();
}

void
BlockFileReader::initializeBlockReader()
{
  m_blockReader.start();
  m_blockReader.setMaxCacheSize(m_maxCacheSize);
  m_blockReader.setBlockSize(m_blockSize);
  m_blockReader.setVoxelType(m_voxelType);
  m_blockReader.setBytesPerVoxel(m_bytesPerVoxel);
  m_blockReader.setBaseFilename(m_baseFilename);
  m_blockReader.setGridSize(m_depth, m_width, m_height);
  m_blockReader.setBlockGridSize(m_dblocks, m_wblocks, m_hblocks);
  m_blockReader.setMinLevel(m_minLevel);

  for (int ib=0; ib<m_minLevel; ib++)
    m_blockReader.setBlockCache(ib, &m_blockCache[ib]);
}

void
BlockFileReader::readDone()
{
  if (m_currentAxis == 0) depthSliceDone();
  if (m_currentAxis == 1) widthSliceDone();
  if (m_currentAxis == 2) heightSliceDone();
}


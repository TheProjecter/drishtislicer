//#include <netcdfcpp.h>
//#include "remapncvolume.h"
//
//RemapNcVolume::RemapNcVolume()
//{
//  m_fileName.clear();
//  m_varName.clear();
//
//  m_depth = m_width = m_height = 0;
//  m_depthList.clear();
//  m_voxelType = _UChar;
//  m_bytesPerVoxel = 1;
//
//  m_headerBytes = 0;
//
//  m_rawMin = m_rawMax = 0;
//  m_histogram.clear();
//
//  m_rawMap.clear();
//  m_pvlMap.clear();
//
//  m_image = 0;
//}
//
//RemapNcVolume::~RemapNcVolume()
//{
//  m_fileName.clear();
//  m_varName.clear();
//
//  m_depth = m_width = m_height = 0;
//  m_voxelType = _UChar;
//  m_bytesPerVoxel = 1;
//
//  m_rawMin = m_rawMax = 0;
//  m_histogram.clear();
//
//  m_rawMap.clear();
//  m_pvlMap.clear();
//
//  if (m_image)
//    delete [] m_image;
//  m_image = 0;
//}
//
//void
//RemapNcVolume::setMinMax(float rmin, float rmax)
//{
//  m_rawMin = rmin;
//  m_rawMax = rmax;
//  
//  generateHistogram();
//}
//
//void
//RemapNcVolume::setMap(QList<float> rm,
//		       QList<uchar> pm)
//{
//  m_rawMap = rm;
//  m_pvlMap = pm;
//}
//
//float RemapNcVolume::rawMin() { return m_rawMin; }
//float RemapNcVolume::rawMax() { return m_rawMax; }
//QList<uint> RemapNcVolume::histogram() { return m_histogram; }
//
//void
//RemapNcVolume::gridSize(int& d, int& w, int& h)
//{
//  d = m_depth;
//  w = m_width;
//  h = m_height;
//}
//
//QList<QString>
//RemapNcVolume::listAllVariables()
//{
//  QList<QString> varNames;
//
//  NcError err(NcError::verbose_nonfatal);
//
//  NcFile dataFile((char*)m_fileName[0].toAscii().data(),
//		  NcFile::ReadOnly);
//
//  if (!dataFile.is_valid())
//    {
//      QMessageBox::information(0, "Error",
//			       QString("%1 is not a valid NetCDF file").arg(m_fileName[0]));
//      return varNames; // empty
//    }
//
//  int nvars = dataFile.num_vars();
//  
//  int i;
//  for (i=0; i < nvars; i++)
//    {
//      NcVar *var;
//      var = dataFile.get_var(i);
//
//      varNames.append(var->name());
//    }
//
//  dataFile.close();
//
//  if (varNames.size() == 0)
//    QMessageBox::information(0, "Error", "No variables found in the file");
//
//  return varNames;
//}
//
//bool
//RemapNcVolume::setFile(QList<QString> fl)
//{
//  m_fileName = fl;
//
//  QList<QString> varNames;
//  QList<QString> allVars = listAllVariables();
//
//  if (allVars.size() == 0)
//    return false;
//
//  NcError err(NcError::verbose_nonfatal);
//
//  NcFile dataFile((char*)m_fileName[0].toAscii().data(),
//		  NcFile::ReadOnly);
//
//  if (!dataFile.is_valid())
//    {
//      QMessageBox::information(0, "Error",
//			       QString("%1 is not a valid NetCDF file").arg(m_fileName[0]));
//      return false;
//    }
//
//  //---------------------------------------------------------
//  // -- Choose a variable for extraction --------------------
//  for(uint i=0; i<allVars.size(); i++)
//    {
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)allVars[i].toAscii().data());
//      if (ncvar->num_dims() == 3)
//	varNames.append(allVars[i]);
//    }
//  if (varNames.size() == 0)
//    {
//      QMessageBox::information(0, "Error", "No 3D variables found in the file");
//      return false;
//    }
//
//  if (varNames.size() == 1)
//    {
//      m_varName = varNames[0];
//    }
//  else
//    {
//      bool ok;
//      QString varName;  
//      varName = QInputDialog::getItem(0,
//				      "Choose a variable for extraction",
//				      "Variables",
//				      varNames,
//				      0,
//				      false,
//				      &ok);
//      if (ok)
//	m_varName = varName;
//      else
//	{
//	  m_varName = varNames[0];
//	  QMessageBox::information(0, "Variable", QString("extracting %1").arg(m_varName));
//	}
//    }
//  //---------------------------------------------------------
//
//  NcVar *ncvar;
//  ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//
//  m_voxelType = _UChar;
//  switch (ncvar->type())
//    {	  
//    case ncByte :
//      m_voxelType = _UChar; break;
//    case ncChar :
//      m_voxelType = _Char; break;
//    case ncShort :
//      m_voxelType = _UShort; break;
//    case ncInt :
//      m_voxelType = _Int; break;
//    case ncFloat :
//      m_voxelType = _Float; break;
//    }
//
//  long sizes[100];
//  memset(sizes, 0, 400);
//  for(uint i=0; i<ncvar->num_dims(); i++)
//    sizes[i] = ncvar->get_dim(i)->size();
//
//  dataFile.close();
//
//  //m_depth = sizes[0];
//  m_width = sizes[1];
//  m_height = sizes[2];
//
//
//  m_bytesPerVoxel = 1;
//  if (m_voxelType == _UChar) m_bytesPerVoxel = 1;
//  else if (m_voxelType == _Char) m_bytesPerVoxel = 1;
//  else if (m_voxelType == _UShort) m_bytesPerVoxel = 2;
//  else if (m_voxelType == _Short) m_bytesPerVoxel = 2;
//  else if (m_voxelType == _Int) m_bytesPerVoxel = 4;
//  else if (m_voxelType == _Float) m_bytesPerVoxel = 4;
//
//  // ---------------------
//  m_depth = 0;
//  m_depthList.clear();
//  for(uint i=0; i<m_fileName.size(); i++)
//    {
//      NcFile ncfile((char*)m_fileName[i].toAscii().data(),
//		      NcFile::ReadOnly);
//      
//      if (!ncfile.is_valid())
//	{
//	  QMessageBox::information(0, "Error",
//				   QString("%1 is not a valid NetCDF file").arg(m_fileName[i]));
//	  return false;
//	}
//      NcVar *ncvar;
//      ncvar = ncfile.get_var((char *)m_varName.toAscii().data());
//      m_depth += ncvar->get_dim(0)->size();
//      m_depthList.append(m_depth);
//      ncfile.close();
//    }
//  // ---------------------
//
//
//  if (m_voxelType == _UChar ||
//      m_voxelType == _Char ||
//      m_voxelType == _UShort ||
//      m_voxelType == _Short)
//    {
//      findMinMaxandGenerateHistogram();
//    }
//  else
//    {
//      findMinMax();
//      generateHistogram();
//    }
//
//  m_rawMap.append(m_rawMin);
//  m_rawMap.append(m_rawMax);
//  m_pvlMap.append(0);
//  m_pvlMap.append(255);
//
//  return true;
//}
//
//#define MINMAXANDHISTOGRAM()				\
//  {							\
//    for(uint j=0; j<nY*nZ; j++)				\
//      {							\
//	int val = ptr[j];				\
//	m_rawMin = qMin(m_rawMin, (float)val);		\
//	m_rawMax = qMax(m_rawMax, (float)val);		\
//							\
//	int idx = val-rMin;				\
//	m_histogram[idx]++;				\
//      }							\
//  }
//
//void
//RemapNcVolume::findMinMaxandGenerateHistogram()
//{
//  QProgressDialog progress("Generating Histogram",
//			   "Cancel",
//			   0, 100,
//			   0);
//  progress.setMinimumDuration(0);
//
//  float rSize;
//  float rMin;
//  m_histogram.clear();
//  if (m_voxelType == _UChar ||
//      m_voxelType == _Char)
//    {
//      if (m_voxelType == _UChar) rMin = 0;
//      if (m_voxelType == _Char) rMin = -127;
//      rSize = 255;
//      for(uint i=0; i<256; i++)
//	m_histogram.append(0);
//    }
//  else if (m_voxelType == _UShort ||
//      m_voxelType == _Short)
//    {
//      if (m_voxelType == _UShort) rMin = 0;
//      if (m_voxelType == _Short) rMin = -32767;
//      rSize = 65536;
//      for(uint i=0; i<65536; i++)
//	m_histogram.append(0);
//    }
//  else
//    {
//      QMessageBox::information(0, "Error", "Why am i here ???");
//      return;
//    }
//
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  m_rawMin = 10000000;
//  m_rawMax = -10000000;
//
//  NcError err(NcError::verbose_nonfatal);
//  for(uint nf=0; nf<m_fileName.size(); nf++)
//    {
//      progress.setLabelText(m_fileName[nf]);
//
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//      
//      int iEnd = ncvar->get_dim(0)->size();
//      for(uint i=0; i<iEnd; i++)
//	{
//	  progress.setValue((int)(100.0*(float)i/(float)iEnd));
//	  qApp->processEvents();
//
//	  ncvar->set_cur(i, 0, 0);
//	  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	    ncvar->get((ncbyte*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncShort)
//	    ncvar->get((short*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncInt)
//	    ncvar->get((int*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncFloat)
//	    ncvar->get((float*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncDouble)
//	    ncvar->get((double*)tmp, 1, m_width, m_height);
//	  
//	  
//	  if (m_voxelType == _UChar)
//	    {
//	      uchar *ptr = tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Char)
//	    {
//	      char *ptr = (char*) tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	  if (m_voxelType == _UShort)
//	    {
//	      ushort *ptr = (ushort*) tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Short)
//	    {
//	      short *ptr = (short*) tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Int)
//	    {
//	      int *ptr = (int*) tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Float)
//	    {
//	      float *ptr = (float*) tmp;
//	      MINMAXANDHISTOGRAM();
//	    }
//	}
//
//      dataFile.close();
//    }
//
//  delete [] tmp;
//
//  while(m_histogram.last() == 0)
//    m_histogram.removeLast();
//  while(m_histogram.first() == 0)
//    m_histogram.removeFirst();
//
//  progress.setValue(100);
//  qApp->processEvents();
//}
//
//
//#define FINDMINMAX()					\
//  {							\
//    for(uint j=0; j<nY*nZ; j++)				\
//      {							\
//	float val = ptr[j];				\
//	m_rawMin = qMin(m_rawMin, val);			\
//	m_rawMax = qMax(m_rawMax, val);			\
//      }							\
//  }
//
//void
//RemapNcVolume::findMinMax()
//{
//  QProgressDialog progress("Finding Min and Max",
//			   "Cancel",
//			   0, 100,
//			   0);
//  progress.setMinimumDuration(0);
//
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  m_rawMin = 10000000;
//  m_rawMax = -10000000;
//
//  NcError err(NcError::verbose_nonfatal);
//  for(uint nf=0; nf<m_fileName.size(); nf++)
//    {
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//
//      int iEnd = ncvar->get_dim(0)->size();
//      for(uint i=0; i<iEnd; i++)
//	{
//	  progress.setValue((int)(100.0*(float)i/(float)iEnd));
//	  qApp->processEvents();
//	  
//	  ncvar->set_cur(i, 0, 0);
//	  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	    ncvar->get((ncbyte*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncShort)
//	    ncvar->get((short*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncInt)
//	    ncvar->get((int*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncFloat)
//	    ncvar->get((float*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncDouble)
//	    ncvar->get((double*)tmp, 1, m_width, m_height);
//
//	  if (m_voxelType == _UChar)
//	    {
//	      uchar *ptr = tmp;
//	      FINDMINMAX();
//	    }
//	  else if (m_voxelType == _Char)
//	    {
//	      char *ptr = (char*) tmp;
//	      FINDMINMAX();
//	}
//	  if (m_voxelType == _UShort)
//	    {
//	      ushort *ptr = (ushort*) tmp;
//	      FINDMINMAX();
//	    }
//	  else if (m_voxelType == _Short)
//	    {
//	      short *ptr = (short*) tmp;
//	      FINDMINMAX();
//	    }
//	  else if (m_voxelType == _Int)
//	    {
//	      int *ptr = (int*) tmp;
//	      FINDMINMAX();
//	    }
//	  else if (m_voxelType == _Float)
//	    {
//	      float *ptr = (float*) tmp;
//	      FINDMINMAX();
//	    }
//	}
//      dataFile.close();
//    }
//
//  delete [] tmp;
//
//  progress.setValue(100);
//  qApp->processEvents();
//}
//
//#define GENHISTOGRAM()					\
//  {							\
//    for(uint j=0; j<nY*nZ; j++)				\
//      {							\
//	float fidx = (ptr[j]-m_rawMin)/rSize;		\
//	fidx = qBound(0.0f, fidx, 1.0f);		\
//	int idx = fidx*histogramSize;			\
//	m_histogram[idx]+=1;				\
//      }							\
//  }
//
//void
//RemapNcVolume::generateHistogram()
//{
//  QProgressDialog progress("Generating Histogram",
//			   "Cancel",
//			   0, 100,
//			   0);
//  progress.setMinimumDuration(0);
//
//  float rSize = m_rawMax-m_rawMin;
//  m_histogram.clear();
//  if (m_voxelType == _UChar ||
//      m_voxelType == _Char ||
//      m_voxelType == _UShort ||
//      m_voxelType == _Short)
//    {
//      for(uint i=0; i<rSize+1; i++)
//	m_histogram.append(0);
//    }
//  else
//    {      
//      for(uint i=0; i<65536; i++)
//	m_histogram.append(0);
//    }
//
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  NcError err(NcError::verbose_nonfatal);
//  int histogramSize = m_histogram.size()-1;
//
//  for(uint nf=0; nf<m_fileName.size(); nf++)
//    {
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//      
//      int iEnd = ncvar->get_dim(0)->size();
//      for(uint i=0; i<iEnd; i++)
//	{
//	  progress.setValue((int)(100.0*(float)i/(float)iEnd));
//	  qApp->processEvents();
//
//	  ncvar->set_cur(i, 0, 0);
//	  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	    ncvar->get((ncbyte*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncShort)
//	    ncvar->get((short*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncInt)
//	    ncvar->get((int*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncFloat)
//	    ncvar->get((float*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncDouble)
//	    ncvar->get((double*)tmp, 1, m_width, m_height);
//	  
//	  
//	  if (m_voxelType == _UChar)
//	    {
//	      uchar *ptr = tmp;
//	      GENHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Char)
//	    {
//	      char *ptr = (char*) tmp;
//	      GENHISTOGRAM();
//	    }
//	  if (m_voxelType == _UShort)
//	    {
//	      ushort *ptr = (ushort*) tmp;
//	      GENHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Short)
//	    {
//	      short *ptr = (short*) tmp;
//	      GENHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Int)
//	    {
//	      int *ptr = (int*) tmp;
//	      GENHISTOGRAM();
//	    }
//	  else if (m_voxelType == _Float)
//	    {
//	      float *ptr = (float*) tmp;
//	      GENHISTOGRAM();
//	    }
//	}
//      dataFile.close();
//    }
//
//  delete [] tmp;
//
//  while(m_histogram.last() == 0)
//    m_histogram.removeLast();
//  while(m_histogram.first() == 0)
//    m_histogram.removeFirst();
//
////  QMessageBox::information(0, "",  QString("%1 %2 : %3").\
////			   arg(m_rawMin).arg(m_rawMax).arg(rSize));
//
//  progress.setValue(100);
//  qApp->processEvents();
//}
//
//
//void
//RemapNcVolume::getDepthSlice(int slc,
//			     uchar* slice)
//{
//  NcError err(NcError::verbose_nonfatal);
//
//  int nf = 0;
//  int slcno = slc;
//  for(uint fl=0; fl<m_fileName.size(); fl++)
//    {
//      if (m_depthList[fl] > slc)
//	{
//	  nf = fl;
//	  if (fl == 0)
//	    slcno = slc;
//	  else
//	    slcno = slc-m_depthList[fl-1];
//	  break;
//	}
//    }
//
//  NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		  NcFile::ReadOnly);
//  NcVar *ncvar;
//  ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//  ncvar->set_cur(slcno, 0, 0);
//  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//    ncvar->get((ncbyte*)slice, 1, m_width, m_height);
//  else if (ncvar->type() == ncShort)
//    ncvar->get((short*)slice, 1, m_width, m_height);
//  else if (ncvar->type() == ncInt)
//    ncvar->get((int*)slice, 1, m_width, m_height);
//  else if (ncvar->type() == ncFloat)
//    ncvar->get((float*)slice, 1, m_width, m_height);
//  else if (ncvar->type() == ncDouble)
//    ncvar->get((double*)slice, 1, m_width, m_height);
//  dataFile.close();
//}
//
//QImage
//RemapNcVolume::getDepthSliceImage(int slc)
//{
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  if (m_image)
//    delete [] m_image;
//
//  m_image = new uchar[nY*nZ];
//
//
//  NcError err(NcError::verbose_nonfatal);
//
//  int nf = 0;
//  int slcno = slc;
//  for(uint fl=0; fl<m_fileName.size(); fl++)
//    {
//      if (m_depthList[fl] > slc)
//	{
//	  nf = fl;
//	  if (fl == 0)
//	    slcno = slc;
//	  else
//	    slcno = slc-m_depthList[fl-1];
//	  break;
//	}
//    }
//
//  NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		  NcFile::ReadOnly);
//  NcVar *ncvar;
//  ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//  ncvar->set_cur(slcno, 0, 0);
//  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//    ncvar->get((ncbyte*)tmp, 1, m_width, m_height);
//  else if (ncvar->type() == ncShort)
//    ncvar->get((short*)tmp, 1, m_width, m_height);
//  else if (ncvar->type() == ncInt)
//    ncvar->get((int*)tmp, 1, m_width, m_height);
//  else if (ncvar->type() == ncFloat)
//    ncvar->get((float*)tmp, 1, m_width, m_height);
//  else if (ncvar->type() == ncDouble)
//    ncvar->get((double*)tmp, 1, m_width, m_height);
//  dataFile.close();
//
//  int rawSize = m_rawMap.size()-1;
//  for(uint i=0; i<nY*nZ; i++)
//    {
//      int idx = m_rawMap.size()-1;
//      float frc = 0;
//      float v;
//
//      if (m_voxelType == _UChar)
//	v = ((uchar *)tmp)[i];
//      else if (m_voxelType == _Char)
//	v = ((char *)tmp)[i];
//      else if (m_voxelType == _UShort)
//	v = ((ushort *)tmp)[i];
//      else if (m_voxelType == _Short)
//	v = ((short *)tmp)[i];
//      else if (m_voxelType == _Int)
//	v = ((int *)tmp)[i];
//      else if (m_voxelType == _Float)
//	v = ((float *)tmp)[i];
//
//      if (v < m_rawMap[0])
//	{
//	  idx = 0;
//	  frc = 0;
//	}
//      else if (v > m_rawMap[rawSize])
//	{
//	  idx = rawSize-1;
//	  frc = 1;
//	}
//      else
//	{
//	  for(uint m=0; m<rawSize; m++)
//	    {
//	      if (v >= m_rawMap[m] &&
//		  v <= m_rawMap[m+1])
//		{
//		  idx = m;
//		  frc = ((float)v-(float)m_rawMap[m])/
//		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
//		}
//	    }
//	}
//
//      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
//      m_image[i] = pv;
//    }
//  QImage img = QImage(m_image, nZ, nY, nZ, QImage::Format_Indexed8);
//
//  delete [] tmp;
//
//  return img;
//}
//
//QImage
//RemapNcVolume::getWidthSliceImage(int slc)
//{
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  if (slc < 0 || slc >= nY)
//    {
//      QImage img = QImage(100, 100, QImage::Format_Indexed8);
//      return img;
//    }
//
//  if (m_image)
//    delete [] m_image;
//  m_image = new uchar[nX*nZ];
//
//  int nbytes = nX*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  NcError err(NcError::verbose_nonfatal);
//
//  for(uint nf=0; nf<m_fileName.size(); nf++)
//    {
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//      ncvar->set_cur(0, slc, 0);
//
//      int depth;
//      uchar *ptmp;
//      if (nf > 0)
//	{
//	  depth = m_depthList[nf]-m_depthList[nf-1];
//	  ptmp = tmp + m_depthList[nf-1]*nZ*m_bytesPerVoxel;
//	}
//      else
//	{
//	  depth = m_depthList[0];
//	  ptmp = tmp;
//	}
//
//
//      if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	ncvar->get((ncbyte*)ptmp, depth, 1, m_height);
//      else if (ncvar->type() == ncShort)
//	ncvar->get((short*)ptmp, depth, 1, m_height);
//      else if (ncvar->type() == ncInt)
//	ncvar->get((int*)ptmp, depth, 1, m_height);
//      else if (ncvar->type() == ncFloat)
//	ncvar->get((float*)ptmp, depth, 1, m_height);
//      else if (ncvar->type() == ncDouble)
//	ncvar->get((double*)ptmp, depth, 1, m_height);
//      dataFile.close();
//    }
//  
//  int rawSize = m_rawMap.size()-1;
//  for(uint i=0; i<nX*nZ; i++)
//    {
//      int idx = m_rawMap.size()-1;
//      float frc = 0;
//      float v;
//      
//      if (m_voxelType == _UChar)
//	v = ((uchar *)tmp)[i];
//      else if (m_voxelType == _Char)
//	v = ((char *)tmp)[i];
//      else if (m_voxelType == _UShort)
//	v = ((ushort *)tmp)[i];
//      else if (m_voxelType == _Short)
//	v = ((short *)tmp)[i];
//      else if (m_voxelType == _Int)
//	v = ((int *)tmp)[i];
//      else if (m_voxelType == _Float)
//	v = ((float *)tmp)[i];
//      
//      if (v < m_rawMap[0])
//	{
//	  idx = 0;
//	  frc = 0;
//	}
//      else if (v > m_rawMap[rawSize])
//	{
//	  idx = rawSize-1;
//	  frc = 1;
//	}
//      else
//	{
//	  for(uint m=0; m<rawSize; m++)
//	    {
//	      if (v >= m_rawMap[m] &&
//		  v <= m_rawMap[m+1])
//		{
//		  idx = m;
//		  frc = ((float)v-(float)m_rawMap[m])/
//		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
//		}
//	    }
//	}
//      
//      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
//      m_image[i] = pv;
//    }
//  
//  QImage img = QImage(m_image, nZ, nX, nZ, QImage::Format_Indexed8);
//
//  delete [] tmp;
//
//  return img;
//}
//
//QImage
//RemapNcVolume::getHeightSliceImage(int slc)
//{
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  if (slc < 0 || slc >= nZ)
//    {
//      QImage img = QImage(100, 100, QImage::Format_Indexed8);
//      return img;
//    }
//
//  if (m_image)
//    delete [] m_image;
//  m_image = new uchar[nX*nY];
//
//  int nbytes = nX*nY*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  NcError err(NcError::verbose_nonfatal);
//
//  for(uint nf=0; nf < m_fileName.size(); nf++)
//    {
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//      ncvar->set_cur(0, 0, slc);
//      
//      int depth;
//      uchar *ptmp;
//      if (nf > 0)
//	{
//	  depth = m_depthList[nf]-m_depthList[nf-1];
//	  ptmp = tmp + m_depthList[nf-1]*nY*m_bytesPerVoxel;
//	}
//      else
//	{
//	  depth = m_depthList[0];
//	  ptmp = tmp;
//	}
//
//      if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	ncvar->get((ncbyte*)ptmp, depth, m_width, 1);
//      else if (ncvar->type() == ncShort)
//	ncvar->get((short*)ptmp, depth, m_width, 1);
//      else if (ncvar->type() == ncInt)
//	ncvar->get((int*)ptmp, depth, m_width, 1);
//      else if (ncvar->type() == ncFloat)
//	ncvar->get((float*)ptmp, depth, m_width, 1);
//      else if (ncvar->type() == ncDouble)
//	ncvar->get((double*)ptmp, depth, m_width, 1);
//      dataFile.close();
//    }
//
//  int rawSize = m_rawMap.size()-1;
//  for(uint i=0; i<nX*nY; i++)
//    {
//      int idx = m_rawMap.size()-1;
//      float frc = 0;
//      float v;
//
//      if (m_voxelType == _UChar)
//	v = ((uchar *)tmp)[i];
//      else if (m_voxelType == _Char)
//	v = ((char *)tmp)[i];
//      else if (m_voxelType == _UShort)
//	v = ((ushort *)tmp)[i];
//      else if (m_voxelType == _Short)
//	v = ((short *)tmp)[i];
//      else if (m_voxelType == _Int)
//	v = ((int *)tmp)[i];
//      else if (m_voxelType == _Float)
//	v = ((float *)tmp)[i];
//
//      if (v < m_rawMap[0])
//	{
//	  idx = 0;
//	  frc = 0;
//	}
//      else if (v > m_rawMap[rawSize])
//	{
//	  idx = rawSize-1;
//	  frc = 1;
//	}
//      else
//	{
//	  for(uint m=0; m<rawSize; m++)
//	    {
//	      if (v >= m_rawMap[m] &&
//		  v <= m_rawMap[m+1])
//		{
//		  idx = m;
//		  frc = ((float)v-(float)m_rawMap[m])/
//		    ((float)m_rawMap[m+1]-(float)m_rawMap[m]);
//		}
//	    }
//	}
//
//      uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
//      m_image[i] = pv;
//    }
//  QImage img = QImage(m_image, nY, nX, nY, QImage::Format_Indexed8);
//
//  delete [] tmp;
//
//  return img;
//}
//
//QPair<QVariant, QVariant>
//RemapNcVolume::rawValue(int d, int w, int h)
//{
//  QPair<QVariant, QVariant> pair;
//
//  if (d < 0 || d >= m_depth ||
//      w < 0 || w >= m_width ||
//      h < 0 || h >= m_height)
//    {
//      pair.first = QVariant("OutOfBounds");
//      pair.second = QVariant("OutOfBounds");
//      return pair;
//    }
//
//
//  //------ cater for multiple netCDF files ------
//  int nf = 0;
//  int slcno = d;
//  for(uint fl=0; fl<m_fileName.size(); fl++)
//    {
//      if (m_depthList[fl] > d)
//	{
//	  nf = fl;
//	  if (fl == 0)
//	    slcno = d;
//	  else
//	    slcno = d-m_depthList[fl-1];
//	  break;
//	}
//    }
//  //----------------------------------------
//
//
//  NcError err(NcError::verbose_nonfatal);
//  NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		  NcFile::ReadOnly);
//  NcVar *ncvar;
//  ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//  ncvar->set_cur(slcno, w, h);
//
//  QVariant v;
//
//  if (m_voxelType == _UChar)
//    {
//      unsigned char a;
//      ncvar->get((ncbyte*)&a, 1, 1, 1);
//      v = QVariant((uint)a);
//    }
//  else if (m_voxelType == _Char)
//    {
//      char a;
//      ncvar->get((ncbyte*)&a, 1, 1, 1);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _UShort)
//    {
//      unsigned short a;
//      ncvar->get((short*)&a, 1, 1, 1);
//      v = QVariant((uint)a);
//    }
//  else if (m_voxelType == _Short)
//    {
//      short a;
//      ncvar->get((short*)&a, 1, 1, 1);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _Int)
//    {
//      int a;
//      ncvar->get((int*)&a, 1, 1, 1);
//      v = QVariant((int)a);
//    }
//  else if (m_voxelType == _Float)
//    {
//      float a;
//      ncvar->get((float*)&a, 1, 1, 1);
//      v = QVariant((double)a);
//    }
//  dataFile.close();
// 
//
//  int rawSize = m_rawMap.size()-1;
//  int idx = rawSize;
//  float frc = 0;
//  float val;
//
//  if (v.type() == QVariant::UInt)
//    val = v.toUInt();
//  else if (v.type() == QVariant::Int)
//    val = v.toInt();
//  else if (v.type() == QVariant::Double)
//    val = v.toDouble();
//
//  if (val <= m_rawMap[0])
//    {
//      idx = 0;
//      frc = 0;
//    }
//  else if (val >= m_rawMap[rawSize])
//    {
//      idx = rawSize-1;
//      frc = 1;
//    }
//  else
//    {
//      for(uint m=0; m<rawSize; m++)
//	{
//	  if (val >= m_rawMap[m] &&
//	      val <= m_rawMap[m+1])
//	    {
//	      idx = m;
//	      frc = ((float)val-(float)m_rawMap[m])/
//		((float)m_rawMap[m+1]-(float)m_rawMap[m]);
//	    }
//	}
//    }
//  
//  uchar pv = m_pvlMap[idx] + frc*(m_pvlMap[idx+1]-m_pvlMap[idx]);
//
//  pair.first = v;
//  pair.second = QVariant((uint)pv);
//  return pair;
//}
//
//void
//RemapNcVolume::saveTrimmed(QString trimFile,
//			   int dmin, int dmax,
//			   int wmin, int wmax,
//			   int hmin, int hmax)
//{
//  QProgressDialog progress("Saving trimmed volume",
//			   "Cancel",
//			   0, 100,
//			   0);
//  progress.setMinimumDuration(0);
//
//  int nX, nY, nZ;
//  nX = m_depth;
//  nY = m_width;
//  nZ = m_height;
//
//  int mX, mY, mZ;
//  mX = dmax-dmin+1;
//  mY = wmax-wmin+1;
//  mZ = hmax-hmin+1;
//
//  int nbytes = nY*nZ*m_bytesPerVoxel;
//  uchar *tmp = new uchar[nbytes];
//
//  uchar vt;
//  if (m_voxelType == _UChar) vt = 0; // unsigned byte
//  if (m_voxelType == _Char) vt = 1; // signed byte
//  if (m_voxelType == _UShort) vt = 2; // unsigned short
//  if (m_voxelType == _Short) vt = 3; // signed short
//  if (m_voxelType == _Int) vt = 4; // int
//  if (m_voxelType == _Float) vt = 8; // float
//  
//  QFile fout(trimFile);
//  fout.open(QFile::WriteOnly);
//
//  fout.write((char*)&vt, 1);
//  fout.write((char*)&mX, 4);
//  fout.write((char*)&mY, 4);
//  fout.write((char*)&mZ, 4);
//
//
//  NcError err(NcError::verbose_nonfatal);
//
//  int nfStart, nfEnd;
//  int slcStart, slcEnd;
//  //------ cater for multiple netCDF files ------
//  for(uint fl=0; fl<m_fileName.size(); fl++)
//    {
//      if (m_depthList[fl] > dmin)
//	{
//	  nfStart = fl;
//	  if (fl == 0)
//	    slcStart = dmin;
//	  else
//	    slcStart = dmin-m_depthList[fl-1];
//	  break;
//	}
//    }
//  for(uint fl=0; fl<m_fileName.size(); fl++)
//    {
//      if (m_depthList[fl] > dmax)
//	{
//	  nfEnd = fl;
//	  if (fl == 0)
//	    slcEnd = dmax;
//	  else
//	    slcEnd = dmax-m_depthList[fl-1];
//	  break;
//	}
//    }
//  //----------------------------------------
//
//  uint nslc = 0;
//  for(uint nf=nfStart; nf<=nfEnd; nf++)
//    {
//      NcFile dataFile((char *)m_fileName[nf].toAscii().data(),
//		      NcFile::ReadOnly);
//      NcVar *ncvar;
//      ncvar = dataFile.get_var((char *)m_varName.toAscii().data());
//
//      uint dStart, dEnd;
//      dStart = 0;
//      dEnd = ncvar->get_dim(0)->size()-1;
//
//      if (nf == nfStart) dStart = slcStart;
//      if (nf == nfEnd) dEnd = slcEnd;
//
//      for(uint i=dStart; i<=dEnd; i++)
//	{
//	  ncvar->set_cur(i, 0, 0);
//	  if (ncvar->type() == ncByte || ncvar->type() == ncChar)
//	    ncvar->get((ncbyte*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncShort)
//	    ncvar->get((short*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncInt)
//	    ncvar->get((int*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncFloat)
//	    ncvar->get((float*)tmp, 1, m_width, m_height);
//	  else if (ncvar->type() == ncDouble)
//	    ncvar->get((double*)tmp, 1, m_width, m_height);
//	  
//	  for(uint j=wmin; j<=wmax; j++)
//	    {
//	      memcpy(tmp+(j-wmin)*mZ*m_bytesPerVoxel,
//		     tmp+(j*nZ + hmin)*m_bytesPerVoxel,
//		     mZ*m_bytesPerVoxel);
//	    }
//	  fout.write((char*)tmp, mY*mZ*m_bytesPerVoxel);
//	  progress.setValue((int)(100*(float)nslc/(float)mX));
//	  qApp->processEvents();
//	  nslc++;
//	}
//      dataFile.close();
//    }
//
//  fout.close();  
//
//  delete [] tmp;
//
//  m_headerBytes = 13; // to be used in applyMapping
//}

//------------------------------------------------
// MarchingCubes
//------------------------------------------------
//
// MarchingCubes Command Line interface
// Version 0.2 - 12/08/2002
//
// Thomas Lewiner thomas.lewiner@polytechnique.org
// Math Dept, PUC-Rio
//
//------------------------------------------------
// modified to use Qt
// data type changed from real to uchar
// by Ajay Limaye (limaye.ajay@gmail.com)
// Vizlab,
// Australian National University
//------------------------------------------------


#include <QApplication>
#include <QtGui>

#include <stdio.h>
#include "marchingcubes.h"
#include "volumefilemanager.h"
#include "ply.h"

QString settingsFilename = ".meshgen";

void
saveSettings(float memGb,
	     QString prevDir)
{
  QString homePath = QDir::homePath();
  QFileInfo settingsFile(homePath, settingsFilename);

  QFile fin(settingsFile.absoluteFilePath());
  if (fin.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream out(&fin);
      out << "use memory :: " << memGb << "\n";
      out << "previous dir :: " << prevDir << "\n";
    }
}

bool
loadSettings(float &memGb,
	     QString &prevDir)
{
  QString homePath = QDir::homePath();
  QFileInfo settingsFile(homePath, settingsFilename);

  memGb = 0.5;
  prevDir = qApp->applicationDirPath();

  bool ok = false;
  if (settingsFile.exists())
    {
      QFile fin(settingsFile.absoluteFilePath());
      if (fin.open(QIODevice::ReadOnly | QIODevice::Text))
	{
	  QTextStream in(&fin);
	  if (!in.atEnd())
	    {
	      QString line = in.readLine();
	      QStringList words = line.split("::");
	      memGb = words[1].toFloat();

	      if (!in.atEnd())
		{
		  line = in.readLine();
		  words = line.split("::");
		  prevDir = words[1].trimmed();		  
		}

	      ok = true;
	    }
	}
    }

  return ok;
}

void
getSettings(float &memGb)
{
  memGb = QInputDialog::getDouble(0, "Use memory", "Max memory we can use (GB)", memGb, 0.1, 1000, 2);
}





//-------------------------------------
VolumeFileManager vfm;
QStringList filenames;
int nX, nY, nZ;
QVector<int> nSlices;

void calculateGridSize()
{
  QFile fin(filenames[0]);
  fin.open(QFile::ReadOnly);
  uchar vtype;
  fin.read((char*)&vtype, 1);
  fin.read((char*)&nX, 4);
  fin.read((char*)&nY, 4);
  fin.read((char*)&nZ, 4);
  fin.close();

  //QMessageBox::information(0, "", QString("%1 %2 %3").arg(nX).arg(nY).arg(nZ));

  nSlices[0] = nX;

  if (filenames.count() > 1)
    {
      int nslices = nX;

      for (int nf=1; nf<filenames.count(); nf++)
	{
	  uchar nvt;
	  int nfX, nfY, nfZ;
	  QFile fd(filenames[nf]);
	  fd.open(QFile::ReadOnly);
	  fd.read((char*)&nvt, sizeof(unsigned char));
	  fd.read((char*)&nfX, sizeof(int));
	  fd.read((char*)&nfY, sizeof(int));
	  fd.read((char*)&nfZ, sizeof(int));
	  fd.close();
	  //QMessageBox::information(0, "", QString("%1 %2 %3").arg(nfX).arg(nfY).arg(nfZ));
	  if (nvt != vtype || nfY != nY || nfZ != nZ)
	    {
	      QMessageBox::information(0, "", "Raw File format does not match");
	      exit(0);
	    }
	    
	  nslices += nfX;

	  nSlices[nf] = nSlices[nf-1] + nfX;
	}
      nX = nslices;
    }
  QMessageBox::information(0, "", QString("Grid size : %1 %2 %3").arg(nX).arg(nY).arg(nZ));
}


void saveMesh(QString flnm,
	      int nSlabs,
	      int nvertices, int ntriangles,
	      bool bin)
{
  QProgressDialog progress("Saving Mesh",
			   "Cancel",
			   0, 100,
			   0);
  progress.setMinimumDuration(0);

  typedef struct PlyFace
  {
    unsigned char nverts;    /* number of Vertex indices in list */
    int *verts;              /* Vertex index list */
  } PlyFace;


  PlyProperty vert_props[]  = { /* list of property information for a PlyVertex */
    {"x", Float32, Float32, offsetof( Vertex,x ), 0, 0, 0, 0},
    {"y", Float32, Float32, offsetof( Vertex,y ), 0, 0, 0, 0},
    {"z", Float32, Float32, offsetof( Vertex,z ), 0, 0, 0, 0},
    {"nx", Float32, Float32, offsetof( Vertex,nx ), 0, 0, 0, 0},
    {"ny", Float32, Float32, offsetof( Vertex,ny ), 0, 0, 0, 0},
    {"nz", Float32, Float32, offsetof( Vertex,nz ), 0, 0, 0, 0}
  };

  PlyProperty face_props[]  = { /* list of property information for a PlyFace */
    {"vertex_indices", Int32, Int32, offsetof( PlyFace,verts ),
      1, Uint8, Uint8, offsetof( PlyFace,nverts )},
  };


  PlyFile    *ply;
  FILE       *fp = fopen(flnm.toAscii().data(),
			 bin ? "wb" : "w");

  PlyFace     face ;
  int         verts[3] ;
  char       *elem_names[]  = { "vertex", "face" };
  ply = write_ply (fp,
		   2,
		   elem_names,
		   bin? PLY_BINARY_LE : PLY_ASCII );

  /* describe what properties go into the PlyVertex elements */
  describe_element_ply ( ply, "vertex", nvertices );
  describe_property_ply ( ply, &vert_props[0] );
  describe_property_ply ( ply, &vert_props[1] );
  describe_property_ply ( ply, &vert_props[2] );
  describe_property_ply ( ply, &vert_props[3] );
  describe_property_ply ( ply, &vert_props[4] );
  describe_property_ply ( ply, &vert_props[5] );

  /* describe PlyFace properties (just list of PlyVertex indices) */
  describe_element_ply ( ply, "face", ntriangles );
  describe_property_ply ( ply, &face_props[0] );

  header_complete_ply ( ply );


  /* set up and write the PlyVertex elements */
  put_element_setup_ply ( ply, "vertex" );
  for (int nb=0; nb<nSlabs; nb++)
    {
      progress.setValue((int)(100.0*(float)nb/(float)nSlabs));
      qApp->processEvents();

      int nverts;
      QString mflnm = flnm + QString(".%1.vert").arg(nb);

      QFile fin(mflnm);
      fin.open(QFile::ReadOnly);
      fin.read((char*)&nverts, 4);
      for(int ni=0; ni<nverts; ni++)
	{
	  Vertex vertex;
	  float v[6];
	  fin.read((char*)v, 24);
	  vertex.x = v[0];
	  vertex.y = v[1];
	  vertex.z = v[2];
	  vertex.nx = v[3];
	  vertex.ny = v[4];
	  vertex.nz = v[5];

	  put_element_ply ( ply, ( void * ) &vertex );
	}
      fin.close();
      fin.remove();
    }

  /* set up and write the PlyFace elements */
  put_element_setup_ply ( ply, "face" );
  face.nverts = 3 ;
  face.verts  = verts ;
  for (int nb=0; nb<nSlabs; nb++)
    {
      progress.setValue((int)(100.0*(float)nb/(float)nSlabs));
      qApp->processEvents();

      int ntrigs;
      QString mflnm = flnm + QString(".%1.tri").arg(nb);

      QFile fin(mflnm);
      fin.open(QFile::ReadOnly);
      fin.read((char*)&ntrigs, 4);      
      for(int ni=0; ni<ntrigs; ni++)
	{
	  int v[3];
	  fin.read((char*)v, 12);
	  face.verts[0] = v[0];
	  face.verts[1] = v[1];
	  face.verts[2] = v[2];

	  put_element_ply ( ply, ( void * ) &face );
	}
      fin.close();
      fin.remove();
    }

  close_ply ( ply );
  free_ply ( ply );
  fclose( fp ) ;

  progress.setValue(100);
}


void generateMesh(int nSlabs,
		  int isoval,
		  QString flnm)
{
  bool saveIntermediate = false;
  if (nSlabs > 1)
    {
      QStringList sl;
      sl << "Yes";
      sl << "No";
      bool ok;
      QString okstr = QInputDialog::getItem(0, "Save slab files",
		      "Save slab files in .ply format.\nFiles will not be collated together to create a unified mesh for the whole sample.",
					    sl, 0, false, &ok);
      if (ok && okstr == "Yes")
	saveIntermediate = true;
    }

  int blockStep = nX/nSlabs;
  int ntriangles = 0;
  int nvertices = 0;
  for (int nb=0; nb<nSlabs; nb++)
    {
      int d0 = nb*blockStep;
      int d1 = qMin(nX-1, (nb+1)*blockStep);
      int dlen = d1-d0+1;
      
      uchar *extData;
      extData = new uchar[dlen*nY*nZ];

      for(int i=d0; i<=d1; i++)
	memcpy(extData + (i-d0)*nY*nZ, vfm.getSlice(i), nY*nZ);

      //----------------------------
      if (saveIntermediate)
	{
	  memset(extData, 0, nY*nZ);
	  memset(extData + (d1-d0)*nY*nZ, 0, nY*nZ);
	}
      //----------------------------

      MarchingCubes mc ;
      mc.set_resolution(nZ, nY, dlen ) ;      
      mc.set_ext_data(extData);     
      mc.init_all() ;
      mc.run(isoval) ;
//      mc.clean_temps() ;

      // save part .ply file
      if (saveIntermediate)
	{
	  QString plyfl = flnm;
	  plyfl.chop(4);
	  plyfl += QString(".%1.ply").arg(nb);
	  mc.writePLY(plyfl.toAscii().data(), true);
	}
      else
	{
	  {
	    QString mflnm = flnm + QString(".%1.tri").arg(nb);
	    int ntrigs = mc.ntrigs();
	    Triangle *triangles = mc.triangles();
	    QFile fout(mflnm);
	    fout.open(QFile::WriteOnly);
	    fout.write((char*)&ntrigs, 4);
	    for(int ni=0; ni<ntrigs; ni++)
	      {
		int v[3];
		v[2] = triangles[ni].v1 + nvertices;
		v[1] = triangles[ni].v2 + nvertices;
		v[0] = triangles[ni].v3 + nvertices;
		fout.write((char*)v, 12);
	      }
	    fout.close();
	    ntriangles += ntrigs;
	  }
	  
	  {
	    QString mflnm = flnm + QString(".%1.vert").arg(nb);
	    int nverts = mc.nverts();
	    Vertex *vertices = mc.vertices();
	    QFile fout(mflnm);
	    fout.open(QFile::WriteOnly);
	    fout.write((char*)&nverts, 4);
	    for(int ni=0; ni<nverts; ni++)
	      {
		float v[6];
		v[0] = vertices[ni].x;
		v[1] = vertices[ni].y;
		v[2] = vertices[ni].z + d0;
		v[3] = vertices[ni].nx;
		v[4] = vertices[ni].ny;
		v[5] = vertices[ni].nz;
		fout.write((char*)v, 24);
	      }
	    fout.close();
	    nvertices += nverts;
	  }
	}

      mc.clean_all();
      delete [] extData;
    }

  // Files are not collated together to create
  // a unified mesh for the whole sample
  if (!saveIntermediate)
    saveMesh(flnm,
	     nSlabs,
	     nvertices, ntriangles,
	     true);
}
//-------------------------------------


//_____________________________________________________________________________
// main function
int main (int argc, char **argv)
//-----------------------------------------------------------------------------
{
  QApplication application(argc,argv);

  QString homePath = QDir::homePath();
  QFileInfo settingsFile(homePath, ".meshgenerator");

  float memGb = 0.5;
  QString prevDir = qApp->applicationDirPath();

  bool ok = loadSettings(memGb, prevDir);
  getSettings(memGb);
  saveSettings(memGb, prevDir);



  qint64 gb = 1024*1024*1024;
  qint64 memsize = memGb*gb; // max memory we can use (in GB)

  qint64 canhandle = memsize/15;
  qint64 gsize = qPow((double)canhandle, 0.333);

  QMessageBox::information(0, "",
	 QString("Can handle data with total grid size of %1 : typically %2^3\nOtherwise slabs method will be used.  Mesh is generated for each slab and then joined together.").\
		arg(canhandle).arg(gsize));

  
  //---- get data filename -----
  QStringList flnms;
  flnms = QFileDialog::getOpenFileNames(0,
					"Open data set for meshing",
					prevDir,
					"*.*");
  if (flnms.size() == 0)
    exit(0);
  //----------------------------


  //---- save data directory for exporting grid ---
  QFileInfo f(flnms[0]);
  QString datadir = f.absolutePath();
  //----------------------------


  //---- get isosurface value ---
  int isoval = QInputDialog::getInt(0, "Isovalue", "Value", 128, 0, 255);
  //----------------------------

  //---- read data ---
  filenames = flnms;
  calculateGridSize();
  vfm.setFilenameList(filenames);
  vfm.setBaseFilename(filenames[0]);
  vfm.setDepth(nX);
  vfm.setWidth(nY);
  vfm.setHeight(nZ);
  vfm.setSlabSize(nSlices[0]);
  vfm.setHeaderSize(13);
  //----------------------------


  //---- export the grid ---
  QString flnm = QFileDialog::getSaveFileName(0,
					      "Export mesh to file",
					      datadir,
					      "*.ply");
  if (flnm.size() == 0)
    exit(0);
  //----------------------------

  int nSlabs = 1;
  qint64 reqmem = nX;
  reqmem *= nY*nZ*20;
  nSlabs = qMax(qint64(1), reqmem/memsize + 1);
//  QMessageBox::information(0, "", QString("Number of Slabs : %1 : %2 %3").\
//			   arg(nSlabs).arg(reqmem).arg(memsize));

  generateMesh(nSlabs,
	       isoval,
	       flnm);


  //---- save the previous working directory ---
  {
    QFileInfo f(flnm);
    QString datadir = f.absolutePath();
    saveSettings(memGb, datadir);
  }
  //----------------------------

  qApp->closeAllWindows();
  qApp->quit();
  QMessageBox::information(0, "", "really done");

  return 0 ;
}
//_____________________________________________________________________________

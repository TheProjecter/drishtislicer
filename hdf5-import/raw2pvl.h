#ifndef RAW2PVL_H
#define RAW2PVL_H

#include <QtGui>

#include "volinterface.h"

//#include "remapanalyze.h"
//#include "remapdicomvolume.h"
//#include "remaphdf4.h"
//#include "remapimagevolume.h"
//#include "remapncvolume.h"
//#include "remaprawslices.h"
//#include "remaprawvolume.h"
//#include "remaptomvolume.h"

class Raw2Pvl
{
 public :

  enum VolumeFilters {
    _NoFilter = 0,
    _MeanFilter,
    _MedianFilter,
    _BilateralFilter
  };
  
  static void applyMapping(QString, QString,
			   int, int, int, int, int,
			   QList<float>,
			   QList<uchar>);

  static void applyRaw2PvlConversion(QString, QString,
				     int, int, int);

  static void applyFilter(uchar*, uchar*, uchar*,
			  uchar*, int, int, int,
			  int);
  
  static void applyVolumeFilter(QString, QString,
				int, int, int, int, int,
				int);

  static void subsampleVolume(QString, QString,
			      int, int,
			      int, int, int,
			      int, int);

  static void savePvl(VolInterface*,
		      int,
		      int, int,
		      int, int,
		      int, int,
		      QStringList);

  static void Old2New(QString, QString);

 private :
  static void createPvlNcFile(QString,
			      bool, QString,
			      int, int,
			      int, int, int,
			      float, float, float,
			      QList<float>, QList<uchar>,
			      QString,
			      int);

  static void savePvlHeader(QString,
			    bool, QString,
			    int, int,
			    int, int, int,
			    float, float, float,
			    QList<float>, QList<uchar>,
			    QString,
			    long, int, int);

  static void applyMeanFilter(uchar**, uchar*,
			      int,
			      int, int,
			      int);
  

  static void Old2NewScalar(QString, QString);
  static void Old2NewRGBA(QString, QString);
};

#endif

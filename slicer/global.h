#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGui>

class Global
{
 public :
  static QString previousDirectory();
  static void setPreviousDirectory(QString);

  static QString documentationPath();

  static QStatusBar *statusBar();
  static void setStatusBar(QStatusBar*);

 private :
  static QString m_previousDirectory;
  static QString m_documentationPath;
  static QStatusBar *m_statusBar;
};

#endif

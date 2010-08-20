/********************************************************************************
** Form generated from reading UI file 'drishtislicer.ui'
**
** Created: Thu 4. Feb 16:12:30 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DRISHTISLICER_H
#define UI_DRISHTISLICER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DrishtiSlicer
{
public:
    QAction *saveImage;
    QAction *actionSave_Images;
    QAction *actionExit;
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    QMenuBar *menubar;
    QMenu *menuFiles;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *DrishtiSlicer)
    {
        if (DrishtiSlicer->objectName().isEmpty())
            DrishtiSlicer->setObjectName(QString::fromUtf8("DrishtiSlicer"));
        DrishtiSlicer->resize(554, 550);
        saveImage = new QAction(DrishtiSlicer);
        saveImage->setObjectName(QString::fromUtf8("saveImage"));
        actionSave_Images = new QAction(DrishtiSlicer);
        actionSave_Images->setObjectName(QString::fromUtf8("actionSave_Images"));
        actionExit = new QAction(DrishtiSlicer);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        centralwidget = new QWidget(DrishtiSlicer);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        vboxLayout = new QVBoxLayout(centralwidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        DrishtiSlicer->setCentralWidget(centralwidget);
        menubar = new QMenuBar(DrishtiSlicer);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 554, 25));
        menuFiles = new QMenu(menubar);
        menuFiles->setObjectName(QString::fromUtf8("menuFiles"));
        DrishtiSlicer->setMenuBar(menubar);
        statusbar = new QStatusBar(DrishtiSlicer);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        DrishtiSlicer->setStatusBar(statusbar);

        menubar->addAction(menuFiles->menuAction());
        menuFiles->addAction(saveImage);
        menuFiles->addAction(actionSave_Images);
        menuFiles->addAction(actionExit);

        retranslateUi(DrishtiSlicer);

        QMetaObject::connectSlotsByName(DrishtiSlicer);
    } // setupUi

    void retranslateUi(QMainWindow *DrishtiSlicer)
    {
        DrishtiSlicer->setWindowTitle(QApplication::translate("DrishtiSlicer", "Drishti - Slicer", 0, QApplication::UnicodeUTF8));
        saveImage->setText(QApplication::translate("DrishtiSlicer", "Save Image (Alt+S)", 0, QApplication::UnicodeUTF8));
        actionSave_Images->setText(QApplication::translate("DrishtiSlicer", "Save Images", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("DrishtiSlicer", "Exit", 0, QApplication::UnicodeUTF8));
        menuFiles->setTitle(QApplication::translate("DrishtiSlicer", "Files", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DrishtiSlicer: public Ui_DrishtiSlicer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DRISHTISLICER_H

/********************************************************************************
** Form generated from reading ui file 'drishtiimport.ui'
**
** Created: Mon Jun 28 10:40:15 2010
**      by: Qt User Interface Compiler version 4.5.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_DRISHTIIMPORT_H
#define UI_DRISHTIIMPORT_H

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

class Ui_DrishtiImport
{
public:
    QAction *actionRAW;
    QAction *actionNetCDF;
    QAction *actionImage_directory;
    QAction *actionDicom_directory;
    QAction *actionSave_As;
    QAction *actionSave_Images;
    QAction *actionExit;
    QAction *actionImageMagick_directory;
    QAction *actionTOM;
    QAction *actionHelp;
    QAction *actionSave_RGB;
    QAction *actionRGB_directory;
    QAction *actionAnalyze;
    QAction *actionHDF4_directory;
    QAction *saveImage;
    QAction *saveLimits;
    QAction *loadLimits;
    QAction *actionRaw_slices;
    QAction *actionTimeSeries;
    QAction *actionConvert;
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    QMenuBar *menubar;
    QMenu *menuFiles;
    QMenu *menuLoad;
    QMenu *menuHelp;
    QMenu *menuConvert;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *DrishtiImport)
    {
        if (DrishtiImport->objectName().isEmpty())
            DrishtiImport->setObjectName(QString::fromUtf8("DrishtiImport"));
        DrishtiImport->resize(554, 550);
        actionRAW = new QAction(DrishtiImport);
        actionRAW->setObjectName(QString::fromUtf8("actionRAW"));
        actionNetCDF = new QAction(DrishtiImport);
        actionNetCDF->setObjectName(QString::fromUtf8("actionNetCDF"));
        actionImage_directory = new QAction(DrishtiImport);
        actionImage_directory->setObjectName(QString::fromUtf8("actionImage_directory"));
        actionDicom_directory = new QAction(DrishtiImport);
        actionDicom_directory->setObjectName(QString::fromUtf8("actionDicom_directory"));
        actionSave_As = new QAction(DrishtiImport);
        actionSave_As->setObjectName(QString::fromUtf8("actionSave_As"));
        actionSave_Images = new QAction(DrishtiImport);
        actionSave_Images->setObjectName(QString::fromUtf8("actionSave_Images"));
        actionExit = new QAction(DrishtiImport);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionImageMagick_directory = new QAction(DrishtiImport);
        actionImageMagick_directory->setObjectName(QString::fromUtf8("actionImageMagick_directory"));
        actionTOM = new QAction(DrishtiImport);
        actionTOM->setObjectName(QString::fromUtf8("actionTOM"));
        actionHelp = new QAction(DrishtiImport);
        actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
        actionSave_RGB = new QAction(DrishtiImport);
        actionSave_RGB->setObjectName(QString::fromUtf8("actionSave_RGB"));
        actionRGB_directory = new QAction(DrishtiImport);
        actionRGB_directory->setObjectName(QString::fromUtf8("actionRGB_directory"));
        actionAnalyze = new QAction(DrishtiImport);
        actionAnalyze->setObjectName(QString::fromUtf8("actionAnalyze"));
        actionHDF4_directory = new QAction(DrishtiImport);
        actionHDF4_directory->setObjectName(QString::fromUtf8("actionHDF4_directory"));
        saveImage = new QAction(DrishtiImport);
        saveImage->setObjectName(QString::fromUtf8("saveImage"));
        saveLimits = new QAction(DrishtiImport);
        saveLimits->setObjectName(QString::fromUtf8("saveLimits"));
        loadLimits = new QAction(DrishtiImport);
        loadLimits->setObjectName(QString::fromUtf8("loadLimits"));
        actionRaw_slices = new QAction(DrishtiImport);
        actionRaw_slices->setObjectName(QString::fromUtf8("actionRaw_slices"));
        actionTimeSeries = new QAction(DrishtiImport);
        actionTimeSeries->setObjectName(QString::fromUtf8("actionTimeSeries"));
        actionConvert = new QAction(DrishtiImport);
        actionConvert->setObjectName(QString::fromUtf8("actionConvert"));
        centralwidget = new QWidget(DrishtiImport);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        vboxLayout = new QVBoxLayout(centralwidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        DrishtiImport->setCentralWidget(centralwidget);
        menubar = new QMenuBar(DrishtiImport);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 554, 25));
        menuFiles = new QMenu(menubar);
        menuFiles->setObjectName(QString::fromUtf8("menuFiles"));
        menuLoad = new QMenu(menuFiles);
        menuLoad->setObjectName(QString::fromUtf8("menuLoad"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuConvert = new QMenu(menubar);
        menuConvert->setObjectName(QString::fromUtf8("menuConvert"));
        DrishtiImport->setMenuBar(menubar);
        statusbar = new QStatusBar(DrishtiImport);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        DrishtiImport->setStatusBar(statusbar);

        menubar->addAction(menuFiles->menuAction());
        menubar->addAction(menuConvert->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFiles->addAction(menuLoad->menuAction());
        menuFiles->addAction(actionTimeSeries);
        menuFiles->addSeparator();
        menuFiles->addAction(saveImage);
        menuFiles->addAction(actionSave_As);
        menuFiles->addAction(actionSave_Images);
        menuFiles->addSeparator();
        menuFiles->addAction(loadLimits);
        menuFiles->addAction(saveLimits);
        menuFiles->addSeparator();
        menuFiles->addAction(actionExit);
        menuLoad->addAction(actionRAW);
        menuLoad->addAction(actionNetCDF);
        menuLoad->addAction(actionTOM);
        menuLoad->addAction(actionAnalyze);
        menuLoad->addSeparator();
        menuLoad->addAction(actionImage_directory);
        menuLoad->addAction(actionRGB_directory);
        menuLoad->addAction(actionDicom_directory);
        menuLoad->addAction(actionImageMagick_directory);
        menuLoad->addAction(actionHDF4_directory);
        menuLoad->addAction(actionRaw_slices);
        menuHelp->addAction(actionHelp);
        menuConvert->addAction(actionConvert);

        retranslateUi(DrishtiImport);

        QMetaObject::connectSlotsByName(DrishtiImport);
    } // setupUi

    void retranslateUi(QMainWindow *DrishtiImport)
    {
        DrishtiImport->setWindowTitle(QApplication::translate("DrishtiImport", "Drishti - Import", 0, QApplication::UnicodeUTF8));
        actionRAW->setText(QApplication::translate("DrishtiImport", "raw", 0, QApplication::UnicodeUTF8));
        actionNetCDF->setText(QApplication::translate("DrishtiImport", "netCDF", 0, QApplication::UnicodeUTF8));
        actionImage_directory->setText(QApplication::translate("DrishtiImport", "standard image directory", 0, QApplication::UnicodeUTF8));
        actionDicom_directory->setText(QApplication::translate("DrishtiImport", "dicom image directory", 0, QApplication::UnicodeUTF8));
        actionSave_As->setText(QApplication::translate("DrishtiImport", "Save As (S)", 0, QApplication::UnicodeUTF8));
        actionSave_Images->setText(QApplication::translate("DrishtiImport", "Save Images", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("DrishtiImport", "Exit", 0, QApplication::UnicodeUTF8));
        actionImageMagick_directory->setText(QApplication::translate("DrishtiImport", "16/32 bit grayscale image directory", 0, QApplication::UnicodeUTF8));
        actionTOM->setText(QApplication::translate("DrishtiImport", "tom (QMUL)", 0, QApplication::UnicodeUTF8));
        actionHelp->setText(QApplication::translate("DrishtiImport", "Help", 0, QApplication::UnicodeUTF8));
        actionSave_RGB->setText(QApplication::translate("DrishtiImport", "Save RGB Volume", 0, QApplication::UnicodeUTF8));
        actionRGB_directory->setText(QApplication::translate("DrishtiImport", "image directory for RGB volumes", 0, QApplication::UnicodeUTF8));
        actionAnalyze->setText(QApplication::translate("DrishtiImport", "Analyze", 0, QApplication::UnicodeUTF8));
        actionHDF4_directory->setText(QApplication::translate("DrishtiImport", "hdf4 directory", 0, QApplication::UnicodeUTF8));
        saveImage->setText(QApplication::translate("DrishtiImport", "Save Image (Alt+S)", 0, QApplication::UnicodeUTF8));
        saveLimits->setText(QApplication::translate("DrishtiImport", "Save Limits", 0, QApplication::UnicodeUTF8));
        loadLimits->setText(QApplication::translate("DrishtiImport", "Load Limits", 0, QApplication::UnicodeUTF8));
        actionRaw_slices->setText(QApplication::translate("DrishtiImport", "raw slices", 0, QApplication::UnicodeUTF8));
        actionTimeSeries->setText(QApplication::translate("DrishtiImport", "Import Time Series", 0, QApplication::UnicodeUTF8));
        actionConvert->setText(QApplication::translate("DrishtiImport", "Convert", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionConvert->setToolTip(QApplication::translate("DrishtiImport", "Conver old style processed volume to new style.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        actionConvert->setStatusTip(QApplication::translate("DrishtiImport", "Conver old style processed volume to new style.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        actionConvert->setWhatsThis(QApplication::translate("DrishtiImport", "Conver old style processed volume to new style.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        menuFiles->setTitle(QApplication::translate("DrishtiImport", "Files", 0, QApplication::UnicodeUTF8));
        menuLoad->setTitle(QApplication::translate("DrishtiImport", "Load", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("DrishtiImport", "Help", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        menuConvert->setToolTip(QApplication::translate("DrishtiImport", "Convert old style .pvl.nc file to new style.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_WHATSTHIS
        menuConvert->setWhatsThis(QApplication::translate("DrishtiImport", "Convert old style .pvl.nc file to new style.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        menuConvert->setTitle(QApplication::translate("DrishtiImport", "Convert", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DrishtiImport: public Ui_DrishtiImport {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DRISHTIIMPORT_H

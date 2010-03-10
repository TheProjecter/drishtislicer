/********************************************************************************
** Form generated from reading UI file 'savepvldialog.ui'
**
** Created: Mon 11. Jan 15:53:13 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAVEPVLDIALOG_H
#define UI_SAVEPVLDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_SavePvlDialog
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *voxelUnit;
    QSpacerItem *spacerItem;
    QDialogButtonBox *buttonBox;
    QLabel *label_2;
    QLineEdit *voxelSize;
    QSpacerItem *spacerItem1;
    QLabel *label_3;
    QLineEdit *description;
    QLabel *label_4;
    QComboBox *volumeFilter;

    void setupUi(QDialog *SavePvlDialog)
    {
        if (SavePvlDialog->objectName().isEmpty())
            SavePvlDialog->setObjectName(QString::fromUtf8("SavePvlDialog"));
        SavePvlDialog->resize(420, 160);
        SavePvlDialog->setMinimumSize(QSize(420, 160));
        SavePvlDialog->setMaximumSize(QSize(420, 170));
        gridLayout = new QGridLayout(SavePvlDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(SavePvlDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        voxelUnit = new QComboBox(SavePvlDialog);
        voxelUnit->setObjectName(QString::fromUtf8("voxelUnit"));
        voxelUnit->setMaximumSize(QSize(100, 16777215));

        gridLayout->addWidget(voxelUnit, 0, 1, 1, 2);

        spacerItem = new QSpacerItem(53, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 0, 3, 1, 2);

        buttonBox = new QDialogButtonBox(SavePvlDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 0, 5, 3, 1);

        label_2 = new QLabel(SavePvlDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        voxelSize = new QLineEdit(SavePvlDialog);
        voxelSize->setObjectName(QString::fromUtf8("voxelSize"));
        voxelSize->setMaximumSize(QSize(100, 16777215));

        gridLayout->addWidget(voxelSize, 1, 1, 1, 2);

        spacerItem1 = new QSpacerItem(53, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem1, 1, 3, 1, 2);

        label_3 = new QLabel(SavePvlDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        description = new QLineEdit(SavePvlDialog);
        description->setObjectName(QString::fromUtf8("description"));

        gridLayout->addWidget(description, 2, 1, 1, 4);

        label_4 = new QLabel(SavePvlDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        volumeFilter = new QComboBox(SavePvlDialog);
        volumeFilter->setObjectName(QString::fromUtf8("volumeFilter"));

        gridLayout->addWidget(volumeFilter, 3, 1, 1, 1);


        retranslateUi(SavePvlDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SavePvlDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SavePvlDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(SavePvlDialog);
    } // setupUi

    void retranslateUi(QDialog *SavePvlDialog)
    {
        SavePvlDialog->setWindowTitle(QApplication::translate("SavePvlDialog", "Additional Information", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_WHATSTHIS
        SavePvlDialog->setWhatsThis(QApplication::translate("SavePvlDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:xx-large;\">Bilateral filtering is a non-linear filtering technique. It extends the concept of Gaussian smoothing by weighting the filter coefficients with their corresponding relative pixel intensities. Voxels that are very different in intensity from the central pixel are weighted less even though they may be in close proximity to the central voxel</span></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        label->setText(QApplication::translate("SavePvlDialog", "Voxel Unit", 0, QApplication::UnicodeUTF8));
        voxelUnit->clear();
        voxelUnit->insertItems(0, QStringList()
         << QApplication::translate("SavePvlDialog", "no units", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "angstrom", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "nanometer", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "micron", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "millimeter", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "centimeter", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "meter", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "kilometer", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "parsec", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "kiloparsec", 0, QApplication::UnicodeUTF8)
        );
        label_2->setText(QApplication::translate("SavePvlDialog", "Voxel Size", 0, QApplication::UnicodeUTF8));
        voxelSize->setText(QApplication::translate("SavePvlDialog", "1 1 1", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("SavePvlDialog", "Description", 0, QApplication::UnicodeUTF8));
        description->setText(QApplication::translate("SavePvlDialog", "Information about volume", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("SavePvlDialog", "Volume Filter", 0, QApplication::UnicodeUTF8));
        volumeFilter->clear();
        volumeFilter->insertItems(0, QStringList()
         << QApplication::translate("SavePvlDialog", "no filter", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "3x3", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "5x5", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "7x7", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "9x9", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("SavePvlDialog", "11x11", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class SavePvlDialog: public Ui_SavePvlDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAVEPVLDIALOG_H

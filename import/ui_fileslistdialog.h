/********************************************************************************
** Form generated from reading UI file 'fileslistdialog.ui'
**
** Created: Mon 11. Jan 15:53:13 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILESLISTDIALOG_H
#define UI_FILESLISTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FilesListDialog
{
public:
    QVBoxLayout *vboxLayout;
    QListWidget *listWidget;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FilesListDialog)
    {
        if (FilesListDialog->objectName().isEmpty())
            FilesListDialog->setObjectName(QString::fromUtf8("FilesListDialog"));
        FilesListDialog->resize(300, 260);
        FilesListDialog->setMinimumSize(QSize(200, 10));
        FilesListDialog->setMaximumSize(QSize(500, 16777215));
        vboxLayout = new QVBoxLayout(FilesListDialog);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        listWidget = new QListWidget(FilesListDialog);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        vboxLayout->addWidget(listWidget);

        buttonBox = new QDialogButtonBox(FilesListDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

        vboxLayout->addWidget(buttonBox);


        retranslateUi(FilesListDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), FilesListDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), FilesListDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(FilesListDialog);
    } // setupUi

    void retranslateUi(QDialog *FilesListDialog)
    {
        FilesListDialog->setWindowTitle(QApplication::translate("FilesListDialog", "Loading Files", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FilesListDialog: public Ui_FilesListDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILESLISTDIALOG_H

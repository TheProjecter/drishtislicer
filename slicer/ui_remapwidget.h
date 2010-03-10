/********************************************************************************
** Form generated from reading UI file 'remapwidget.ui'
**
** Created: Thu 4. Feb 16:05:12 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REMAPWIDGET_H
#define UI_REMAPWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RemapWidget
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QCheckBox *checkBox;
    QWidget *colorFrame;
    QSpacerItem *spacerItem;
    QRadioButton *butZ;
    QRadioButton *butY;
    QRadioButton *butX;
    QFrame *histogramFrame;
    QHBoxLayout *hboxLayout1;
    QFrame *sliderFrame;
    QFrame *imageFrame;

    void setupUi(QWidget *RemapWidget)
    {
        if (RemapWidget->objectName().isEmpty())
            RemapWidget->setObjectName(QString::fromUtf8("RemapWidget"));
        RemapWidget->resize(466, 627);
        vboxLayout = new QVBoxLayout(RemapWidget);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        checkBox = new QCheckBox(RemapWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(checkBox->sizePolicy().hasHeightForWidth());
        checkBox->setSizePolicy(sizePolicy);
        checkBox->setMinimumSize(QSize(70, 30));
        checkBox->setMaximumSize(QSize(100, 30));
        checkBox->setFocusPolicy(Qt::ClickFocus);
        checkBox->setChecked(true);

        hboxLayout->addWidget(checkBox);

        colorFrame = new QWidget(RemapWidget);
        colorFrame->setObjectName(QString::fromUtf8("colorFrame"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(colorFrame->sizePolicy().hasHeightForWidth());
        colorFrame->setSizePolicy(sizePolicy1);
        colorFrame->setMinimumSize(QSize(200, 50));
        colorFrame->setMaximumSize(QSize(16777215, 50));

        hboxLayout->addWidget(colorFrame);

        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        butZ = new QRadioButton(RemapWidget);
        butZ->setObjectName(QString::fromUtf8("butZ"));
        butZ->setChecked(true);

        hboxLayout->addWidget(butZ);

        butY = new QRadioButton(RemapWidget);
        butY->setObjectName(QString::fromUtf8("butY"));

        hboxLayout->addWidget(butY);

        butX = new QRadioButton(RemapWidget);
        butX->setObjectName(QString::fromUtf8("butX"));

        hboxLayout->addWidget(butX);


        vboxLayout->addLayout(hboxLayout);

        histogramFrame = new QFrame(RemapWidget);
        histogramFrame->setObjectName(QString::fromUtf8("histogramFrame"));
        histogramFrame->setMaximumSize(QSize(16777215, 300));
        histogramFrame->setFocusPolicy(Qt::StrongFocus);
        histogramFrame->setFrameShape(QFrame::StyledPanel);
        histogramFrame->setFrameShadow(QFrame::Plain);
        histogramFrame->setLineWidth(2);
        histogramFrame->setMidLineWidth(0);

        vboxLayout->addWidget(histogramFrame);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        sliderFrame = new QFrame(RemapWidget);
        sliderFrame->setObjectName(QString::fromUtf8("sliderFrame"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(sliderFrame->sizePolicy().hasHeightForWidth());
        sliderFrame->setSizePolicy(sizePolicy2);
        sliderFrame->setMinimumSize(QSize(100, 16));
        sliderFrame->setMaximumSize(QSize(160, 16777215));
        sliderFrame->setFrameShape(QFrame::StyledPanel);
        sliderFrame->setFrameShadow(QFrame::Plain);

        hboxLayout1->addWidget(sliderFrame);

        imageFrame = new QFrame(RemapWidget);
        imageFrame->setObjectName(QString::fromUtf8("imageFrame"));
        imageFrame->setFocusPolicy(Qt::StrongFocus);
        imageFrame->setFrameShape(QFrame::StyledPanel);
        imageFrame->setFrameShadow(QFrame::Plain);
        imageFrame->setLineWidth(21);

        hboxLayout1->addWidget(imageFrame);


        vboxLayout->addLayout(hboxLayout1);


        retranslateUi(RemapWidget);
        QObject::connect(checkBox, SIGNAL(toggled(bool)), histogramFrame, SLOT(setVisible(bool)));

        QMetaObject::connectSlotsByName(RemapWidget);
    } // setupUi

    void retranslateUi(QWidget *RemapWidget)
    {
        RemapWidget->setWindowTitle(QApplication::translate("RemapWidget", "Remap Raw Volume", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("RemapWidget", "Histogram", 0, QApplication::UnicodeUTF8));
        butZ->setText(QApplication::translate("RemapWidget", "Z", 0, QApplication::UnicodeUTF8));
        butY->setText(QApplication::translate("RemapWidget", "Y", 0, QApplication::UnicodeUTF8));
        butX->setText(QApplication::translate("RemapWidget", "X", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RemapWidget: public Ui_RemapWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REMAPWIDGET_H

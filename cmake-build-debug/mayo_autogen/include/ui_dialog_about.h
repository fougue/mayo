/********************************************************************************
** Form generated from reading UI file 'dialog_about.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_ABOUT_H
#define UI_DIALOG_ABOUT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>

namespace Mayo {

class Ui_DialogAbout
{
public:
    QGridLayout *gridLayout_2;
    QLabel *label_Logo;
    QLabel *label_AppByOrg;
    QLabel *label_Version;
    QLabel *label_BuildDateTime;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QLabel *label_Qt;
    QLabel *label_Occ;
    QLabel *label_Gmio;
    QSpacerItem *verticalSpacer_3;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Mayo__DialogAbout)
    {
        if (Mayo__DialogAbout->objectName().isEmpty())
            Mayo__DialogAbout->setObjectName(QString::fromUtf8("Mayo__DialogAbout"));
        Mayo__DialogAbout->resize(288, 175);
        Mayo__DialogAbout->setModal(true);
        gridLayout_2 = new QGridLayout(Mayo__DialogAbout);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_Logo = new QLabel(Mayo__DialogAbout);
        label_Logo->setObjectName(QString::fromUtf8("label_Logo"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_Logo->sizePolicy().hasHeightForWidth());
        label_Logo->setSizePolicy(sizePolicy);
        label_Logo->setMaximumSize(QSize(128, 128));
        label_Logo->setPixmap(QPixmap(QString::fromUtf8(":/images/appicon_64.png")));
        label_Logo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_2->addWidget(label_Logo, 0, 0, 4, 1);

        label_AppByOrg = new QLabel(Mayo__DialogAbout);
        label_AppByOrg->setObjectName(QString::fromUtf8("label_AppByOrg"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        label_AppByOrg->setFont(font);

        gridLayout_2->addWidget(label_AppByOrg, 0, 1, 1, 2);

        label_Version = new QLabel(Mayo__DialogAbout);
        label_Version->setObjectName(QString::fromUtf8("label_Version"));

        gridLayout_2->addWidget(label_Version, 1, 1, 1, 2);

        label_BuildDateTime = new QLabel(Mayo__DialogAbout);
        label_BuildDateTime->setObjectName(QString::fromUtf8("label_BuildDateTime"));

        gridLayout_2->addWidget(label_BuildDateTime, 2, 1, 1, 2);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 3, 1, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_Qt = new QLabel(Mayo__DialogAbout);
        label_Qt->setObjectName(QString::fromUtf8("label_Qt"));

        gridLayout->addWidget(label_Qt, 0, 0, 1, 1);

        label_Occ = new QLabel(Mayo__DialogAbout);
        label_Occ->setObjectName(QString::fromUtf8("label_Occ"));

        gridLayout->addWidget(label_Occ, 1, 0, 1, 1);

        label_Gmio = new QLabel(Mayo__DialogAbout);
        label_Gmio->setObjectName(QString::fromUtf8("label_Gmio"));

        gridLayout->addWidget(label_Gmio, 2, 0, 1, 1);


        gridLayout_2->addLayout(gridLayout, 3, 2, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 35, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_3, 4, 1, 1, 1);

        buttonBox = new QDialogButtonBox(Mayo__DialogAbout);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        gridLayout_2->addWidget(buttonBox, 5, 2, 1, 1);


        retranslateUi(Mayo__DialogAbout);
        QObject::connect(buttonBox, SIGNAL(accepted()), Mayo__DialogAbout, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Mayo__DialogAbout, SLOT(reject()));

        QMetaObject::connectSlotsByName(Mayo__DialogAbout);
    } // setupUi

    void retranslateUi(QDialog *Mayo__DialogAbout)
    {
        Mayo__DialogAbout->setWindowTitle(QApplication::translate("Mayo::DialogAbout", "About", nullptr));
        label_Logo->setText(QString());
        label_AppByOrg->setText(QApplication::translate("Mayo::DialogAbout", "Mayo By Fougue Ltd.", nullptr));
        label_Version->setText(QApplication::translate("Mayo::DialogAbout", "Version %1 (%2bit)", nullptr));
        label_BuildDateTime->setText(QApplication::translate("Mayo::DialogAbout", "Built on %1 at %2", nullptr));
        label_Qt->setText(QApplication::translate("Mayo::DialogAbout", "Qt %1", nullptr));
        label_Occ->setText(QApplication::translate("Mayo::DialogAbout", "OpenCascade %1", nullptr));
        label_Gmio->setText(QApplication::translate("Mayo::DialogAbout", "gmio %1", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class DialogAbout: public Ui_DialogAbout {};
} // namespace Ui
} // namespace Mayo

#endif // UI_DIALOG_ABOUT_H

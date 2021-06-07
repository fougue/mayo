/********************************************************************************
** Form generated from reading UI file 'dialog_options.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_OPTIONS_H
#define UI_DIALOG_OPTIONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

namespace Mayo {

class Ui_DialogOptions
{
public:
    QGridLayout *gridLayout;
    QTreeView *treeView_GroupSections;
    QDialogButtonBox *buttonBox;
    QListWidget *listWidget_Settings;
    QWidget *widget_Spacer;

    void setupUi(QDialog *Mayo__DialogOptions)
    {
        if (Mayo__DialogOptions->objectName().isEmpty())
            Mayo__DialogOptions->setObjectName(QString::fromUtf8("Mayo__DialogOptions"));
        Mayo__DialogOptions->resize(1002, 686);
        Mayo__DialogOptions->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"    font-weight: bold;\n"
"}"));
        Mayo__DialogOptions->setModal(true);
        gridLayout = new QGridLayout(Mayo__DialogOptions);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        treeView_GroupSections = new QTreeView(Mayo__DialogOptions);
        treeView_GroupSections->setObjectName(QString::fromUtf8("treeView_GroupSections"));
        treeView_GroupSections->header()->setVisible(false);

        gridLayout->addWidget(treeView_GroupSections, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(Mayo__DialogOptions);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::RestoreDefaults);

        gridLayout->addWidget(buttonBox, 1, 3, 1, 1);

        listWidget_Settings = new QListWidget(Mayo__DialogOptions);
        listWidget_Settings->setObjectName(QString::fromUtf8("listWidget_Settings"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(5);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(listWidget_Settings->sizePolicy().hasHeightForWidth());
        listWidget_Settings->setSizePolicy(sizePolicy);

        gridLayout->addWidget(listWidget_Settings, 0, 1, 1, 3);

        widget_Spacer = new QWidget(Mayo__DialogOptions);
        widget_Spacer->setObjectName(QString::fromUtf8("widget_Spacer"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(5);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget_Spacer->sizePolicy().hasHeightForWidth());
        widget_Spacer->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(widget_Spacer, 1, 2, 1, 1);


        retranslateUi(Mayo__DialogOptions);
        QObject::connect(buttonBox, SIGNAL(accepted()), Mayo__DialogOptions, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Mayo__DialogOptions, SLOT(reject()));

        QMetaObject::connectSlotsByName(Mayo__DialogOptions);
    } // setupUi

    void retranslateUi(QDialog *Mayo__DialogOptions)
    {
        Mayo__DialogOptions->setWindowTitle(QApplication::translate("Mayo::DialogOptions", "Options", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class DialogOptions: public Ui_DialogOptions {};
} // namespace Ui
} // namespace Mayo

#endif // UI_DIALOG_OPTIONS_H

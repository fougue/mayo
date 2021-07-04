/********************************************************************************
** Form generated from reading UI file 'dialog_inspect_xde.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_INSPECT_XDE_H
#define UI_DIALOG_INSPECT_XDE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

namespace Mayo {

class Ui_DialogInspectXde
{
public:
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTreeWidget *treeWidget_Document;
    QTreeWidget *treeWidget_LabelProps;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Mayo__DialogInspectXde)
    {
        if (Mayo__DialogInspectXde->objectName().isEmpty())
            Mayo__DialogInspectXde->setObjectName(QString::fromUtf8("Mayo__DialogInspectXde"));
        Mayo__DialogInspectXde->resize(1050, 665);
        Mayo__DialogInspectXde->setModal(true);
        verticalLayout = new QVBoxLayout(Mayo__DialogInspectXde);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        splitter = new QSplitter(Mayo__DialogInspectXde);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        treeWidget_Document = new QTreeWidget(splitter);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget_Document->setHeaderItem(__qtreewidgetitem);
        treeWidget_Document->setObjectName(QString::fromUtf8("treeWidget_Document"));
        splitter->addWidget(treeWidget_Document);
        treeWidget_Document->header()->setVisible(false);
        treeWidget_LabelProps = new QTreeWidget(splitter);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        treeWidget_LabelProps->setHeaderItem(__qtreewidgetitem1);
        treeWidget_LabelProps->setObjectName(QString::fromUtf8("treeWidget_LabelProps"));
        treeWidget_LabelProps->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeWidget_LabelProps->setSelectionMode(QAbstractItemView::NoSelection);
        treeWidget_LabelProps->setColumnCount(2);
        splitter->addWidget(treeWidget_LabelProps);
        treeWidget_LabelProps->header()->setVisible(false);

        verticalLayout->addWidget(splitter);

        buttonBox = new QDialogButtonBox(Mayo__DialogInspectXde);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(Mayo__DialogInspectXde);
        QObject::connect(buttonBox, SIGNAL(accepted()), Mayo__DialogInspectXde, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Mayo__DialogInspectXde, SLOT(reject()));

        QMetaObject::connectSlotsByName(Mayo__DialogInspectXde);
    } // setupUi

    void retranslateUi(QDialog *Mayo__DialogInspectXde)
    {
        Mayo__DialogInspectXde->setWindowTitle(QApplication::translate("Mayo::DialogInspectXde", "XDE", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class DialogInspectXde: public Ui_DialogInspectXde {};
} // namespace Ui
} // namespace Mayo

#endif // UI_DIALOG_INSPECT_XDE_H

/********************************************************************************
** Form generated from reading UI file 'widget_model_tree.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_MODEL_TREE_H
#define UI_WIDGET_MODEL_TREE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "widget_model_tree.h"

namespace Mayo {

class Ui_WidgetModelTree
{
public:
    QVBoxLayout *verticalLayout;
    Mayo::Internal::TreeWidget *treeWidget_Model;

    void setupUi(QWidget *Mayo__WidgetModelTree)
    {
        if (Mayo__WidgetModelTree->objectName().isEmpty())
            Mayo__WidgetModelTree->setObjectName(QString::fromUtf8("Mayo__WidgetModelTree"));
        Mayo__WidgetModelTree->resize(247, 472);
        verticalLayout = new QVBoxLayout(Mayo__WidgetModelTree);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        treeWidget_Model = new Mayo::Internal::TreeWidget(Mayo__WidgetModelTree);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget_Model->setHeaderItem(__qtreewidgetitem);
        treeWidget_Model->setObjectName(QString::fromUtf8("treeWidget_Model"));
        treeWidget_Model->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeWidget_Model->setTextElideMode(Qt::ElideNone);
        treeWidget_Model->header()->setVisible(false);

        verticalLayout->addWidget(treeWidget_Model);


        retranslateUi(Mayo__WidgetModelTree);

        QMetaObject::connectSlotsByName(Mayo__WidgetModelTree);
    } // setupUi

    void retranslateUi(QWidget *Mayo__WidgetModelTree)
    {
        Mayo__WidgetModelTree->setWindowTitle(QApplication::translate("Mayo::WidgetModelTree", "Form", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class WidgetModelTree: public Ui_WidgetModelTree {};
} // namespace Ui
} // namespace Mayo

#endif // UI_WIDGET_MODEL_TREE_H

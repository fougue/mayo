/********************************************************************************
** Form generated from reading UI file 'dialog_task_manager.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_TASK_MANAGER_H
#define UI_DIALOG_TASK_MANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace Mayo {

class Ui_DialogTaskManager
{
public:
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaContents;
    QVBoxLayout *contentsLayout;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *Mayo__DialogTaskManager)
    {
        if (Mayo__DialogTaskManager->objectName().isEmpty())
            Mayo__DialogTaskManager->setObjectName(QString::fromUtf8("Mayo__DialogTaskManager"));
        Mayo__DialogTaskManager->resize(493, 211);
        Mayo__DialogTaskManager->setModal(true);
        verticalLayout = new QVBoxLayout(Mayo__DialogTaskManager);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        scrollArea = new QScrollArea(Mayo__DialogTaskManager);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaContents = new QWidget();
        scrollAreaContents->setObjectName(QString::fromUtf8("scrollAreaContents"));
        scrollAreaContents->setGeometry(QRect(0, 0, 491, 209));
        contentsLayout = new QVBoxLayout(scrollAreaContents);
        contentsLayout->setObjectName(QString::fromUtf8("contentsLayout"));
        verticalSpacer = new QSpacerItem(20, 207, QSizePolicy::Minimum, QSizePolicy::Expanding);

        contentsLayout->addItem(verticalSpacer);

        scrollArea->setWidget(scrollAreaContents);

        verticalLayout->addWidget(scrollArea);


        retranslateUi(Mayo__DialogTaskManager);

        QMetaObject::connectSlotsByName(Mayo__DialogTaskManager);
    } // setupUi

    void retranslateUi(QDialog *Mayo__DialogTaskManager)
    {
        Mayo__DialogTaskManager->setWindowTitle(QApplication::translate("Mayo::DialogTaskManager", "Tasks", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class DialogTaskManager: public Ui_DialogTaskManager {};
} // namespace Ui
} // namespace Mayo

#endif // UI_DIALOG_TASK_MANAGER_H

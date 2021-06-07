/********************************************************************************
** Form generated from reading UI file 'widget_explode_assembly.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_EXPLODE_ASSEMBLY_H
#define UI_WIDGET_EXPLODE_ASSEMBLY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

namespace Mayo {

class Ui_WidgetExplodeAssembly
{
public:
    QHBoxLayout *horizontalLayout;
    QSlider *slider_Factor;
    QSpinBox *edit_Factor;

    void setupUi(QWidget *Mayo__WidgetExplodeAssembly)
    {
        if (Mayo__WidgetExplodeAssembly->objectName().isEmpty())
            Mayo__WidgetExplodeAssembly->setObjectName(QString::fromUtf8("Mayo__WidgetExplodeAssembly"));
        Mayo__WidgetExplodeAssembly->resize(156, 22);
        horizontalLayout = new QHBoxLayout(Mayo__WidgetExplodeAssembly);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        slider_Factor = new QSlider(Mayo__WidgetExplodeAssembly);
        slider_Factor->setObjectName(QString::fromUtf8("slider_Factor"));
        slider_Factor->setMaximum(100);
        slider_Factor->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(slider_Factor);

        edit_Factor = new QSpinBox(Mayo__WidgetExplodeAssembly);
        edit_Factor->setObjectName(QString::fromUtf8("edit_Factor"));
        edit_Factor->setMaximum(100);

        horizontalLayout->addWidget(edit_Factor);


        retranslateUi(Mayo__WidgetExplodeAssembly);

        QMetaObject::connectSlotsByName(Mayo__WidgetExplodeAssembly);
    } // setupUi

    void retranslateUi(QWidget *Mayo__WidgetExplodeAssembly)
    {
        Mayo__WidgetExplodeAssembly->setWindowTitle(QApplication::translate("Mayo::WidgetExplodeAssembly", "Form", nullptr));
        edit_Factor->setSuffix(QApplication::translate("Mayo::WidgetExplodeAssembly", "%", nullptr));
    } // retranslateUi

};

} // namespace Mayo

namespace Mayo {
namespace Ui {
    class WidgetExplodeAssembly: public Ui_WidgetExplodeAssembly {};
} // namespace Ui
} // namespace Mayo

#endif // UI_WIDGET_EXPLODE_ASSEMBLY_H

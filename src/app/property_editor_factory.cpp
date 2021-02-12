/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_editor_factory.h"

#include "../base/application.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/string_utils.h"
#include "../base/unit_system.h"
#include "../gui/qtgui_utils.h"
#include "app_module.h"
#include "widgets_utils.h"

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

namespace Mayo {

class PropertyEditorI18N { Q_DECLARE_TR_FUNCTIONS(Mayo::PropertyEditorI18N) };

namespace {

static QWidget* hSpacerWidget(QWidget* parent, int stretch = 1)
{
    auto widget = new QWidget(parent);
    QSizePolicy sp = widget->sizePolicy();
    sp.setHorizontalStretch(stretch);
    widget->setSizePolicy(sp);
    return widget;
}

struct InterfacePropertyEditor {
    virtual void syncWithProperty() = 0;
};

struct PropertyBoolEditor : public InterfacePropertyEditor, public QCheckBox {
    PropertyBoolEditor(PropertyBool* property, QWidget* parentWidget)
        : QCheckBox(parentWidget), m_property(property)
    {
        QObject::connect(this, &QCheckBox::toggled, [=](bool on) {
            property->setValue(on);
            this->setText(StringUtils::yesNoText(on));
        });
    }

    void syncWithProperty() override {
        this->setText(StringUtils::yesNoText(*m_property));
        this->setChecked(*m_property);
    }

    PropertyBool* m_property;
};

struct PropertyIntEditor : public InterfacePropertyEditor, public QSpinBox {
    PropertyIntEditor(PropertyInt* property, QWidget* parentWidget)
        : QSpinBox(parentWidget), m_property(property)
    {
        if (property->constraintsEnabled()) {
            this->setRange(property->minimum(), property->maximum());
            this->setSingleStep(property->singleStep());
        }

        QObject::connect(this, qOverload<int>(&QSpinBox::valueChanged), [=](int val) {
            property->setValue(val);
        });
    }

    void syncWithProperty() override {
        this->setValue(m_property->value());
    }

    PropertyInt* m_property;
};

struct PropertyDoubleEditor : public InterfacePropertyEditor, public QDoubleSpinBox {
    PropertyDoubleEditor(PropertyDouble* property, QWidget* parentWidget)
        : QDoubleSpinBox(parentWidget), m_property(property)
    {
        if (property->constraintsEnabled()) {
            this->setRange(property->minimum(), property->maximum());
            this->setSingleStep(property->singleStep());
        }

        this->setDecimals(AppModule::get(Application::instance())->unitSystemDecimals.value());
        QObject::connect(this, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double val) {
            property->setValue(val);
        });
    }

    void syncWithProperty() override {
        this->setValue(m_property->value());
    }

    PropertyDouble* m_property;
};

struct PropertyCheckStateEditor : public InterfacePropertyEditor, public QCheckBox {
    PropertyCheckStateEditor(PropertyCheckState* property, QWidget* parentWidget)
        : QCheckBox(parentWidget), m_property(property)
    {
        //this->setTristate();
        QObject::connect(this, &QCheckBox::stateChanged, [=](int state) {
            property->setValue(Qt::CheckState(state));
            this->setText(StringUtils::yesNoText(Qt::CheckState(state)));
        });
    }

    void syncWithProperty() override {
        this->setText(StringUtils::yesNoText(*m_property));
        this->setChecked(*m_property);
    }

    PropertyCheckState* m_property;
};

struct PropertyQStringEditor : public InterfacePropertyEditor, public QLineEdit {
    PropertyQStringEditor(PropertyQString* property, QWidget* parentWidget)
        : QLineEdit(parentWidget), m_property(property)
    {
        QObject::connect(this, &QLineEdit::textChanged, [=](const QString& text) {
            property->setValue(text);
        });
    }

    void syncWithProperty() override {
        this->setText(m_property->value());
    }

    PropertyQString* m_property;
};

struct PropertyEnumerationEditor : public InterfacePropertyEditor, public QComboBox {
    PropertyEnumerationEditor(PropertyEnumeration* property, QWidget* parentWidget)
        : QComboBox(parentWidget), m_property(property)
    {
        for (const Enumeration::Item& enumItem : property->enumeration().items()) {
            this->addItem(enumItem.name.tr(), enumItem.value);
            const QString itemDescr = property->findDescription(enumItem.value);
            if (!itemDescr.isEmpty()) {
                const int itemIndex = this->count() - 1;
                this->setItemData(itemIndex, itemDescr, Qt::ToolTipRole);
            }
        }

        QObject::connect(this, qOverload<int>(&QComboBox::activated), [=](int index) {
            property->setValue(this->itemData(index).toInt());
        });
    }

    void syncWithProperty() override {
        this->setCurrentIndex(this->findData(m_property->value()));
    }

    PropertyEnumeration* m_property;
};

struct PropertyOccColorEditor : public InterfacePropertyEditor, public QWidget {
    PropertyOccColorEditor(PropertyOccColor* property, QWidget* parentWidget)
        : QWidget(parentWidget), m_property(property)
    {
        QWidget* frame = this;
        auto frameLayout = new QHBoxLayout(frame);
        frameLayout->setContentsMargins(0, 0, 0, 0);

        m_labelColor = new QLabel(frame);
        m_labelRgb = new QLabel(frame);
        auto btnColor = new QToolButton(frame);
        btnColor->setText("...");
        btnColor->setToolTip(PropertyEditorI18N::tr("Choose color ..."));

        QObject::connect(btnColor, &QAbstractButton::clicked, [=]{
            auto dlg = new QColorDialog(frame);
            dlg->setCurrentColor(QtGuiUtils::toQColor(property->value()));
            QObject::connect(dlg, &QColorDialog::colorSelected, [=](const QColor& c) {
                property->setValue(QtGuiUtils::toColor<Quantity_Color>(c));
                this->syncWithProperty();
            });
            WidgetsUtils::asyncDialogExec(dlg);
        });

        frameLayout->addWidget(m_labelColor);
        frameLayout->addWidget(m_labelRgb);
        frameLayout->addWidget(btnColor);
        frameLayout->addWidget(hSpacerWidget(frame));
    }

    void syncWithProperty() override {
        const QColor qtColor = QtGuiUtils::toQColor(m_property->value());
        m_labelColor->setPixmap(PropertyEditorFactory::colorSquarePixmap(qtColor));
        m_labelRgb->setText(StringUtils::text(m_property->value()));
    }

    PropertyOccColor* m_property;
    QLabel* m_labelColor = nullptr;
    QLabel* m_labelRgb = nullptr;
};

struct PropertyOccPntEditor : public InterfacePropertyEditor, public QWidget {
    PropertyOccPntEditor(PropertyOccPnt* property, QWidget* parentWidget)
        : QWidget(parentWidget), m_property(property)
    {
        QWidget* frame = this;
        auto frameLayout = new QHBoxLayout(frame);
        frameLayout->setContentsMargins(0, 0, 0, 0);
        m_xCoordEditor = createCoordEditor(frame, property, &gp_Pnt::SetX);
        m_yCoordEditor = createCoordEditor(frame, property, &gp_Pnt::SetY);
        m_zCoordEditor = createCoordEditor(frame, property, &gp_Pnt::SetZ);
        frameLayout->addWidget(new QLabel("X", frame));
        frameLayout->addWidget(m_xCoordEditor);
        frameLayout->addWidget(new QLabel("Y", frame));
        frameLayout->addWidget(m_yCoordEditor);
        frameLayout->addWidget(new QLabel("Z", frame));
        frameLayout->addWidget(m_zCoordEditor);
    }

    void syncWithProperty() override {
        auto fnSetEditorCoord = [](QDoubleSpinBox* editor, double coord){
            auto appModule = AppModule::get(Application::instance());
            auto trRes = UnitSystem::translate(appModule->unitSystemSchema, coord * Quantity_Millimeter);
            QSignalBlocker sigBlock(editor); Q_UNUSED(sigBlock);
            editor->setValue(trRes.value);
        };

        fnSetEditorCoord(m_xCoordEditor, m_property->value().X());
        fnSetEditorCoord(m_yCoordEditor, m_property->value().Y());
        fnSetEditorCoord(m_zCoordEditor, m_property->value().Z());
    }

    static QDoubleSpinBox* createCoordEditor(
            QWidget* parentWidget,
            PropertyOccPnt* property,
            void (gp_Pnt::*funcSetCoord)(double))
    {
        auto editor = new QDoubleSpinBox(parentWidget);
        auto appModule = AppModule::get(Application::instance());
        auto trRes = UnitSystem::translate(appModule->unitSystemSchema, 1., Unit::Length);
        //editor->setSuffix(QString::fromUtf8(trRes.strUnit));
        editor->setDecimals(appModule->unitSystemDecimals.value());
        editor->setButtonSymbols(QDoubleSpinBox::NoButtons);
        editor->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
        QSizePolicy sp = editor->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        editor->setSizePolicy(sp);
        editor->setMinimumWidth(25);
        QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value) {
            const double f = trRes.factor;
            value = qFuzzyCompare(f, 1.) ? value : value * f;
            gp_Pnt pnt = property->value();
            (pnt.*funcSetCoord)(value);
            property->setValue(pnt);
        });
        return editor;
    }

    PropertyOccPnt* m_property;
    QDoubleSpinBox* m_xCoordEditor = nullptr;
    QDoubleSpinBox* m_yCoordEditor = nullptr;
    QDoubleSpinBox* m_zCoordEditor = nullptr;
};

struct PropertyQuantityEditor : public InterfacePropertyEditor, public QDoubleSpinBox {
    PropertyQuantityEditor(BasePropertyQuantity* property, QWidget* parentWidget)
        : QDoubleSpinBox(parentWidget), m_property(property)
    {
        const UnitSystem::TranslateResult trRes = PropertyEditorFactory::unitTranslate(property);
        this->setSuffix(QString::fromUtf8(trRes.strUnit));
        this->setDecimals(AppModule::get(Application::instance())->unitSystemDecimals.value());
        const double rangeMin =
                property->constraintsEnabled() ?
                    property->minimum() : std::numeric_limits<double>::min();
        const double rangeMax =
                property->constraintsEnabled() ?
                    property->maximum() : std::numeric_limits<double>::max();
        this->setRange(rangeMin, rangeMax);
        QObject::connect(this, &QDoubleSpinBox::editingFinished, [=]{
            double value = this->value();
            const double f = trRes.factor;
            value = qFuzzyCompare(f, 1.) ? value : value * f;
            if (!qFuzzyCompare(property->quantityValue(), value))
                property->setQuantityValue(value);
        });
    }

    void syncWithProperty() override {
        const UnitSystem::TranslateResult trRes = PropertyEditorFactory::unitTranslate(m_property);
        this->setValue(trRes.value);
    }

    BasePropertyQuantity* m_property;
};

} // namespace

QWidget* DefaultPropertyEditorFactory::createEditor(Property* property, QWidget* parentWidget) const
{
    QWidget* editor = nullptr;
    const char* propTypeName = property ? property->dynTypeName() : nullptr;
    if (propTypeName == PropertyBool::TypeName)
        editor = new PropertyBoolEditor(static_cast<PropertyBool*>(property), parentWidget);

    if (propTypeName == PropertyInt::TypeName)
        editor = new PropertyIntEditor(static_cast<PropertyInt*>(property), parentWidget);

    if (propTypeName == PropertyDouble::TypeName)
        editor = new PropertyDoubleEditor(static_cast<PropertyDouble*>(property), parentWidget);

    if (propTypeName == PropertyCheckState::TypeName)
        editor = new PropertyCheckStateEditor(static_cast<PropertyCheckState*>(property), parentWidget);

    if (propTypeName == PropertyQString::TypeName)
        editor = new PropertyQStringEditor(static_cast<PropertyQString*>(property), parentWidget);

    if (propTypeName == PropertyOccColor::TypeName)
        editor = new PropertyOccColorEditor(static_cast<PropertyOccColor*>(property), parentWidget);

    if (propTypeName == PropertyOccPnt::TypeName)
        editor = new PropertyOccPntEditor(static_cast<PropertyOccPnt*>(property), parentWidget);

    if (propTypeName == PropertyEnumeration::TypeName)
        editor = new PropertyEnumerationEditor(static_cast<PropertyEnumeration*>(property), parentWidget);

    if (propTypeName == BasePropertyQuantity::TypeName)
        editor = new PropertyQuantityEditor(static_cast<BasePropertyQuantity*>(property), parentWidget);

    this->syncEditorWithProperty(editor);
    return editor;
}

void DefaultPropertyEditorFactory::syncEditorWithProperty(QWidget* editor) const
{
    auto intf = dynamic_cast<InterfacePropertyEditor*>(editor);
    if (intf) {
        QSignalBlocker sigBlocker(editor); Q_UNUSED(sigBlocker);
        intf->syncWithProperty();
    }
}

UnitSystem::TranslateResult PropertyEditorFactory::unitTranslate(const BasePropertyQuantity* property)
{
    if (!property)
        return {};

    if (property->quantityUnit() == Unit::Angle) {
        auto propAngle = static_cast<const PropertyAngle*>(property);
        return UnitSystem::degrees(propAngle->quantity());
    }

    return UnitSystem::translate(
                AppModule::get(Application::instance())->unitSystemSchema,
                property->quantityValue(),
                property->quantityUnit());
}

QPixmap PropertyEditorFactory::colorSquarePixmap(const QColor& c, int sideLen)
{
    QPixmap pixColor(sideLen, sideLen);
    pixColor.fill(c);
    return pixColor;
}

} // namespace Mayo

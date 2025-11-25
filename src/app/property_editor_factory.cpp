/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_editor_factory.h"

#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/unit_system.h"
#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"
#include "app_module.h"
#include "qstring_utils.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

#include <limits>

namespace Mayo {

class PropertyEditorI18N { Q_DECLARE_TR_FUNCTIONS(Mayo::PropertyEditorI18N) };

namespace {

// Helper that returns an empty widget with stretch-based horizontal space
static QWidget* hSpacerWidget(QWidget* parent, int stretch = 1)
{
    auto widget = new QWidget(parent);
    QSizePolicy sp = widget->sizePolicy();
    sp.setHorizontalStretch(stretch);
    widget->setSizePolicy(sp);
    return widget;
}

// Base interface of all property editors
struct InterfacePropertyEditor {
    virtual void syncWithProperty() = 0;
};

struct PropertyBoolEditor : public InterfacePropertyEditor, public QCheckBox {
    PropertyBoolEditor(PropertyBool* property, QWidget* parentWidget)
        : QCheckBox(parentWidget), m_property(property)
    {
        QObject::connect(this, &QCheckBox::toggled, [=](bool on) {
            property->setValue(on);
            this->setText(QStringUtils::yesNoText(on));
        });
    }

    void syncWithProperty() override {
        this->setText(QStringUtils::yesNoText(*m_property));
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
        else {
            this->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
            this->setButtonSymbols(QAbstractSpinBox::NoButtons);
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
        else {
            this->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
            this->setButtonSymbols(QAbstractSpinBox::NoButtons);
        }

        this->setDecimals(AppModule::get()->properties()->unitSystemDecimals.value());
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
            auto estate = QtCoreUtils::toCheckState(Qt::CheckState(state));
            property->setValue(estate);
            this->setText(QStringUtils::yesNoText(estate));
        });
    }

    void syncWithProperty() override {
        this->setText(QStringUtils::yesNoText(*m_property));
        this->setCheckState(QtCoreUtils::toQtCheckState(*m_property));
    }

    PropertyCheckState* m_property;
};

struct PropertyStringEditor : public InterfacePropertyEditor, public QLineEdit {
    PropertyStringEditor(PropertyString* property, QWidget* parentWidget)
        : QLineEdit(parentWidget), m_property(property)
    {
        QObject::connect(this, &QLineEdit::textChanged, [=](const QString& text) {
            property->setValue(to_stdString(text));
        });
    }

    void syncWithProperty() override {
        this->setText(to_QString(m_property->value()));
    }

    PropertyString* m_property;
};

struct PropertyEnumerationEditor : public InterfacePropertyEditor, public QComboBox {
    PropertyEnumerationEditor(PropertyEnumeration* property, QWidget* parentWidget)
        : QComboBox(parentWidget), m_property(property)
    {
        for (const Enumeration::Item& enumItem : property->enumeration().items()) {
            this->addItem(to_QString(enumItem.name.tr()), enumItem.value);
            const QString itemDescr = to_QString(property->findDescription(enumItem.value));
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
            QtWidgetsUtils::asyncDialogExec(dlg);
        });

        frameLayout->addWidget(m_labelColor);
        frameLayout->addWidget(m_labelRgb);
        frameLayout->addWidget(btnColor);
        frameLayout->addWidget(hSpacerWidget(frame));
    }

    void syncWithProperty() override {
        const QColor qtColor = QtGuiUtils::toQColor(m_property->value());
        m_labelColor->setPixmap(IPropertyEditorFactory::colorSquarePixmap(qtColor));
        m_labelRgb->setText(QStringUtils::text(m_property->value()));
    }

    PropertyOccColor* m_property;
    QLabel* m_labelColor = nullptr;
    QLabel* m_labelRgb = nullptr;
};

// Helper generic interface over a XYZ-value property type to get/set the coordinates with gp_XYZ
template<typename PropertyCoordsType> class IProperty3dCoords {
    // Expected functions
    //     static const gp_XYZ& coords(const PROPERTY_COORDS_TYPE* prop);
    //     static void setCoords(PROPERTY_COORDS_TYPE* prop, const gp_XYZ& coords);
};

// Partial specialization of IProperty3dCoords for PropertyOccPnt
template<> struct IProperty3dCoords<PropertyOccPnt> {
    static const gp_XYZ& coords(const PropertyOccPnt* prop) { return prop->value().XYZ(); }
    static void setCoords(PropertyOccPnt* prop, const gp_XYZ& coords) { prop->setValue(coords); }
};

// Partial specialization of IProperty3dCoords for PropertyOccVec
template<> struct IProperty3dCoords<PropertyOccVec> {
    static const gp_XYZ& coords(const PropertyOccVec* prop) { return prop->value().XYZ(); }
    static void setCoords(PropertyOccVec* prop, const gp_XYZ& coords) { prop->setValue(coords); }
};

// Generic editor of XYZ-value properties
// The property type must provide a partial specialization of IProperty3dCoords
template<typename PropertyXyzType>
struct Property3dCoordsEditor : public InterfacePropertyEditor, public QWidget {
    using PropertyCoordsType = PropertyXyzType;
    using IProperty3dCoordsType = IProperty3dCoords<PropertyCoordsType>;
    enum class Coord { X, Y, Z };

    Property3dCoordsEditor(PropertyCoordsType* property, QWidget* parentWidget)
        : QWidget(parentWidget), m_property(property)
    {
        QWidget* frame = this;
        auto frameLayout = new QHBoxLayout(frame);
        frameLayout->setContentsMargins(0, 0, 0, 0);
        m_xCoordEditor = createCoordEditor(frame, property, Coord::X);
        m_yCoordEditor = createCoordEditor(frame, property, Coord::Y);
        m_zCoordEditor = createCoordEditor(frame, property, Coord::Z);
        frameLayout->addWidget(new QLabel("X", frame));
        frameLayout->addWidget(m_xCoordEditor);
        frameLayout->addWidget(new QLabel("Y", frame));
        frameLayout->addWidget(m_yCoordEditor);
        frameLayout->addWidget(new QLabel("Z", frame));
        frameLayout->addWidget(m_zCoordEditor);
        frameLayout->addWidget(hSpacerWidget(frame));
    }

    void syncWithProperty() override {
        auto fnSetEditorCoord = [](QDoubleSpinBox* editor, double coord){
            auto appModule = AppModule::get();
            auto trRes = UnitSystem::translate(appModule->properties()->unitSystemSchema, coord * Quantity_Millimeter);
            QSignalBlocker _(editor);
            editor->setValue(trRes.value);
        };

        fnSetEditorCoord(m_xCoordEditor, IProperty3dCoordsType::coords(m_property).X());
        fnSetEditorCoord(m_yCoordEditor, IProperty3dCoordsType::coords(m_property).Y());
        fnSetEditorCoord(m_zCoordEditor, IProperty3dCoordsType::coords(m_property).Z());
    }

    static QDoubleSpinBox* createCoordEditor(QWidget* parentWidget, PropertyCoordsType* property, Coord specCoord)
    {
        auto editor = new QDoubleSpinBox(parentWidget);
        auto appModule = AppModule::get();
        auto trRes = UnitSystem::translate(appModule->properties()->unitSystemSchema, 1., Unit::Length);
        //editor->setSuffix(QString::fromUtf8(trRes.strUnit));
        editor->setDecimals(appModule->properties()->unitSystemDecimals);
        editor->setButtonSymbols(QDoubleSpinBox::NoButtons);
        editor->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        QSizePolicy sp = editor->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        editor->setSizePolicy(sp);
        editor->setMinimumWidth(25);
        QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value) {
            const double f = trRes.factor;
            value = qFuzzyCompare(f, 1.) ? value : value * f;
            gp_XYZ coords = IProperty3dCoordsType::coords(property);
            switch (specCoord) {
            case Coord::X: coords.SetX(value); break;
            case Coord::Y: coords.SetY(value); break;
            case Coord::Z: coords.SetZ(value); break;
            }

            IProperty3dCoordsType::setCoords(property, coords);
        });
        return editor;
    }

    PropertyCoordsType* m_property;
    QDoubleSpinBox* m_xCoordEditor = nullptr;
    QDoubleSpinBox* m_yCoordEditor = nullptr;
    QDoubleSpinBox* m_zCoordEditor = nullptr;
};

struct PropertyQuantityEditor : public InterfacePropertyEditor, public QDoubleSpinBox {
    PropertyQuantityEditor(BasePropertyQuantity* property, QWidget* parentWidget)
        : QDoubleSpinBox(parentWidget), m_property(property)
    {
        const UnitSystem::TranslateResult trRes = IPropertyEditorFactory::unitTranslate(property);
        this->setSuffix(QString::fromUtf8(trRes.strUnit));
        this->setDecimals(AppModule::get()->properties()->unitSystemDecimals.value());
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
        const UnitSystem::TranslateResult trRes = IPropertyEditorFactory::unitTranslate(m_property);
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

    if (propTypeName == PropertyString::TypeName)
        editor = new PropertyStringEditor(static_cast<PropertyString*>(property), parentWidget);

    if (propTypeName == PropertyOccColor::TypeName)
        editor = new PropertyOccColorEditor(static_cast<PropertyOccColor*>(property), parentWidget);

    if (propTypeName == PropertyOccPnt::TypeName)
        editor = new Property3dCoordsEditor<PropertyOccPnt>(static_cast<PropertyOccPnt*>(property), parentWidget);

    if (propTypeName == PropertyOccVec::TypeName)
        editor = new Property3dCoordsEditor<PropertyOccVec>(static_cast<PropertyOccVec*>(property), parentWidget);

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

UnitSystem::TranslateResult IPropertyEditorFactory::unitTranslate(const BasePropertyQuantity* property)
{
    if (!property)
        return {};

    if (property->quantityUnit() == Unit::Angle) {
        auto propAngle = static_cast<const PropertyAngle*>(property);
        return UnitSystem::degrees(propAngle->quantity());
    }

    return UnitSystem::translate(
                AppModule::get()->properties()->unitSystemSchema,
                property->quantityValue(),
                property->quantityUnit()
    );
}

QPixmap IPropertyEditorFactory::colorSquarePixmap(const QColor& c, int sideLen)
{
    QPixmap pixColor(sideLen, sideLen);
    pixColor.fill(c);
    return pixColor;
}

} // namespace Mayo

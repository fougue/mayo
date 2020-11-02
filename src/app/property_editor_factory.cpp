/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_editor_factory.h"

#include "../base/application.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/string_utils.h"
#include "../base/unit_system.h"
#include "app_module.h"

#include <fougtools/occtools/qt_utils.h>
#include <fougtools/qttools/gui/qwidget_utils.h>

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

namespace Mayo {

class PropertyEditorI18N { Q_DECLARE_TR_FUNCTIONS(PropertyEditorI18N) };

namespace {

static QWidget* hSpacerWidget(QWidget* parent, int stretch = 1)
{
    auto widget = new QWidget(parent);
    QSizePolicy sp = widget->sizePolicy();
    sp.setHorizontalStretch(stretch);
    widget->setSizePolicy(sp);
    return widget;
}

static QString stringYesNo(bool on)
{
    return on ? PropertyEditorI18N::tr("Yes") : PropertyEditorI18N::tr("No");
}

static bool handlePropertyValidationError(
        const Result<void>& resultValidation,
        QWidget* propertyEditor,
        const std::function<void()>& fnCallbackError = nullptr)
{
    if (!resultValidation) {
        qtgui::QWidgetUtils::asyncMsgBoxCritical(
                    propertyEditor->parentWidget(),
                    PropertyEditorI18N::tr("Error"),
                    resultValidation.errorText());
        QSignalBlocker sigBlock(propertyEditor); Q_UNUSED(sigBlock);
        if (fnCallbackError)
            fnCallbackError();
    }

    return resultValidation.valid();
}

static QWidget* createPropertyEditor(BasePropertyQuantity* property, QWidget* parentWidget)
{
    auto editor = new QDoubleSpinBox(parentWidget);
    const UnitSystem::TranslateResult trRes = PropertyEditorFactory::unitTranslate(property);
    editor->setSuffix(QString::fromUtf8(trRes.strUnit));
    editor->setDecimals(AppModule::get(Application::instance())->unitSystemDecimals.value());
    const double rangeMin =
            property->constraintsEnabled() ?
                property->minimum() : std::numeric_limits<double>::min();
    const double rangeMax =
            property->constraintsEnabled() ?
                property->maximum() : std::numeric_limits<double>::max();
    editor->setRange(rangeMin, rangeMax);
    editor->setValue(trRes.value);
    QObject::connect(editor, &QDoubleSpinBox::editingFinished, [=]{
        double value = editor->value();
        const double f = trRes.factor;
        value = qFuzzyCompare(f, 1.) ? value : value * f;
        if (!qFuzzyCompare(property->quantityValue(), value)) {
            const Result result = property->setQuantityValue(value);
            handlePropertyValidationError(result, editor, [=]{
                editor->setValue(trRes.value);
            });
        }
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyBool* property, QWidget* parentWidget)
{
    auto editor = new QCheckBox(parentWidget);
    editor->setText(stringYesNo(property->value()));
    editor->setChecked(property->value());
    QObject::connect(editor, &QCheckBox::toggled, [=](bool on) {
        const Result<void> result = property->setValue(on);
        handlePropertyValidationError(result, editor, [=]{
            editor->setChecked(property->value());
        });
        editor->setText(stringYesNo(property->value()));
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyInt* property, QWidget* parentWidget)
{
    auto editor = new QSpinBox(parentWidget);
    if (property->constraintsEnabled()) {
        editor->setRange(property->minimum(), property->maximum());
        editor->setSingleStep(property->singleStep());
    }

    editor->setValue(property->value());
    QObject::connect(editor, qOverload<int>(&QSpinBox::valueChanged), [=](int val) {
        const Result<void> result = property->setValue(val);
        handlePropertyValidationError(result, editor, [=]{ editor->setValue(val); });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyDouble* property, QWidget* parentWidget)
{
    auto editor = new QDoubleSpinBox(parentWidget);
    if (property->constraintsEnabled()) {
        editor->setRange(property->minimum(), property->maximum());
        editor->setSingleStep(property->singleStep());
    }
    editor->setValue(property->value());
    editor->setDecimals(AppModule::get(Application::instance())->unitSystemDecimals.value());
    QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double val) {
        const Result<void> result = property->setValue(val);
        handlePropertyValidationError(result, editor, [=]{ editor->setValue(val); });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyQString* property, QWidget* parentWidget)
{
    auto editor = new QLineEdit(parentWidget);
    editor->setText(property->value());
    QObject::connect(editor, &QLineEdit::textChanged, [=](const QString& text) {
        const Result<void> result = property->setValue(text);
        handlePropertyValidationError(result, editor, [=]{ editor->setText(text); });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyEnumeration* property, QWidget* parentWidget)
{
    auto editor = new QComboBox(parentWidget);
    const Enumeration* enumDef = property->enumeration();
    if (!enumDef)
        return editor;

    for (const Enumeration::Item& enumItem : enumDef->items())
        editor->addItem(enumItem.name.tr(), enumItem.value);

    editor->setCurrentIndex(editor->findData(property->value()));
    QObject::connect(editor, qOverload<int>(&QComboBox::activated), [=](int index) {
        const Result<void> result = property->setValue(editor->itemData(index).toInt());
        const int indexComboBox = editor->findData(property->value());
        handlePropertyValidationError(result, editor, [=]{ editor->setCurrentIndex(indexComboBox); });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyOccColor* property, QWidget* parentWidget)
{
    auto frame = new QWidget(parentWidget);
    auto frameLayout = new QHBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    auto labelColor = new QLabel(frame);
    const QColor inputColor = occ::QtUtils::toQColor(property->value());
    labelColor->setPixmap(PropertyEditorFactory::colorSquarePixmap(inputColor));

    auto labelRgb = new QLabel(frame);
    labelRgb->setText(StringUtils::text(property->value()));

    auto btnColor = new QToolButton(frame);
    btnColor->setText("...");
    btnColor->setToolTip(PropertyEditorI18N::tr("Choose color ..."));
    QObject::connect(btnColor, &QAbstractButton::clicked, [=]{
        auto dlg = new QColorDialog(frame);
        dlg->setCurrentColor(inputColor);
        QObject::connect(dlg, &QColorDialog::colorSelected, [=](const QColor& c) {
            property->setValue(occ::QtUtils::toOccColor(c));
            labelColor->setPixmap(PropertyEditorFactory::colorSquarePixmap(c));
        });
        qtgui::QWidgetUtils::asyncDialogExec(dlg);
    });

    frameLayout->addWidget(labelColor);
    frameLayout->addWidget(labelRgb);
    frameLayout->addWidget(btnColor);
    frameLayout->addWidget(hSpacerWidget(frame));
    return frame;
}

static QDoubleSpinBox* createOccPntCoordEditor(
        QWidget* parentWidget,
        PropertyOccPnt* property,
        double (gp_Pnt::*funcGetCoord)() const,
        void (gp_Pnt::*funcSetCoord)(double))
{
    auto editor = new QDoubleSpinBox(parentWidget);
    const QuantityLength lenCoord = ((property->value()).*funcGetCoord)() * Quantity_Millimeter;
    const UnitSystem::TranslateResult trRes =
            UnitSystem::translate(
                AppModule::get(Application::instance())->unitSystemSchema.valueAs<UnitSystem::Schema>(),
                lenCoord);
    //editor->setSuffix(QString::fromUtf8(trRes.strUnit));
    editor->setDecimals(AppModule::get(Application::instance())->unitSystemDecimals.value());
    editor->setButtonSymbols(QDoubleSpinBox::NoButtons);
    editor->setRange(std::numeric_limits<double>::min(),
                     std::numeric_limits<double>::max());
    editor->setValue(trRes.value);
    QSizePolicy sp = editor->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Expanding);
    editor->setSizePolicy(sp);
    editor->setMinimumWidth(25);
    QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value) {
        const double f = trRes.factor;
        value = qFuzzyCompare(f, 1.) ? value : value * f;
        gp_Pnt pnt = property->value();
        (pnt.*funcSetCoord)(value);
        const Result<void> result = property->setValue(pnt);
        handlePropertyValidationError(result, editor);
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyOccPnt* prop, QWidget* parentWidget)
{
    auto frame = new QWidget(parentWidget);
    auto frameLayout = new QHBoxLayout(frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(new QLabel("X", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::X, &gp_Pnt::SetX));
    frameLayout->addWidget(new QLabel("Y", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::Y, &gp_Pnt::SetY));
    frameLayout->addWidget(new QLabel("Z", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::Z, &gp_Pnt::SetZ));
    return frame;
}

} // namespace

QWidget* DefaultPropertyEditorFactory::createEditor(Property* property, QWidget* parentWidget) const
{
    if (!property)
        return nullptr;

    const char* propTypeName = property->dynTypeName();
    if (propTypeName == PropertyBool::TypeName)
        return createPropertyEditor(static_cast<PropertyBool*>(property), parentWidget);

    if (propTypeName == PropertyInt::TypeName)
        return createPropertyEditor(static_cast<PropertyInt*>(property), parentWidget);

    if (propTypeName == PropertyDouble::TypeName)
        return createPropertyEditor(static_cast<PropertyDouble*>(property), parentWidget);

    if (propTypeName == PropertyQString::TypeName)
        return createPropertyEditor(static_cast<PropertyQString*>(property), parentWidget);

    if (propTypeName == PropertyOccColor::TypeName)
        return createPropertyEditor(static_cast<PropertyOccColor*>(property), parentWidget);

    if (propTypeName == PropertyOccPnt::TypeName)
        return createPropertyEditor(static_cast<PropertyOccPnt*>(property), parentWidget);

    if (propTypeName == PropertyEnumeration::TypeName)
        return createPropertyEditor(static_cast<PropertyEnumeration*>(property), parentWidget);

    if (propTypeName == BasePropertyQuantity::TypeName)
        return createPropertyEditor(static_cast<BasePropertyQuantity*>(property), parentWidget);

    return nullptr;
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
                AppModule::get(Application::instance())->unitSystemSchema.valueAs<UnitSystem::Schema>(),
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

/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "widget_document_item_props.h"

#include "document_item.h"
#include "gui_application.h"
#include "gui_document.h"
#include "gpx_document_item.h"
#include "options.h"
#include "string_utils.h"
#include "unit_system.h"
#include "ui_widget_document_item_props.h"
#include "fougtools/occtools/qt_utils.h"
#include <QtVariantPropertyManager>

namespace Mayo {

namespace Internal {

template<typename T>
void setQtPropertyScalarConstraints(
        QtVariantProperty* qtProp, const PropertyScalarConstraints<T>* cnts)
{
    if (cnts->constraintsEnabled()) {
        qtProp->setAttribute("minimum", cnts->minimum());
        qtProp->setAttribute("maximum", cnts->maximum());
        qtProp->setAttribute("singleStep", cnts->singleStep());
    }
}

template<typename PROPERTY> struct PropertyHelper {};

template<> struct PropertyHelper<PropertyBool> {
    using PropertyType = PropertyBool;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::Bool; }
    static void init(QtVariantProperty*, const PropertyType*) {}
    static QVariant toQVariant(const PropertyType* prop) { return prop->value(); }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return qvariant_cast<ValueType>(value);
    }
};

template<> struct PropertyHelper<PropertyInt> {
    using PropertyType = PropertyInt;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::Int; }
    static void init(QtVariantProperty* qtProp, const PropertyType* prop) {
        setQtPropertyScalarConstraints(qtProp, prop);
    }
    static QVariant toQVariant(const PropertyType* prop) { return prop->value(); }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return qvariant_cast<ValueType>(value);
    }
};

template<> struct PropertyHelper<PropertyDouble> {
    using PropertyType = PropertyDouble;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::Double; }
    static void init(QtVariantProperty* qtProp, const PropertyType* prop) {
        setQtPropertyScalarConstraints(qtProp, prop);
    }
    static QVariant toQVariant(const PropertyType* prop) { return prop->value(); }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return qvariant_cast<ValueType>(value);
    }
};

template<> struct PropertyHelper<PropertyOccColor> {
    using PropertyType = PropertyOccColor;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::Color; }
    static void init(QtVariantProperty*, const PropertyType*) {}
    static QVariant toQVariant(const PropertyType* prop) {
        return occ::QtUtils::toQColor(prop->value());
    }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return occ::QtUtils::toOccColor(qvariant_cast<QColor>(value));
    }
};

template<> struct PropertyHelper<PropertyOccPnt> {
    using PropertyType = PropertyOccPnt;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::String; }
    static void init(QtVariantProperty*, const PropertyType*) {}
    static QVariant toQVariant(const PropertyType* prop) {
        return StringUtils::text(
                    prop->value(), Options::instance()->unitSystemSchema());
    }
    static ValueType toPropertyValue(const QVariant&, const PropertyType*) {
        return gp_Pnt(); // TODO
    }
};

template<> struct PropertyHelper<PropertyOccTrsf> {
    using PropertyType = PropertyOccTrsf;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::String; }
    static void init(QtVariantProperty*, const PropertyType*) {}
    static QVariant toQVariant(const PropertyType* prop) {
        return StringUtils::text(
                    prop->value(), Options::instance()->unitSystemSchema());
    }
    static ValueType toPropertyValue(const QVariant&, const PropertyType*) {
        return gp_Trsf(); // TODO
    }
};

template<> struct PropertyHelper<PropertyQString> {
    using PropertyType = PropertyQString;
    using ValueType = PropertyType::ValueType;

    static int qVariantTypeId() { return QVariant::String; }
    static void init(QtVariantProperty*, const PropertyType*) {}
    static QVariant toQVariant(const PropertyType* prop) { return prop->value(); }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return qvariant_cast<ValueType>(value);
    }
};

template<> struct PropertyHelper<PropertyEnumeration> {
    using PropertyType = PropertyEnumeration;
    using ValueType = Enumeration::Value;

    static int qVariantTypeId() { return QtVariantPropertyManager::enumTypeId(); }
    static void init(QtVariantProperty* qtProp, const PropertyType* prop) {
        QStringList enumNames;
        const auto& enumMappings = prop->enumeration().mappings();
        for (const Enumeration::Mapping& mapping : enumMappings)
            enumNames.push_back(mapping.string);
        qtProp->setAttribute("enumNames", enumNames);
    }
    static QVariant toQVariant(const PropertyType* prop) {
        const auto& enumMappings = prop->enumeration().mappings();
        auto itEnum =
                std::find_if(
                    enumMappings.cbegin(),
                    enumMappings.cend(),
                    [=](const Enumeration::Mapping& mapping) {
            return mapping.value == prop->value();
        });
        if (itEnum != enumMappings.cend())
            return itEnum - enumMappings.cbegin();
        return -1;
    }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType* prop) {
        const int enumValueId = value.toInt();
        return prop->enumeration().mapping(enumValueId).value;
    }
};

template<Unit UNIT> struct PropertyQuantityHelper {
    using PropertyType = GenericPropertyQuantity<UNIT>;
    using ValueType = typename PropertyType::QuantityType;

    static int qVariantTypeId() { return QVariant::Double; }
    static void init(QtVariantProperty* qtProp, const PropertyType* prop) {
        const UnitSystem::TranslateResult res =
                Options::unitSystemTranslate(prop->quantity());
        qtProp->setAttribute("suffix", QString::fromUtf8(res.strUnit));
        qtProp->setAttribute("decimals", Options::instance()->unitSystemDecimals());
        qtProp->setValue(res.value);
    }
    static QVariant toQVariant(const PropertyType* prop) {
        return Options::unitSystemTranslate(prop->quantity()).value;
    }
    static ValueType toPropertyValue(const QVariant& value, const PropertyType*) {
        return value.toDouble(); // TODO This is broken
    }
};

template<> struct PropertyHelper<PropertyLength> : public PropertyQuantityHelper<Unit::Length> {};
template<> struct PropertyHelper<PropertyArea> : public PropertyQuantityHelper<Unit::Area> {};
template<> struct PropertyHelper<PropertyVolume> : public PropertyQuantityHelper<Unit::Volume> {};
template<> struct PropertyHelper<PropertyMass> : public PropertyQuantityHelper<Unit::Mass> {};
template<> struct PropertyHelper<PropertyTime> : public PropertyQuantityHelper<Unit::Time> {};
template<> struct PropertyHelper<PropertyAngle> : public PropertyQuantityHelper<Unit::Angle> {};
template<> struct PropertyHelper<PropertyVelocity> : public PropertyQuantityHelper<Unit::Velocity> {};

// --
// -- Generic API
// --

template<typename PROPERTY>
QtVariantProperty* createQtProperty(
        const Property* prop, QtVariantPropertyManager* varPropMgr)
{
    auto castedProp = static_cast<const PROPERTY*>(prop);
    QtVariantProperty* qtProp =
            varPropMgr->addProperty(
                PropertyHelper<PROPERTY>::qVariantTypeId(),
                prop->label());
    PropertyHelper<PROPERTY>::init(qtProp, castedProp);
    qtProp->setValue(PropertyHelper<PROPERTY>::toQVariant(castedProp));
    return qtProp;
}

template<typename PROPERTY>
void setPropertyValue(Property* prop, const QVariant& value)
{
    auto castedProp = static_cast<PROPERTY*>(prop);
    castedProp->setValue(
                PropertyHelper<PROPERTY>::toPropertyValue(value, castedProp));
}

} // namespace Internal

WidgetDocumentItemProps::WidgetDocumentItemProps(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetDocumentItemProps),
      m_varPropMgr(new QtVariantPropertyManager(this))
{
    m_ui->setupUi(this);
    auto variantEditorFactory = new QtVariantEditorFactory(this);
    m_ui->propsBrowser_DocumentItem->setFactoryForManager(
                m_varPropMgr, variantEditorFactory);
    m_ui->propsBrowser_DocumentItem->setResizeMode(
                QtTreePropertyBrowser::ResizeToContents);
    m_ui->propsBrowser_DocumentItem->setIndentation(15);

    QObject::connect(
                Options::instance(), &Options::unitSystemSchemaChanged,
                this, &WidgetDocumentItemProps::refreshAllQtProperties);
    QObject::connect(
                Options::instance(), &Options::unitSystemDecimalsChanged,
                this, &WidgetDocumentItemProps::refreshAllQtProperties);
}

WidgetDocumentItemProps::~WidgetDocumentItemProps()
{
    delete m_ui;
}

void WidgetDocumentItemProps::setGuiApplication(GuiApplication *guiApp)
{
    m_guiApp = guiApp;
}

void WidgetDocumentItemProps::editDocumentItem(DocumentItem *docItem)
{
    m_currentVecHndProperty.clear();
    if (docItem != nullptr) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        m_currentDocItem = docItem;
        const GuiDocument* guiDoc =
                m_guiApp->findGuiDocument(m_currentDocItem->document());
        m_currentGpxDocItem = guiDoc->findItemGpx(m_currentDocItem);
        this->refreshAllQtProperties();
    }
    else {
        this->connectPropertyValueChangeSignals(false);
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
        m_currentDocItem = nullptr;
        m_currentGpxDocItem = nullptr;
    }
}

void WidgetDocumentItemProps::editProperties(Span<HandleProperty> spanHndProp)
{
    m_currentDocItem = nullptr;
    m_currentGpxDocItem = nullptr;
    m_currentVecHndProperty.clear();
    for (HandleProperty& hndProp : spanHndProp)
        m_currentVecHndProperty.push_back(std::move(hndProp));
    if (!m_currentVecHndProperty.empty()) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        this->refreshAllQtProperties();
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
    }
}

void WidgetDocumentItemProps::connectPropertyValueChangeSignals(bool on)
{
    if (on) {
        QObject::connect(
                    m_varPropMgr, &QtVariantPropertyManager::valueChanged,
                    this, &WidgetDocumentItemProps::onQVariantPropertyValueChanged,
                    Qt::UniqueConnection);
    }
    else {
        QObject::disconnect(
                    m_varPropMgr, &QtVariantPropertyManager::valueChanged,
                    this, &WidgetDocumentItemProps::onQVariantPropertyValueChanged);
    }
}

void WidgetDocumentItemProps::onQVariantPropertyValueChanged(
        QtProperty *qtProp, const QVariant &value)
{
    if (m_currentDocItem == nullptr && m_currentGpxDocItem == nullptr)
        return;

    auto itFound = std::find_if(
                m_vecQtPropProp.cbegin(),
                m_vecQtPropProp.cend(),
                [=](const QtProp_Prop& pair) { return pair.qtProp == qtProp; });
    if (itFound == m_vecQtPropProp.cend())
        return;

    Property* prop = itFound->prop;
    const char* strPropType = prop->dynTypeName();
    using FuncSetPropertyValue = void (*)(Property*, const QVariant&);
    using PropType_FuncSetPropertyValue = std::pair<const char*, FuncSetPropertyValue>;
    static const PropType_FuncSetPropertyValue arrayPair[] = {
        { PropertyOccColor::TypeName, &Internal::setPropertyValue<PropertyOccColor> },
        { PropertyEnumeration::TypeName, &Internal::setPropertyValue<PropertyEnumeration> },
        { PropertyBool::TypeName, &Internal::setPropertyValue<PropertyBool> },
        { PropertyInt::TypeName, &Internal::setPropertyValue<PropertyInt> },
        { PropertyDouble::TypeName, &Internal::setPropertyValue<PropertyDouble> },
        { PropertyQString::TypeName, &Internal::setPropertyValue<PropertyQString> }/*,
        { PropertyLength::TypeName, &Internal::setPropertyValue<PropertyLength> },
        { PropertyArea::TypeName, &Internal::setPropertyValue<PropertyArea> },
        { PropertyVolume::TypeName, &Internal::setPropertyValue<PropertyVolume> },
        { PropertyMass::TypeName, &Internal::setPropertyValue<PropertyMass> },
        { PropertyTime::TypeName, &Internal::setPropertyValue<PropertyTime> },
        { PropertyAngle::TypeName, &Internal::setPropertyValue<PropertyAngle> },
        { PropertyVelocity::TypeName, &Internal::setPropertyValue<PropertyVelocity> }*/
    };
    for (const PropType_FuncSetPropertyValue& pair : arrayPair) {
        const char* propDynTypeName = pair.first;
        const FuncSetPropertyValue funcSetPropValue = pair.second;
        if (std::strcmp(strPropType, propDynTypeName) == 0) {
            funcSetPropValue(prop, value);
            break;
        }
    }
}

void WidgetDocumentItemProps::createQtProperties(
        const std::vector<Property*>& properties, QtProperty *parentProp)
{
    for (Property* prop : properties)
        this->createQtProperty(prop, parentProp);
}

void WidgetDocumentItemProps::createQtProperty(
        Property *property, QtProperty *parentProp)
{
    using FuncCreateQtProperty =
        QtVariantProperty* (*)(const Property*, QtVariantPropertyManager*);
    using PropType_FuncCreateQtProp =
        std::pair<const char*, FuncCreateQtProperty>;
    static const PropType_FuncCreateQtProp arrayPair[] = {
        { PropertyOccColor::TypeName, &Internal::createQtProperty<PropertyOccColor> },
        { PropertyOccPnt::TypeName, &Internal::createQtProperty<PropertyOccPnt> },
        { PropertyOccTrsf::TypeName, &Internal::createQtProperty<PropertyOccTrsf> },
        { PropertyEnumeration::TypeName, &Internal::createQtProperty<PropertyEnumeration> },
        { PropertyBool::TypeName, &Internal::createQtProperty<PropertyBool> },
        { PropertyInt::TypeName, &Internal::createQtProperty<PropertyInt> },
        { PropertyDouble::TypeName, &Internal::createQtProperty<PropertyDouble> },
        { PropertyQString::TypeName, &Internal::createQtProperty<PropertyQString> },
        { PropertyLength::TypeName, &Internal::createQtProperty<PropertyLength> },
        { PropertyArea::TypeName, &Internal::createQtProperty<PropertyArea> },
        { PropertyVolume::TypeName, &Internal::createQtProperty<PropertyVolume> },
        { PropertyMass::TypeName, &Internal::createQtProperty<PropertyMass> },
        { PropertyTime::TypeName, &Internal::createQtProperty<PropertyTime> },
        { PropertyAngle::TypeName, &Internal::createQtProperty<PropertyAngle> },
        { PropertyVelocity::TypeName, &Internal::createQtProperty<PropertyVelocity> }
    };

    QtVariantProperty* qtProp = nullptr;
    const char* strPropType = property->dynTypeName();
    for (const PropType_FuncCreateQtProp& pair : arrayPair) {
        const char* propDynTypeName = pair.first;
        const FuncCreateQtProperty funcCreateQtProp = pair.second;
        if (std::strcmp(strPropType, propDynTypeName) == 0) {
            qtProp = funcCreateQtProp(property, m_varPropMgr);
            break;
        }
    }
    if (qtProp != nullptr) {
        parentProp->addSubProperty(qtProp);
        if (property->isUserReadOnly())
            qtProp->setAttribute(QLatin1String("readOnly"), true);
        foreach (QtBrowserItem* item, m_ui->propsBrowser_DocumentItem->items(qtProp))
            m_ui->propsBrowser_DocumentItem->setExpanded(item, false);
        this->mapProperty(qtProp, property);
    }
}

void WidgetDocumentItemProps::mapProperty(
        QtVariantProperty *qtProp, Property *prop)
{
    const QtProp_Prop pair = { qtProp, prop };
    m_vecQtPropProp.emplace_back(std::move(pair));
}

void WidgetDocumentItemProps::refreshAllQtProperties()
{
    m_varPropMgr->clear();
    m_ui->propsBrowser_DocumentItem->clear();
    m_vecQtPropProp.clear();

    // Data
    if (m_currentDocItem != nullptr) {
        this->connectPropertyValueChangeSignals(false);
        QtProperty* propGroupData =
                m_varPropMgr->addProperty(
                    QtVariantPropertyManager::groupTypeId(), tr("Data"));
        m_ui->propsBrowser_DocumentItem->addProperty(propGroupData);
        this->createQtProperties(m_currentDocItem->properties(), propGroupData);
        this->connectPropertyValueChangeSignals(true);
    }

    // Graphics
    if (m_currentGpxDocItem != nullptr) {
        this->connectPropertyValueChangeSignals(false);
        QtProperty* propGroupGpx =
                m_varPropMgr->addProperty(
                    QtVariantPropertyManager::groupTypeId(), tr("Graphics"));
        m_ui->propsBrowser_DocumentItem->addProperty(propGroupGpx);
        const GpxDocumentItem* gpxItem = m_currentGpxDocItem;
        this->createQtProperties(gpxItem->properties(), propGroupGpx);
        this->connectPropertyValueChangeSignals(true);
    }

    // "On-the-fly" properties
    if (!m_currentVecHndProperty.empty()) {
        QtProperty* propGroupData =
                m_varPropMgr->addProperty(
                    QtVariantPropertyManager::groupTypeId(), tr("Properties"));
        m_ui->propsBrowser_DocumentItem->addProperty(propGroupData);
        for (const HandleProperty& propHnd : m_currentVecHndProperty)
            this->createQtProperty(propHnd.get(), propGroupData);
    }
}

} // namespace Mayo

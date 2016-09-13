#include "widget_document_item_props.h"

#include "document_item.h"
#include "gui_application.h"
#include "gui_document.h"
#include "gpx_document_item.h"
#include "options.h"
#include "ui_widget_document_item_props.h"
#include "fougtools/occtools/qt_utils.h"

#include <QtVariantEditorFactory>
#include <QtVariantPropertyManager>

namespace Mayo {

namespace Internal {

struct EnumTag {};

// --
// -- PropTraits<>
// --
template<typename T> struct PropTraits {};

template<> struct PropTraits<bool> {
    static int qVariantTypeId() { return QVariant::Bool; }
    typedef bool QVariantType;
    typedef PropertyBool PropertyType;
};

template<> struct PropTraits<int> {
    static int qVariantTypeId() { return QVariant::Int; }
    typedef int QVariantType;
    typedef PropertyInt PropertyType;
};

template<> struct PropTraits<double> {
    static int qVariantTypeId() { return QVariant::Double; }
    typedef double QVariantType;
    typedef PropertyDouble PropertyType;
};

template<> struct PropTraits<QColor> {
    static int qVariantTypeId() { return QVariant::Color; }
    typedef QColor QVariantType;
    typedef PropertyOccColor PropertyType;
};

template<> struct PropTraits<QString> {
    static int qVariantTypeId() { return QVariant::String; }
    typedef QString QVariantType;
    typedef PropertyQString PropertyType;
};

template<> struct PropTraits<EnumTag> {
    static int qVariantTypeId() { return QtVariantPropertyManager::enumTypeId(); }
    typedef int QVariantType;
    typedef PropertyEnumeration PropertyType;
};

// --
// -- PropertyQVariantCast<>
// --
template<typename T> struct PropertyQVariantCast {
    typedef typename PropTraits<T>::QVariantType QVariantType;
    typedef typename PropTraits<T>::PropertyType PropertyType;

    static QVariantType toQVariantValue(const PropertyType* prop)
    { return qvariant_cast<QVariantType>(prop->value()); }

    static T toPropertyValue(const QVariant& value, const PropertyType*)
    { return qvariant_cast<T>(value); }
};

template<> struct PropertyQVariantCast<QColor> {
    static QColor toQVariantValue(const PropertyOccColor* prop)
    { return occ::QtUtils::toQColor(prop->value()); }

    static Quantity_Color toPropertyValue(
            const QVariant& value, const PropertyOccColor*)
    { return occ::QtUtils::toOccColor(qvariant_cast<QColor>(value)); }
};

template<> struct PropertyQVariantCast<EnumTag> {
    static int toQVariantValue(const PropertyEnumeration* prop)
    {
        const auto& enumMappings = prop->enumeration().mappings();
        auto itEnum =
                std::find_if(
                    enumMappings.cbegin(),
                    enumMappings.cend(),
                    [=](const Enumeration::Mapping& mapping) {
            return mapping.value == prop->value();
        } );
        if (itEnum != enumMappings.cend())
            return itEnum - enumMappings.cbegin();
        return -1;
    }

    static Enumeration::Value toPropertyValue(
            const QVariant& value, const PropertyEnumeration* prop)
    {
        const int enumValueId = value.toInt();
        return prop->enumeration().mapping(enumValueId).value;
    }
};

// --
// -- QtPropertyInit<>
// --

template<typename T> struct QtPropertyInit {
    static void func(QtVariantProperty*, const Property*) {}
};

template<typename T> struct QtPropertyInitConstraints {
    static void func(
            QtVariantProperty* qtProp, const PropertyScalarConstraints<T>* cnts)
    {
        if (cnts->constraintsEnabled()) {
            qtProp->setAttribute(QLatin1String("minimum"), cnts->minimum());
            qtProp->setAttribute(QLatin1String("maximum"), cnts->maximum());
            qtProp->setAttribute(QLatin1String("singleStep"), cnts->singleStep());
        }
    }
};
template<> struct QtPropertyInit<int> : public QtPropertyInitConstraints<int> {};
template<> struct QtPropertyInit<double> : public QtPropertyInitConstraints<double> {};

template<> struct QtPropertyInit<EnumTag> {
    static void func(QtVariantProperty* qtProp, const PropertyEnumeration* prop)
    {
        QStringList enumNames;
        const auto& enumMappings = prop->enumeration().mappings();
        for (const Enumeration::Mapping& mapping : enumMappings)
            enumNames.push_back(mapping.string);
        qtProp->setAttribute(QLatin1String("enumNames"), enumNames);
    }
};

// --
// -- Generic API
// --


template<typename T>
static QtVariantProperty* createQtProperty(
        const Property* prop, QtVariantPropertyManager* varPropMgr)
{
    typedef typename PropTraits<T>::PropertyType PropertyType;
    auto castedProp = static_cast<const PropertyType*>(prop);
    QtVariantProperty* qtProp =
            varPropMgr->addProperty(
                PropTraits<T>::qVariantTypeId(), prop->label());
    QtPropertyInit<T>::func(qtProp, castedProp);
    qtProp->setValue(PropertyQVariantCast<T>::toQVariantValue(castedProp));
    return qtProp;
}

template<typename T> void setPropertyValue(Property* prop, const QVariant& value)
{
    typedef typename PropTraits<T>::PropertyType PropertyType;
    auto castedProp = static_cast<PropertyType*>(prop);
    castedProp->setValue(
                PropertyQVariantCast<T>::toPropertyValue(value, castedProp));
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
}

WidgetDocumentItemProps::~WidgetDocumentItemProps()
{
    delete m_ui;
}

void WidgetDocumentItemProps::setGuiApplication(GuiApplication *guiApp)
{
    m_guiApp = guiApp;
}

void WidgetDocumentItemProps::editDocumentItems(
        const std::vector<DocumentItem*>& vecDocItem)
{
    this->connectPropertyValueChangeSignals(false);
    if (vecDocItem.size() == 1) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        m_varPropMgr->clear();
        m_ui->propsBrowser_DocumentItem->clear();
        m_vecQtPropProp.clear();

        // Data
        m_currentDocItem = vecDocItem.front();
        {
            QtProperty* propGroupData =
                    m_varPropMgr->addProperty(
                        QtVariantPropertyManager::groupTypeId(), tr("Data"));
            m_ui->propsBrowser_DocumentItem->addProperty(propGroupData);
            Property* propLabel = &m_currentDocItem->propertyLabel;
            QtVariantProperty* qtPropLabel =
                    Internal::createQtProperty<QString>(propLabel, m_varPropMgr);
            propGroupData->addSubProperty(qtPropLabel);
            this->mapProperty(qtPropLabel, propLabel);
        }

        // Graphics
        const GuiDocument* guiDoc =
                m_guiApp->findGuiDocument(m_currentDocItem->document());
        m_currentGpxDocItem = guiDoc->findItemGpx(m_currentDocItem);
        if (m_currentGpxDocItem != nullptr) {
            QtProperty* propGroupGpx =
                    m_varPropMgr->addProperty(
                        QtVariantPropertyManager::groupTypeId(), tr("Graphics"));
            m_ui->propsBrowser_DocumentItem->addProperty(propGroupGpx);

            const GpxDocumentItem* gpxItem = m_currentGpxDocItem;
            for (Property* prop : gpxItem->properties()) {
                typedef QtVariantProperty* (*FuncCreateQtProperty)(
                            const Property*, QtVariantPropertyManager*);
                typedef std::pair<const char*, FuncCreateQtProperty>
                        PropType_FuncCreateQtProp;
                static const PropType_FuncCreateQtProp arrayPair[] = {
                    { Property::OccColorTypeName, &Internal::createQtProperty<QColor>},
                    { Property::EnumerationTypeName, &Internal::createQtProperty<Internal::EnumTag> },
                    { Property::BoolTypeName, &Internal::createQtProperty<bool> },
                    { Property::IntTypeName, &Internal::createQtProperty<int> },
                    { Property::DoubleTypeName, &Internal::createQtProperty<double> },
                    { Property::QStringTypeName, &Internal::createQtProperty<QString> }
                };
                QtVariantProperty* qtProp = nullptr;
                const char* strPropType = prop->dynTypeName();
                for (const PropType_FuncCreateQtProp& pair : arrayPair) {
                    const char* propDynTypeName = pair.first;
                    const FuncCreateQtProperty funcCreateQtProp = pair.second;
                    if (std::strcmp(strPropType, propDynTypeName) == 0) {
                        qtProp = funcCreateQtProp(prop, m_varPropMgr);
                        break;
                    }
                }

                if (qtProp != nullptr) {
                    propGroupGpx->addSubProperty(qtProp);
                    this->mapProperty(qtProp, prop);
                }
            }
        }
        this->connectPropertyValueChangeSignals(true);
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
        m_currentDocItem = nullptr;
        m_currentGpxDocItem = nullptr;
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
    if (m_currentGpxDocItem != nullptr) {
        auto itFound = std::find_if(
                    m_vecQtPropProp.cbegin(),
                    m_vecQtPropProp.cend(),
                    [=](const QtProp_Prop& pair) { return pair.qtProp == qtProp; });
        if (itFound != m_vecQtPropProp.cend()) {
            Property* prop = itFound->prop;
            const char* strPropType = prop->dynTypeName();
            typedef void (*FuncSetPropertyValue)(Property*, const QVariant&);
            typedef std::pair<const char*, FuncSetPropertyValue>
                    PropType_FuncSetPropertyValue;
            static const PropType_FuncSetPropertyValue arrayPair[] = {
                { Property::OccColorTypeName, &Internal::setPropertyValue<QColor>},
                { Property::EnumerationTypeName, &Internal::setPropertyValue<Internal::EnumTag> },
                { Property::BoolTypeName, &Internal::setPropertyValue<bool> },
                { Property::IntTypeName, &Internal::setPropertyValue<int> },
                { Property::DoubleTypeName, &Internal::setPropertyValue<double> },
                { Property::QStringTypeName, &Internal::setPropertyValue<QString> }
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
    }
}

void WidgetDocumentItemProps::mapProperty(
        QtVariantProperty *qtProp, Property *prop)
{
    const QtProp_Prop pair = { qtProp, prop };
    m_vecQtPropProp.emplace_back(std::move(pair));
}

} // namespace Mayo

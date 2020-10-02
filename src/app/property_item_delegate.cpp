/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_item_delegate.h"

namespace Mayo {

#if 0
PropertyItemDelegate::PropertyItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

bool PropertyItemDelegate::overridePropertyUnitTranslation(
        const BasePropertyQuantity *prop, PropertyItemDelegate::UnitTranslation unitTr)
{
    if (!prop || prop->quantityUnit() != unitTr.unit)
        return false;
    m_mapPropUnitTr.emplace(prop, unitTr);
    return true;
}

void PropertyItemDelegate::paint(
        QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.column() == 1) {
        const auto prop = qvariant_cast<Property*>(index.data());
        if (prop && prop->dynTypeName() == PropertyOccColor::TypeName) {
            auto propColor = static_cast<const PropertyOccColor*>(prop);
            painter->save();

            QApplication::style()->drawPrimitive(
                        QStyle::PE_PanelItemViewItem,
                        &option,
                        painter,
                        option.widget);

            const QColor color = occ::QtUtils::toQColor(propColor->value());
            const QPixmap pixColor = colorSquarePixmap(color, option.rect.height());
            painter->drawPixmap(option.rect.x(), option.rect.y(), pixColor);
            const QString strColor = propertyValueText(propColor);

            QRect labelRect = option.rect;
            labelRect.setX(option.rect.x() + pixColor.width() + 6);
            QApplication::style()->drawItemText(
                        painter,
                        labelRect,
                        Qt::AlignLeft | Qt::AlignVCenter,
                        option.palette,
                        option.state.testFlag(QStyle::State_Enabled),
                        strColor);

            painter->restore();
            return;
        }
    }

    QStyledItemDelegate::paint(painter, option, index);
    if (index.column() == 1 && !index.data().isNull()) {
        const auto prop = qvariant_cast<Property*>(index.data());
        if (!prop->isUserReadOnly()
                && option.state.testFlag(QStyle::State_Enabled)
                && option.state.testFlag(QStyle::State_MouseOver))
        {
            const QSize itemSize = this->sizeHint(option, index);
            const QSize pixItemSize = option.decorationSize * 0.75;
            const QPixmap pixEdit =
                    mayoTheme()->icon(Theme::Icon::Edit).pixmap(pixItemSize);
            painter->drawPixmap(
                        option.rect.x() + itemSize.width() + 4,
                        option.rect.y() + (itemSize.height() - pixItemSize.height()) / 2.,
                        pixEdit);
        }
    }
}

QString PropertyItemDelegate::displayText(const QVariant& value, const QLocale&) const
{
    if (value.type() == QVariant::String) {
        return value.toString();
    }
    else if (value.canConvert<Property*>()) {
        const auto prop = qvariant_cast<Property*>(value);
        //return propertyValueText(prop);
        const char* propTypeName = prop ? prop->dynTypeName() : "";
        if (propTypeName == PropertyBool::TypeName)
            return propertyValueText(static_cast<const PropertyBool*>(prop));
        if (propTypeName == PropertyInt::TypeName)
            return propertyValueText(static_cast<const PropertyInt*>(prop));
        if (propTypeName == PropertyDouble::TypeName)
            return propertyValueText(static_cast<const PropertyDouble*>(prop));
        if (propTypeName == PropertyQByteArray::TypeName)
            return propertyValueText(static_cast<const PropertyQByteArray*>(prop));
        if (propTypeName == PropertyQString::TypeName)
            return propertyValueText(static_cast<const PropertyQString*>(prop));
        if (propTypeName == PropertyQDateTime::TypeName)
            return propertyValueText(static_cast<const PropertyQDateTime*>(prop));
        if (propTypeName == PropertyOccColor::TypeName)
            return propertyValueText(static_cast<const PropertyOccColor*>(prop));
        if (propTypeName == PropertyOccPnt::TypeName)
            return propertyValueText(static_cast<const PropertyOccPnt*>(prop));
        if (propTypeName == PropertyOccTrsf::TypeName)
            return propertyValueText(static_cast<const PropertyOccTrsf*>(prop));
        if (propTypeName == PropertyEnumeration::TypeName)
            return propertyValueText(static_cast<const PropertyEnumeration*>(prop));
        if (propTypeName == BasePropertyQuantity::TypeName) {
            auto qtyProp = static_cast<const BasePropertyQuantity*>(prop);
            auto itFound = m_mapPropUnitTr.find(qtyProp);
            if (itFound != m_mapPropUnitTr.cend())
                return propertyValueText(qtyProp, itFound->second);
            else
                return propertyValueText(qtyProp);
        }
        return tr("ERROR no stringifier for property type '%1'").arg(propTypeName);
    }
    return QString();
}

QWidget* PropertyItemDelegate::createEditor(
        QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    if (index.column() == 0)
        return nullptr;
    auto prop = qvariant_cast<Property*>(index.data());
    if (!prop || prop->isUserReadOnly())
        return nullptr;
    const char* propTypeName = prop->dynTypeName();
    if (propTypeName == PropertyBool::TypeName)
        return createPropertyEditor(static_cast<PropertyBool*>(prop), parent);
    if (propTypeName == PropertyInt::TypeName)
        return createPropertyEditor(static_cast<PropertyInt*>(prop), parent);
    if (propTypeName == PropertyDouble::TypeName)
        return createPropertyEditor(static_cast<PropertyDouble*>(prop), parent);
    if (propTypeName == PropertyQString::TypeName)
        return createPropertyEditor(static_cast<PropertyQString*>(prop), parent);
    if (propTypeName == PropertyOccColor::TypeName)
        return createPropertyEditor(static_cast<PropertyOccColor*>(prop), parent);
    if (propTypeName == PropertyOccPnt::TypeName)
        return createPropertyEditor(static_cast<PropertyOccPnt*>(prop), parent);
    if (propTypeName == PropertyEnumeration::TypeName)
        return createPropertyEditor(static_cast<PropertyEnumeration*>(prop), parent);
    if (propTypeName == BasePropertyQuantity::TypeName)
        return createPropertyEditor(static_cast<BasePropertyQuantity*>(prop), parent);
    return nullptr;
}

void PropertyItemDelegate::setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const
{
    // Disable default behavior that sets item data(property is changed directly)
}

QSize PropertyItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(Qt::SizeHintRole).isNull())
        return QSize(baseSize.width(), m_rowHeightFactor * baseSize.height());
    return baseSize;
}
#endif

} // namespace Mayo

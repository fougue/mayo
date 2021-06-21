/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_value_conversion.h"

#include "property_builtins.h"
#include "property_enumeration.h"
#include "string_conv.h"
#include "tkernel_utils.h"
#include "unit_system.h"

#include <QtCore/QtDebug>
#include <charconv>
#include <sstream>

namespace Mayo {

QVariant PropertyValueConversion::toVariant(const Property& prop) const
{
    if (isType<PropertyBool>(prop)) {
        return constRef<PropertyBool>(prop).value();
    }
    else if (isType<PropertyInt>(prop)) {
        return constRef<PropertyInt>(prop).value();
    }
    else if (isType<PropertyDouble>(prop)) {
        return constRef<PropertyDouble>(prop).value();
    }
    else if (isType<PropertyCheckState>(prop)) {
        qCritical() << "toVariant() not yet implemented for PropertyCheckState";
    }
    else if (isType<PropertyString>(prop)) {
        return to_QString(constRef<PropertyString>(prop).value());
    }
    else if (isType<PropertyFilePath>(prop)) {
        return filepathTo<QString>(constRef<PropertyFilePath>(prop).value());
    }
    else if (isType<PropertyOccPnt>(prop)) {
        qCritical() << "toVariant() not yet implemented for PropertyOccPnt";
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        qCritical() << "toVariant() not yet implemented for PropertyOccTrsf";
    }
    else if (isType<PropertyOccColor>(prop)) {
        return to_QString(TKernelUtils::colorToHex(constRef<PropertyOccColor>(prop)));
    }
    else if (isType<PropertyEnumeration>(prop)) {
        return to_QString(constRef<PropertyEnumeration>(prop).name());
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        const auto& qtyProp = constRef<BasePropertyQuantity>(prop);
        const UnitSystem::TranslateResult trRes = UnitSystem::translate(
                    UnitSystem::SI, qtyProp.quantityValue(), qtyProp.quantityUnit());
        const QString strUnit = to_QString(trRes.strUnit);
#if __cpp_lib_to_chars
        char buff[64] = {};
        auto resToChars = std::to_chars(
                    std::begin(buff),
                    std::end(buff),
                    trRes.value * trRes.factor,
                    std::chars_format::general,
                    m_doubleToStringPrecision);
        if (resToChars.ec == std::errc())
            return QString::fromUtf8(buff, resToChars.ptr - buff) + strUnit;
        else
            qCritical() << "toVariant(PropertyQuantity) failed with error: 'value_too_large'";
#else
        std::stringstream sstr;
        sstr.precision(m_doubleToStringPrecision);
        sstr << trRes.value * trRes.factor;
        return to_QString(sstr.str()) + strUnit;
#endif
    }

    return QVariant();
}

bool PropertyValueConversion::fromVariant(Property* prop, const QVariant& variant) const
{
    if (!prop)
        return false;

    auto fnError = [=](const QString& text) {
        qCritical() << QString("[property '%1'] %2").arg(QString::fromUtf8(prop->name().key), text);
        return false;
    };

    if (isType<PropertyBool>(prop)) {
        return ptr<PropertyBool>(prop)->setValue(variant.toBool());
    }
    else if (isType<PropertyInt>(prop)) {
        return ptr<PropertyInt>(prop)->setValue(variant.toInt());
    }
    else if (isType<PropertyDouble>(prop)) {
        return ptr<PropertyDouble>(prop)->setValue(variant.toDouble());
    }
    else if (isType<PropertyCheckState>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyCheckState");
    }
    else if (isType<PropertyString>(prop)) {
        return ptr<PropertyString>(prop)->setValue(to_stdString(variant.toString()));
    }
    else if (isType<PropertyFilePath>(prop)) {
        return ptr<PropertyFilePath>(prop)->setValue(filepathFrom(variant.toString()));
    }
    else if (isType<PropertyOccPnt>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyOccPnt");
    }
    else if (isType<PropertyOccTrsf>(prop)) {
        return fnError("fromVariant() not yet implemented for PropertyOccTrsf");
    }
    else if (isType<PropertyOccColor>(prop)) {
        auto strColorHex = to_stdString(variant.toString());
        Quantity_Color color;
        if (!TKernelUtils::colorFromHex(strColorHex, &color))
            return fnError(QString("Not hexadecimal format '%1'").arg(variant.toString()));

        return ptr<PropertyOccColor>(prop)->setValue(color);
    }
    else if (isType<PropertyEnumeration>(prop)) {
        const QByteArray name = variant.toByteArray();
        const Enumeration::Item* ptrItem = ptr<PropertyEnumeration>(prop)->enumeration().findItem(name);
        if (!ptrItem)
            return fnError(QString("fromVariant() found no enumeration item for '%1'").arg(variant.toString()));

        return ptr<PropertyEnumeration>(prop)->setValue(ptrItem->value);
    }
    else if (isType<BasePropertyQuantity>(prop)) {
        const std::string strQty = variant.toString().toStdString();
        Unit unit;
        const UnitSystem::TranslateResult trRes = UnitSystem::parseQuantity(strQty, &unit);
        if (!trRes.strUnit || qFuzzyIsNull(trRes.factor))
            return fnError(QString("fromVariant() failed to parse quantity string '%1'").arg(variant.toString()));

        if (unit != Unit::None && unit != ptr<BasePropertyQuantity>(prop)->quantityUnit())
            return fnError(QString("fromVariant() unit mismatch with quantity string '%1'").arg(variant.toString()));

        return ptr<BasePropertyQuantity>(prop)->setQuantityValue(trRes.value * trRes.factor);
    }

    return false;
}

} // namespace Mayo

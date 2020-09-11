/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include "result.h"

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <vector>

namespace Mayo {

class Property;

// TODO Rename to PropertyGroup
class PropertyOwner {
public:
    // TODO Rename to get() or items() ?
    Span<Property* const> properties() const;

protected:
    virtual void onPropertyChanged(Property* prop);
    virtual Result<void> isPropertyValid(const Property* prop) const;
    void blockPropertyChanged(bool on);
    bool isPropertyChangedBlocked() const;

    void addProperty(Property* prop);
    void removeProperty(Property* prop);

private:
    friend class Property;
    friend struct PropertyChangedBlocker;
    std::vector<Property*> m_properties;
    bool m_propertyChangedBlocked = false;
};

struct PropertyChangedBlocker {
    PropertyChangedBlocker(PropertyOwner* owner);
    ~PropertyChangedBlocker();
    PropertyOwner* const m_owner;
};

#define Mayo_PropertyChangedBlocker(owner) \
            Mayo::PropertyChangedBlocker __Mayo_PropertyChangedBlocker(owner); \
            Q_UNUSED(__Mayo_PropertyChangedBlocker);

class Property {
public:
    Property(PropertyOwner* owner, const QString& label);
    Property() = delete;
    Property(const Property&) = delete;
    Property(Property&&) = delete;
    Property& operator=(const Property&) = delete;
    Property& operator=(Property&&) = delete;
    virtual ~Property() = default;

    const QString& label() const;

    virtual QVariant valueAsVariant() const = 0;
    virtual Result<void> setValueFromVariant(const QVariant& value) = 0;

    bool isUserReadOnly() const;
    void setUserReadOnly(bool on);

    virtual const char* dynTypeName() const = 0;

protected:
    void notifyChanged();
    Result<void> isValid() const;
    bool hasOwner() const;

    template<typename T>
    static Result<void> setValueHelper(Property* prop, T* ptrValue, const T& newValue);

private:
    PropertyOwner* const m_owner = nullptr;
    const QString m_label;
    bool m_isUserReadOnly = false;
};

class PropertyOwnerSignals : public QObject, public PropertyOwner {
    Q_OBJECT
public:
    PropertyOwnerSignals(QObject* parent = nullptr);

signals:
    void propertyChanged(Property* prop);

protected:
    void onPropertyChanged(Property* prop) override;
};


// --
// -- Implementation
// --

template<typename T> Result<void> Property::setValueHelper(
        Property* prop, T* ptrValue, const T& newValue)
{
    Result<void> result = Result<void>::ok();
    if (prop->hasOwner()) {
        const T previousValue = *ptrValue;
        *ptrValue = newValue;
        result = prop->isValid();
        if (result.valid())
            prop->notifyChanged();
        else
            *ptrValue = previousValue;
    }
    else {
        *ptrValue = newValue;
        prop->notifyChanged();
    }

    return result;
}

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::Property*)
Q_DECLARE_METATYPE(const Mayo::Property*)

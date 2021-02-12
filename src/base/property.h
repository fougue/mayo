/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include "result.h"
#include "text_id.h"

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <vector>

namespace Mayo {

class Property;

class PropertyGroup {
public:
    PropertyGroup(PropertyGroup* parentGroup = nullptr);

    // TODO Rename to get() or items() ?
    Span<Property* const> properties() const;

    PropertyGroup* parentGroup() const { return m_parentGroup; }

    virtual void restoreDefaults();

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
    PropertyGroup* m_parentGroup = nullptr;
    std::vector<Property*> m_properties; // TODO Replace by QVarLengthArray<Property*> ?
    bool m_propertyChangedBlocked = false;
};

struct PropertyChangedBlocker {
    PropertyChangedBlocker(PropertyGroup* group);
    ~PropertyChangedBlocker();
    PropertyGroup* const m_group = nullptr;
};

#define Mayo_PropertyChangedBlocker(group) \
            Mayo::PropertyChangedBlocker __Mayo_PropertyChangedBlocker(group); \
            Q_UNUSED(__Mayo_PropertyChangedBlocker);

class Property {
public:
    Property(PropertyGroup* group, const TextId& name);
    Property() = delete;
    Property(const Property&) = delete;
    Property(Property&&) = delete;
    Property& operator=(const Property&) = delete;
    Property& operator=(Property&&) = delete;
    virtual ~Property() = default;

    PropertyGroup* group() const { return m_group; }

    const TextId& name() const;
    QString label() const;

    virtual QVariant valueAsVariant() const = 0;
    virtual Result<void> setValueFromVariant(const QVariant& value) = 0; // TODO Remove use of Result<>

    const QString& description() const { return m_description; }
    void setDescription(const QString& text) { m_description = text; }

    bool isUserReadOnly() const { return m_isUserReadOnly; }
    void setUserReadOnly(bool on) { m_isUserReadOnly = on; }

    bool isUserVisible() const { return m_isUserVisible; }
    void setUserVisible(bool on) { m_isUserVisible = on; }

    virtual const char* dynTypeName() const = 0;

protected:
    void notifyChanged();
    Result<void> isValid() const;
    bool hasGroup() const;

    template<typename T>
    static Result<void> setValueHelper(Property* prop, T* ptrValue, const T& newValue);

private:
    PropertyGroup* const m_group = nullptr;
    const TextId m_name;
    QString m_description;
    bool m_isUserReadOnly = false;
    bool m_isUserVisible = true;
};

class PropertyGroupSignals : public QObject, public PropertyGroup {
    Q_OBJECT
public:
    PropertyGroupSignals(QObject* parent = nullptr);

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
    if (prop->hasGroup()) {
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

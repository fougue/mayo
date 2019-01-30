/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <vector>

namespace Mayo {

class Property;

class PropertyOwner {
public:
    // TODO change to computed properties, remove member m_properties
    Span<Property* const> properties() const;

protected:
    virtual void onPropertyChanged(Property* prop);
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

    bool isUserReadOnly() const;
    void setUserReadOnly(bool on);

    virtual const char* dynTypeName() const = 0;

protected:
    void notifyChanged();

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

class HandleProperty {
public:
    enum Storage {
        Pointer,
        Owner
    };

    HandleProperty() = default;
    HandleProperty(Property* prop, Storage storage);
    HandleProperty(HandleProperty&& other);
    HandleProperty(const HandleProperty&) = delete;
    ~HandleProperty();

    HandleProperty& operator=(HandleProperty&& other);
    HandleProperty& operator=(const HandleProperty&) = delete;

    Property* get() const;
    Property* operator->() const;
    Storage storage() const;

private:
    void swap(HandleProperty&& other);

    Property* m_prop = nullptr;
    Storage m_storage = Pointer;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::Property*)
Q_DECLARE_METATYPE(const Mayo::Property*)

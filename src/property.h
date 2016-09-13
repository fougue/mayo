#pragma once

#include <QtCore/QString>
#include <vector>

namespace Mayo {

class Property;
class PropertyEnumeration;

class PropertyOwner
{
public:
    const std::vector<Property*>& properties() const;

protected:
    virtual void onPropertyChanged(Property* prop);
    void blockPropertyChanged(bool on);
    bool isPropertyChangedBlocked() const;

    void addProperty(Property* prop);

    struct PropertyChangedBlocker {
        PropertyChangedBlocker(PropertyOwner* owner);
        ~PropertyChangedBlocker();
        PropertyOwner* const m_owner;
    };

private:
    friend class Property;
    friend struct PropertyChangedBlocker;
    std::vector<Property*> m_properties;
    bool m_propertyChangedBlocked = false;
};

#define Mayo_PropertyChangedBlocker(owner) \
            PropertyChangedBlocker __Mayo_PropertyChangedBlocker(owner); \
            Q_UNUSED(__Mayo_PropertyChangedBlocker);

class Property
{
public:
    Property(PropertyOwner* owner, const QString& label);
    Property() = delete;
    Property(const Property&) = delete;
    Property(Property&&) = delete;
    Property& operator=(const Property&) = delete;
    Property& operator=(Property&&) = delete;

    const QString& label() const;

    virtual const char* dynTypeName() const = 0;

    static const char BoolTypeName[];
    static const char IntTypeName[];
    static const char DoubleTypeName[];
    static const char QByteArrayTypeName[];
    static const char QStringTypeName[];
    static const char QDateTimeTypeName[];
    static const char EnumerationTypeName[];
    static const char OccColorTypeName[];

protected:
    void notifyChanged();

private:
    PropertyOwner* const m_owner = nullptr;
    const QString m_label;
};

} // namespace Mayo

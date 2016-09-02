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

#if 0
class Property
{
public:
    struct EnumeratedValue
    {
        EnumeratedValue() = default;
        EnumeratedValue(const QString& lbl, const QVariant& val);
        QString label;
        QVariant value;
    };

    const QString& label() const;

    const QVariant& value() const;
    void setValue(const QVariant& v);

    int valueType() const; // -> QVariant::Type+UserType
    bool isScalarValueType() const;

    bool hasScalarRange() const;
    const QVariant& scalarMinValue() const;
    const QVariant& scalarMaxValue() const;

    bool hasEnumeration() const;
    const std::vector<EnumeratedValue>& enumeration() const;
    const EnumeratedValue& enumValue() const;

protected:
    Property(PropertyOwner* owner, const QString& label, int valueType);
    Property(PropertyOwner* owner,
             const QString& label,
             int valueType,
             const std::pair<QVariant, QVariant>& scalarRange);
    Property(PropertyOwner* owner,
             const QString& label,
             int valueType,
             const std::vector<EnumeratedValue>& vecEnum);

    void notifyChanged();

private:
    PropertyOwner* m_owner = nullptr;
    const QString m_label;
    QVariant m_value;
    int m_valueType = -1;
    bool m_isScalarValueType = false;
    QVariant m_scalarMinValue;
    QVariant m_scalarMaxValue;
    std::vector<EnumeratedValue> m_enumeration;
};
#endif

} // namespace Mayo

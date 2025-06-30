/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "signal.h"
#include "span.h"
#include "text_id.h"

#include <vector>
#include <string_view>

namespace Mayo {

class Property;

// Provides a cohesive container of Property objects
// PropertyGroup defines callbacks to be executed when special events happen on contained properties:
//     - a property value is about to be changed
//     - a property value was changed
//     - the "enabled" status of a property was toggled
// A PropertyGroup can be linked to a parent group(optional). In such case the child group executes
// as well the parent group's callbacks.
class PropertyGroup {
public:
    PropertyGroup(PropertyGroup* parentGroup = nullptr);
    virtual ~PropertyGroup() = default;

    // TODO Rename to get() or items() ?
    Span<Property* const> properties() const;

    PropertyGroup* parentGroup() const { return m_parentGroup; }

    // Reinitialize properties to their default values
    virtual void restoreDefaults();

    Signal<Property*> signalPropertyAboutToChange;
    Signal<Property*> signalPropertyChanged;
    Signal<Property*, bool> signalPropertyEnabled;

protected:
    // Callback executed when Property value is about to change
    virtual void onPropertyAboutToChange(Property* prop);

    // Callback executed when Property value was changed
    virtual void onPropertyChanged(Property* prop);

    // Callback executed when Property "enabled" status was changed
    virtual void onPropertyEnabled(Property* prop, bool on);

    virtual bool isPropertyValid(const Property* prop) const;

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

// Exception-safe wrapper around PropertyGroup::blockPropertyChanged()
// It blocks call to PropertyGroup::onPropertyChanged() in its constructor and in the destructor it
// resets the state to what it was before the constructor ran.
struct PropertyChangedBlocker {
    PropertyChangedBlocker(PropertyGroup* group);
    ~PropertyChangedBlocker();
    PropertyGroup* const m_group = nullptr;
};

#define Mayo_PropertyChangedBlocker(group) \
            [[maybe_unused]] Mayo::PropertyChangedBlocker __Mayo_PropertyChangedBlocker(group);

// Provides an abstract storage of a value with associated meta-data(name, description, ...)
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
    std::string_view label() const;

    const std::string& description() const { return m_description; }
    void setDescription(std::string_view text) { m_description = text; }

    bool isUserReadOnly() const { return m_isUserReadOnly; }
    void setUserReadOnly(bool on) { m_isUserReadOnly = on; }

    bool isUserVisible() const { return m_isUserVisible; }
    void setUserVisible(bool on) { m_isUserVisible = on; }

    bool isEnabled() const { return m_isEnabled; }
    void setEnabled(bool on);

    bool hasUserData() const { return m_hasUserData; }
    uint64_t userData() const;
    void setUserData(uint64_t d);
    void clearUserData();

    virtual const char* dynTypeName() const = 0;

protected:
    void notifyAboutToChange();
    void notifyChanged();
    void notifyEnabled(bool on);

    bool isValid() const;

    bool hasGroup() const;

    template<typename T>
    static bool setValueHelper(Property* prop, T* ptrValue, const T& newValue);

private:
    PropertyGroup* const m_group = nullptr;
    const TextId m_name;
    std::string m_description;
    bool m_isUserReadOnly = false;
    bool m_isUserVisible = true;
    bool m_isEnabled = true;
    uint64_t m_userData;
    bool m_hasUserData = false;
};

// --
// -- Implementation
// --

template<typename T> bool Property::setValueHelper(Property* prop, T* ptrValue, const T& newValue)
{
    bool okResult = true;
    if (prop->hasGroup()) {
        prop->notifyAboutToChange();
        const T previousValue = *ptrValue;
        *ptrValue = newValue;
        okResult = prop->isValid();
        if (okResult)
            prop->notifyChanged();
        else
            *ptrValue = previousValue;
    }
    else {
        *ptrValue = newValue;
        prop->notifyChanged();
    }

    return okResult;
}

} // namespace Mayo

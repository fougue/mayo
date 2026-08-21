/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "signal.h"
#include "text_id.h"

#include <gsl/span>
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
    explicit PropertyGroup(PropertyGroup* parentGroup = nullptr);
    virtual ~PropertyGroup() = default;

    // TODO Rename to get() or items() ?
    gsl::span<Property* const> properties() const;

    PropertyGroup* parentGroup() const { return m_parentGroup; }
    void setParentGroup(PropertyGroup* group) { m_parentGroup = group; }

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
    explicit PropertyChangedBlocker(PropertyGroup* group);
    ~PropertyChangedBlocker();
    PropertyGroup* const m_group = nullptr;
};

// Temporarily blocks property-changed notifications for the specified group.
// The blocker is automatically destroyed at the end of the current scope, restoring the group's
// previous notification state
// TODO Use __COUNTER__ to generated unique variable name ? Needs ugly macro concatenation
#define Mayo_PropertyChangedBlocker(group) \
    [[maybe_unused]] Mayo::PropertyChangedBlocker __Mayo_PropertyChangedBlocker(group)

// Describes the metadata associated with a Property
// PropertyMeta stores information that is independent from the property's value, it can therefore
// be shared by different Property instances when appropriate
class PropertyMeta {
public:
    explicit PropertyMeta(const TextId& name);

    // Returns the property's identifier
    const TextId& name() const { return m_name; }

    // Returns the property's translated descriptio
    std::string_view description() const { return m_description; }
    PropertyMeta& setDescription(const TextId& text);
    PropertyMeta& setDescription(std::string_view trText);

    // Returns whether the property is read-only from the user's perspective
    // This flag affects user interaction with the property but does not prevent the property from
    // being modified programmatically
    bool isUserReadOnly() const { return m_isUserReadOnly; }
    PropertyMeta& setUserReadOnly(bool on);

    // Returns whether the property should be visible to the user
    bool isUserVisible() const { return m_isUserVisible; }
    PropertyMeta& setUserVisible(bool on);

private:
    const TextId m_name;
    std::string m_description;
    bool m_isUserReadOnly = false;
    bool m_isUserVisible = true;
};

// Provides an abstract storage of a value with associated meta-data(name, description, ...)
class Property {
public:
    using MetaType = PropertyMeta;

    Property(PropertyGroup* group, const PropertyMeta& meta);
    Property() = delete;
    Property(const Property&) = delete;
    Property(Property&&) = delete;
    Property& operator=(const Property&) = delete;
    Property& operator=(Property&&) = delete;
    virtual ~Property() = default;

    PropertyGroup* group() const { return m_group; }

    const MetaType& meta() const { return m_meta; }

    const TextId& name() const { return m_meta.name(); }
    std::string_view description() const { return m_meta.description(); }

    bool isUserReadOnly() const { return m_meta.isUserReadOnly(); }
    bool isUserVisible() const { return m_meta.isUserVisible(); }

    bool isEnabled() const { return m_isEnabled; }
    void setEnabled(bool on);

    bool hasUserData() const { return m_hasUserData; }
    uint64_t userData() const;
    void setUserData(uint64_t d);
    void clearUserData();

    virtual bool copyValue(const Property& /*other*/) { return false; }

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
    const PropertyMeta& m_meta;
    PropertyGroup* const m_group = nullptr;
    bool m_isEnabled = true;
    uint64_t m_userData; // TODO std::optional<> ?
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

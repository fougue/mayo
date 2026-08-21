/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "property.h"

#include <type_traits>
#include <vector>

namespace Mayo {

template<typename ObjectType>
class PropertyBinding {
public:
    virtual ~PropertyBinding() = default;
    virtual bool loadFrom(const ObjectType& obj) = 0;
    virtual bool saveTo(ObjectType& obj) const = 0;
};

template<typename ObjectType> class PropertyBindingGroup;

template<typename PropertyType, typename ObjectType>
class BoundProperty : public PropertyType, public PropertyBinding<ObjectType>
{
public:
    using ValueType = typename PropertyType::ValueType;

    BoundProperty(
        PropertyBindingGroup<ObjectType>* group,
        const typename PropertyType::MetaType& meta,
        ValueType ObjectType::* member
    )
        : PropertyType(group, meta), m_member(member)
    {
        group->registerBinding(this);
    }

    bool loadFrom(const ObjectType& obj) override
    {
        return this->setValue(static_cast<ValueType>(obj.*m_member));
    }

    bool saveTo(ObjectType& obj) const override
    {
        obj.*m_member = this->value();
        return true;
    }

private:
    ValueType ObjectType::* m_member;
};

template<typename ObjectType>
class PropertyBindingGroup {
public:
    bool loadFrom(const ObjectType& params)
    {
        for (auto binding : m_bindings) {
            if (!binding->loadFrom(params))
                return false;
        }

        return true;
    }

    bool saveTo(ObjectType& params) const
    {
        for (auto binding : m_bindings) {
            if (!binding->saveTo(params))
                return false;
        }

        return true;
    }

protected:
    void registerBinding(PropertyBinding<ObjectType>* binding)
    {
        m_bindings.push_back(binding);
    }

private:
    std::vector<PropertyBinding<ObjectType>*> m_bindings;
};

} // namespace Mayo

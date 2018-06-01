/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "property.h"
#include <utility>
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Enumeration {
public:
    using Value = int;
    struct Mapping {
        Value value;
        QString string;
    };

    Enumeration() = default;

    void map(int eval, const QString& str);

    size_t size() const;
    size_t index(int eval) const;

    Value valueAt(size_t i) const;
    Value value(const QString& str) const;
    const QString& string(Value eval) const;

    Mapping mapping(size_t i) const;
    const std::vector<Mapping>& mappings() const;

private:
    std::vector<Mapping>::const_iterator findCppSql(Value eval) const;
    std::vector<Mapping> m_vecMapping;
};

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(
            PropertyOwner* owner,
            const QString& label,
            const Enumeration* enumeration);

    const Enumeration& enumeration() const;

    const QString& string() const;
    Enumeration::Value value() const;
    template<typename T> T valueAs() const;
    void setValue(Enumeration::Value v);

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    Enumeration::Value m_value;
    const Enumeration* m_enumeration;
};

const Enumeration& enum_Graphic3dNameOfMaterial();
const Enumeration& enum_AspectHatchStyle();



// -- Implementation

template<typename T> T PropertyEnumeration::valueAs() const
{ return static_cast<T>(m_value); }

} // namespace Mayo

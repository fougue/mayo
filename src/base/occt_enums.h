/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "property_enumeration.h"
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class OcctEnums {
public:
    static const Enumeration& Graphic3d_NameOfMaterial();
    static const Enumeration& Aspect_HatchStyle();
};

} // namespace Mayo

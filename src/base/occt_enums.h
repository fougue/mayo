/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_enumeration.h"
#include <QtCore/QCoreApplication>
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class OcctEnums {
    Q_DECLARE_TR_FUNCTIONS(Mayo::OcctEnums)
public:
    static const Enumeration& Graphic3d_NameOfMaterial();
    static const Enumeration& Aspect_HatchStyle();
};

} // namespace Mayo

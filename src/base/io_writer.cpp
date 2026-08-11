/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_writer.h"

#include "cpp_utils.h"
#include "property.h"

#include <utility>

namespace Mayo::IO {

PropertyGroup& Writer::parameters()
{
    return const_cast<PropertyGroup&>(std::as_const(*this).constParameters());
}

const PropertyGroup& Writer::constParameters() const
{
    return Cpp::staticObject<PropertyGroup>();
}

} // namespace Mayo::IO

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ.h"

#include "../base/io_format.h"
#include "../base/tkernel_utils.h"
#include "io_occ_brep.h"
#include "io_occ_iges.h"
#include "io_occ_step.h"
#include "io_occ_stl.h"
#include "io_occ_vrml_writer.h"

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include "io_occ_gltf_reader.h"
#  include "io_occ_obj_reader.h"
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include "io_occ_gltf_writer.h"
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
#  include "io_occ_obj_writer.h"
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 7, 0)
#  include "io_occ_vrml_reader.h"
#endif

namespace Mayo::IO {

namespace { using PtrPropertyGroup = std::unique_ptr<PropertyGroup>; }

Span<const Format> OccFactoryReader::formats() const
{
    static const Format arrayFormat[] = {
        Format_STEP, Format_IGES, Format_OCCBREP, Format_STL
    #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        , Format_GLTF, Format_OBJ
    #endif
    #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 7, 0)
        , Format_VRML
    #endif
    };
    return arrayFormat;
}

std::unique_ptr<Reader> OccFactoryReader::create(Format format) const
{
    if (format == Format_STEP)
        return std::make_unique<OccStepReader>();
    if (format == Format_IGES)
        return std::make_unique<OccIgesReader>();
    if (format == Format_OCCBREP)
        return std::make_unique<OccBRepReader>();
    if (format == Format_STL)
        return std::make_unique<OccStlReader>();

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    if (format == Format_GLTF)
        return std::make_unique<OccGltfReader>();
    if (format == Format_OBJ)
        return std::make_unique<OccObjReader>();
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 7, 0)
    if (format == Format_VRML)
        return std::make_unique<OccVrmlReader>();
#endif

    return {};
}

PtrPropertyGroup OccFactoryReader::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Format_STEP)
        return OccStepReader::createProperties(parentGroup);
    if (format == Format_IGES)
        return OccIgesReader::createProperties(parentGroup);

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    if (format == Format_GLTF)
        return OccGltfReader::createProperties(parentGroup);
    if (format == Format_OBJ)
        return OccObjReader::createProperties(parentGroup);
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 7, 0)
    if (format == Format_VRML)
        return OccVrmlReader::createProperties(parentGroup);
#endif

    return {};
}

Span<const Format> OccFactoryWriter::formats() const
{
    static const Format arrayFormat[] = {
        Format_STEP, Format_IGES, Format_OCCBREP, Format_STL, Format_VRML
    #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        , Format_GLTF
    #endif
    #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        , Format_OBJ
    #endif
    };
    return arrayFormat;
}

std::unique_ptr<Writer> OccFactoryWriter::create(Format format) const
{
    if (format == Format_STEP)
        return std::make_unique<OccStepWriter>();
    if (format == Format_IGES)
        return std::make_unique<OccIgesWriter>();
    if (format == Format_OCCBREP)
        return std::make_unique<OccBRepWriter>();
    if (format == Format_STL)
        return std::make_unique<OccStlWriter>();
    if (format == Format_VRML)
        return std::make_unique<OccVrmlWriter>();

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    if (format == Format_GLTF)
        return std::make_unique<OccGltfWriter>();
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    if (format == Format_OBJ)
        return std::make_unique<OccObjWriter>();
#endif

    return {};
}

PtrPropertyGroup OccFactoryWriter::createProperties(Format format, PropertyGroup* parentGroup) const
{
    if (format == Format_STEP)
        return OccStepWriter::createProperties(parentGroup);
    if (format == Format_IGES)
        return OccIgesWriter::createProperties(parentGroup);
    if (format == Format_STL)
        return OccStlWriter::createProperties(parentGroup);
    if (format == Format_VRML)
        return OccVrmlWriter::createProperties(parentGroup);

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    if (format == Format_GLTF)
        return OccGltfWriter::createProperties(parentGroup);
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    if (format == Format_OBJ)
        return OccObjWriter::createProperties(parentGroup);
#endif

    return {};
}

} // namespace Mayo::IO

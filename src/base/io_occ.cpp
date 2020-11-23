/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ.h"

#include "io_format.h"
#include "io_occ_brep.h"
#include "io_occ_iges.h"
#include "io_occ_step.h"
#include "io_occ_stl.h"
#include "io_occ_vrml.h"
#include "tkernel_utils.h"

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include "io_occ_gltf_reader.h"
#  include "io_occ_obj.h"
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include "io_occ_gltf_writer.h"
#endif

#include <functional>

namespace Mayo {
namespace IO {

namespace {

// Helper for the creation of readers/writers

template<typename KEY, typename PRODUCT, typename... ARGS>
struct Generator {
    using KeyType = KEY;
    using ProductType = PRODUCT;

    KEY key;
    std::function<PRODUCT (const ARGS&...)> fn;

    static const auto& null() {
        static const Generator<KEY, PRODUCT, ARGS...> object = {
            Format_Unknown, [](const ARGS&...) { return PRODUCT(); }
        };
        return object;
    }
};

template<typename GENERATOR>
const GENERATOR& findGenerator(
        const typename GENERATOR::KeyType& key, Span<const GENERATOR> spanGenerator)
{
    for (const GENERATOR& generator : spanGenerator) {
        if (generator.key == key)
            return generator;
    }

    return GENERATOR::null();
}

template<typename PRODUCT> std::unique_ptr<PRODUCT> createProduct() {
    return std::make_unique<PRODUCT>();
}

using ReaderGenerator = Generator<Format, std::unique_ptr<Reader>>;
using WriterGenerator = Generator<Format, std::unique_ptr<Writer>>;

using ReaderParametersGenerator = Generator<Format, std::unique_ptr<PropertyGroup>, PropertyGroup*>;
using WriterParametersGenerator = Generator<Format, std::unique_ptr<PropertyGroup>, PropertyGroup*>;

} // namespace

Span<const Format> OccFactoryReader::formats() const
{
    static const Format array[] = {
        Format_STEP,
        Format_IGES,
        Format_OCCBREP,
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        Format_GLTF,
        Format_OBJ,
#endif
        Format_STL
    };
    return array;
}

std::unique_ptr<Reader> OccFactoryReader::create(const Format& format) const
{
    static const ReaderGenerator array[] = {
        { Format_STEP, createProduct<OccStepReader> },
        { Format_IGES, createProduct<OccIgesReader> },
        { Format_OCCBREP, createProduct<OccBRepReader> },
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        { Format_GLTF, createProduct<OccGltfReader> },
        { Format_OBJ, createProduct<OccObjReader> },
#endif
        { Format_STL, createProduct<OccStlReader> }
    };
    return findGenerator<ReaderGenerator>(format, array).fn();
}

std::unique_ptr<PropertyGroup> OccFactoryReader::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    static const ReaderParametersGenerator array[] = {
        { Format_STEP, &OccStepReader::createProperties },
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        { Format_GLTF, &OccGltfReader::createProperties },
        { Format_OBJ, &OccObjReader::createProperties }
#endif
    };
    return findGenerator<ReaderParametersGenerator>(format, array).fn(parentGroup);
}

Span<const Format> OccFactoryWriter::formats() const
{
    static const Format array[] = {
        Format_STEP,
        Format_IGES,
        Format_OCCBREP,
        Format_STL,
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        Format_GLTF,
#endif
        Format_VRML
    };
    return array;
}

std::unique_ptr<Writer> OccFactoryWriter::create(const Format& format) const
{
    static const WriterGenerator array[] = {
        { Format_STEP, createProduct<OccStepWriter> },
        { Format_IGES, createProduct<OccIgesWriter> },
        { Format_OCCBREP, createProduct<OccBRepWriter> },
        { Format_STL, createProduct<OccStlWriter> },
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        { Format_GLTF, createProduct<OccGltfWriter> },
#endif
        { Format_VRML, createProduct<OccVrmlWriter> }
    };
    return findGenerator<WriterGenerator>(format, array).fn();
}

std::unique_ptr<PropertyGroup> OccFactoryWriter::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    static const WriterParametersGenerator array[] = {
        { Format_STEP, &OccStepWriter::createProperties },
        { Format_STL, &OccStlWriter::createProperties },
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        { Format_GLTF, &OccGltfWriter::createProperties },
#endif
        { Format_VRML, &OccVrmlWriter::createProperties }
    };
    return findGenerator<WriterParametersGenerator>(format, array).fn(parentGroup);
}

} // namespace IO
} // namespace Mayo

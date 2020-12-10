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
#include <type_traits>

namespace Mayo {
namespace IO {

namespace {

// Helper for the creation of readers/writers

template<typename KEY, typename PRODUCT, typename... ARGS>
struct Generator {
    using KeyType = KEY;
    using ProductType = PRODUCT;
    using FunctionType = std::function<PRODUCT (const ARGS&...)>;

    KEY key;
    FunctionType fn;

    static const auto& null() {
        static const Generator<KEY, PRODUCT, ARGS...> object = {
            Format_Unknown, [](const ARGS&...) { return PRODUCT(); }
        };
        return object;
    }
};

template<typename PRODUCT> std::unique_ptr<PRODUCT> createProduct() {
    return std::make_unique<PRODUCT>();
}

template<typename EXCHANGER>
class FactoryHelper {
public:
    using ExchangerGenerator = Generator<Format, std::unique_ptr<EXCHANGER>>;
    using PropertiesGenerator = Generator<Format, std::unique_ptr<PropertyGroup>, PropertyGroup*>;

    void addExchanger(
            const Format& format,
            const typename ExchangerGenerator::FunctionType& fnExchanger,
            const PropertiesGenerator::FunctionType& fnProperties)
    {
        m_vecFormat.push_back(format);
        m_vecExchangerGenerator.push_back({ format, fnExchanger });
        if (fnProperties)
            m_vecPropertiesGenerator.push_back({ format, fnProperties });
    }

    template<typename T> void addExchanger(const Format& format) {
        this->addExchanger(format, &createProduct<T>, &T::createProperties);
    }

    bool empty() const { return m_vecFormat.empty(); }
    Span<const Format> formats() const { return m_vecFormat; }

    typename ExchangerGenerator::ProductType createExchanger(const Format& format) const {
        return findGenerator<ExchangerGenerator>(format, m_vecExchangerGenerator).fn();
    }

    PropertiesGenerator::ProductType createProperties(
            const Format& format, PropertyGroup* parentGroup) const
    {
        return findGenerator<PropertiesGenerator>(format, m_vecPropertiesGenerator).fn(parentGroup);
    }

private:
    template<typename GENERATOR>
    static const GENERATOR& findGenerator(
            const typename GENERATOR::KeyType& key, Span<const GENERATOR> spanGenerator)
    {
        for (const GENERATOR& generator : spanGenerator) {
            if (generator.key == key)
                return generator;
        }

        return GENERATOR::null();
    }

    std::vector<Format> m_vecFormat;
    std::vector<ExchangerGenerator> m_vecExchangerGenerator;
    std::vector<PropertiesGenerator> m_vecPropertiesGenerator;
};

const FactoryHelper<Reader>& helper_OccFactoryReader()
{
    static FactoryHelper<Reader> helper;
    if (helper.empty()) {
        helper.addExchanger<OccStepReader>(Format_STEP);
        helper.addExchanger<OccIgesReader>(Format_IGES);
        helper.addExchanger(Format_OCCBREP, &createProduct<OccBRepReader>, nullptr);
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        helper.addExchanger<OccGltfReader>(Format_GLTF);
        helper.addExchanger<OccObjReader>(Format_OBJ);
#endif
        helper.addExchanger(Format_STL, &createProduct<OccStlReader>, nullptr);
    }

    return helper;
}

const FactoryHelper<Writer>& helper_OccFactoryWriter()
{
    static FactoryHelper<Writer> helper;
    if (helper.empty()) {
        helper.addExchanger<OccStepWriter>(Format_STEP);
        helper.addExchanger<OccIgesWriter>(Format_IGES);
        helper.addExchanger(Format_OCCBREP, &createProduct<OccBRepWriter>, nullptr);
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        helper.addExchanger<OccGltfWriter>(Format_GLTF);
#endif
        helper.addExchanger<OccStlWriter>(Format_STL);
        helper.addExchanger<OccVrmlWriter>(Format_VRML);
    }

    return helper;
}

} // namespace

Span<const Format> OccFactoryReader::formats() const
{
    return helper_OccFactoryReader().formats();
}

std::unique_ptr<Reader> OccFactoryReader::create(const Format& format) const
{
    return helper_OccFactoryReader().createExchanger(format);
}

std::unique_ptr<PropertyGroup> OccFactoryReader::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    return helper_OccFactoryReader().createProperties(format, parentGroup);
}

Span<const Format> OccFactoryWriter::formats() const
{
    return helper_OccFactoryWriter().formats();
}

std::unique_ptr<Writer> OccFactoryWriter::create(const Format& format) const
{
    return helper_OccFactoryWriter().createExchanger(format);
}

std::unique_ptr<PropertyGroup> OccFactoryWriter::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    return helper_OccFactoryWriter().createProperties(format, parentGroup);
}

} // namespace IO
} // namespace Mayo

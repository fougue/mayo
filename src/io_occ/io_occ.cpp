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
#include "io_occ_vrml.h"

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
class FactoryData {
public:
    using ExchangerGenerator = Generator<Format, std::unique_ptr<EXCHANGER>>;
    using PropertiesGenerator = Generator<Format, std::unique_ptr<PropertyGroup>, PropertyGroup*>;

    FactoryData& addExchanger(
            const Format& format,
            const typename ExchangerGenerator::FunctionType& fnExchanger,
            const PropertiesGenerator::FunctionType& fnProperties)
    {
        m_vecFormat.push_back(format);
        m_vecExchangerGenerator.push_back({ format, fnExchanger });
        if (fnProperties)
            m_vecPropertiesGenerator.push_back({ format, fnProperties });

        return *this;
    }

    template<typename T> FactoryData& addExchanger(const Format& format) {
        static_assert(std::is_base_of<EXCHANGER, T>::value);
        if constexpr(HasMember_createProperties<T>::value) {
            return this->addExchanger(format, &createProduct<T>, &T::createProperties);
        }
        else {
            return this->addExchanger(format, &createProduct<T>, nullptr);
        }
    }

    Span<const Format> formats() const { return m_vecFormat; }

    typename ExchangerGenerator::ProductType createExchanger(const Format& format) const {
        return findGenerator<ExchangerGenerator>(format, m_vecExchangerGenerator).fn();
    }

    PropertiesGenerator::ProductType createProperties(const Format& format, PropertyGroup* parentGroup) const
    {
        return findGenerator<PropertiesGenerator>(format, m_vecPropertiesGenerator).fn(parentGroup);
    }

private:
    template<typename T> class HasMember_createProperties {
        using Yes = char[2];
        using No = char[1];
        struct Fallback { int createProperties; };
        struct Derived : T, Fallback {};
        template<typename U> static No& test(decltype(U::createProperties)*);
        template<typename U> static Yes& test(U*);
    public:
        static constexpr bool value = sizeof(test<Derived>(nullptr)) == sizeof(Yes);
    };

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

inline static const auto occFactoryReaderData = FactoryData<Reader>()
        .addExchanger<OccStepReader>(Format_STEP)
        .addExchanger<OccIgesReader>(Format_IGES)
        .addExchanger<OccBRepReader>(Format_OCCBREP)
        #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        .addExchanger<OccGltfReader>(Format_GLTF)
        .addExchanger<OccObjReader>(Format_OBJ)
        #endif
        .addExchanger<OccStlReader>(Format_STL);

inline static const auto occFactoryWriterData = FactoryData<Writer>()
        .addExchanger<OccStepWriter>(Format_STEP)
        .addExchanger<OccIgesWriter>(Format_IGES)
        .addExchanger<OccBRepWriter>(Format_OCCBREP)
        #if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        .addExchanger<OccGltfWriter>(Format_GLTF)
        #endif
        .addExchanger<OccStlWriter>(Format_STL)
        .addExchanger<OccVrmlWriter>(Format_VRML);

} // namespace

Span<const Format> OccFactoryReader::formats() const
{
    return occFactoryReaderData.formats();
}

std::unique_ptr<Reader> OccFactoryReader::create(const Format& format) const
{
    return occFactoryReaderData.createExchanger(format);
}

std::unique_ptr<PropertyGroup> OccFactoryReader::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    return occFactoryReaderData.createProperties(format, parentGroup);
}

Span<const Format> OccFactoryWriter::formats() const
{
    return occFactoryWriterData.formats();
}

std::unique_ptr<Writer> OccFactoryWriter::create(const Format& format) const
{
    return occFactoryWriterData.createExchanger(format);
}

std::unique_ptr<PropertyGroup> OccFactoryWriter::createProperties(
        const Format& format, PropertyGroup* parentGroup) const
{
    return occFactoryWriterData.createProperties(format, parentGroup);
}

} // namespace IO
} // namespace Mayo

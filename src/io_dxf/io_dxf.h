/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"
#include "../base/io_single_format_factory.h"

#include <TopoDS_Shape.hxx>
#include <unordered_map>
#include <string>
#include <vector>

namespace Mayo::IO {

// Reader for DXF file format based on FreeCad's CDxfRead
class DxfReader : public Reader {
public:
    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

    struct Parameters {
        double scaling = 1.;
        bool importAnnotations = true;
        bool groupLayers = true;
        std::string fontNameForTextObjects = "Arial";
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

private:
    class Properties;
    class Internal;

    struct Entity {
        int aci = 0;
        TopoDS_Shape shape;
    };
    std::unordered_map<std::string, std::vector<Entity>> m_layers;
    Parameters m_params;
};

// Provides factory to create DxfReader objects
class DxfFactoryReader : public SingleFormatFactoryReader<Format_DXF, DxfReader> {};

} // namespace Mayo::IO

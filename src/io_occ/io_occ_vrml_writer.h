/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include <VrmlAPI_RepresentationOfShape.hxx>
#include <VrmlData_Scene.hxx>
#include <memory>

namespace Mayo::IO {

// Opencascade-based writer for VRML(v2.0 UTF8) file format
class OccVrmlWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    struct Parameters {
        VrmlAPI_RepresentationOfShape shapeRepresentation = VrmlAPI_BothRepresentation;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    class Properties;
    Parameters m_params;
    std::unique_ptr<VrmlData_Scene> m_scene;
    VrmlAPI_RepresentationOfShape m_shapeRepresentation = VrmlAPI_BothRepresentation;
};

} // namespace Mayo::IO

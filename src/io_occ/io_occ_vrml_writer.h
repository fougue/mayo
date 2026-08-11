/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_writer.h"
#include "../base/property_enumeration.h"
#include <VrmlAPI_RepresentationOfShape.hxx>
#include <VrmlData_Scene.hxx>
#include <memory>

namespace Mayo::IO {

// Opencascade-based writer for VRML(v2.0 UTF8) file format
class OccVrmlWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters
    struct Parameters : public PropertyGroup {
        PropertyEnum<VrmlAPI_RepresentationOfShape> shapeRepresentation{this, textId("shapeRepresentation")};
        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccVrmlWriter)
    Parameters m_params;
    std::unique_ptr<VrmlData_Scene> m_scene;
};

} // namespace Mayo::IO

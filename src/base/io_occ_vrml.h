/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_writer.h"
#include <VrmlAPI_RepresentationOfShape.hxx>
#include <VrmlData_Scene.hxx>
#include <memory>

namespace Mayo {
namespace IO {

// Opencascade-based writer for VRML(v2.0 UTF8) file format
class OccVrmlWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createParameters(PropertyGroup* parentGroup);
    void applyParameters(const PropertyGroup* params) override;

    // Parameters

    VrmlAPI_RepresentationOfShape shapeRepresentation() const { return m_shapeRepresentation; }
    void setShapeRepresentation(VrmlAPI_RepresentationOfShape rep) { m_shapeRepresentation = rep; }

private:
    std::unique_ptr<VrmlData_Scene> m_scene;
    VrmlAPI_RepresentationOfShape m_shapeRepresentation = VrmlAPI_BothRepresentation;
};

} // namespace IO
} // namespace Mayo

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_vrml.h"

#include "../base/application_item.h"
#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/math_utils.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <OSD_OpenFile.hxx>
#include <VrmlData_ShapeConvert.hxx>
#include <fstream>

namespace Mayo {
namespace IO {

class OccVrmlWriter::Properties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccVrmlWriter::Properties)
public:
    Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->shapeRepresentation.mutableEnumeration().chopPrefix("VrmlAPI_");
    }

    void restoreDefaults() override {
        const OccVrmlWriter::Parameters params;
        this->shapeRepresentation.setValue(params.shapeRepresentation);
    }

//    PropertyBool m_meshDeflectionFromShapeRelativeSize;
//    PropertyDouble m_meshDeflection;
//    PropertyDouble scale;
    PropertyEnum<VrmlAPI_RepresentationOfShape> shapeRepresentation{ this, textId("shapeRepresentation") };
};

bool OccVrmlWriter::transfer(Span<const ApplicationItem> spanAppItem, TaskProgress* progress)
{
    m_scene.reset(new VrmlData_Scene);
    VrmlData_ShapeConvert converter(*m_scene);
    for (const ApplicationItem& appItem : spanAppItem) {
        if (appItem.isDocument()) {
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
            converter.ConvertDocument(appItem.document());
#else
            // TODO Call VrmlData_ShapeConvert::AddShape() on each child entity of "shape" type
#endif
        }
        else if (appItem.isDocumentTreeNode()) {
            const TDF_Label label = appItem.documentTreeNode().label();
            if (XCaf::isShape(label))
                converter.AddShape(XCaf::shape(label));
        }

        const int index = &appItem - &spanAppItem.at(0);
        progress->setValue(MathUtils::mappedValue(index, 0, spanAppItem.size() - 1, 0, 100));
    }

    const auto rep = m_shapeRepresentation;
    converter.Convert(
                rep == VrmlAPI_ShadedRepresentation || rep == VrmlAPI_BothRepresentation,
                rep == VrmlAPI_WireFrameRepresentation || rep == VrmlAPI_BothRepresentation);
    return true;
}

bool OccVrmlWriter::writeFile(const QString& filepath, TaskProgress*)
{
    if (!m_scene)
        return false;

    std::ofstream outs;
    OSD_OpenStream(outs, filepath.toUtf8().constData(), std::ios::out);
    if (outs) {
        outs << *m_scene;
        outs.close();
        return outs.good();
    }

    return false;
}

std::unique_ptr<PropertyGroup> OccVrmlWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccVrmlWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.shapeRepresentation = static_cast<VrmlAPI_RepresentationOfShape>(ptr->shapeRepresentation);
    }
}

} // namespace IO
} // namespace Mayo

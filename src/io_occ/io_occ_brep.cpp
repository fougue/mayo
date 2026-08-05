/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_occ_brep.h"

#include "../base/application_item.h"
#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/filepath_conv.h"
#include "../base/occ_progress_indicator.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/task_progress.h"
#include "../base/tkernel_utils.h"

#include <BinTools.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Standard_Version.hxx>
#include <TDataStd_Name.hxx>

namespace Mayo::IO {

struct OccBRepI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccBRepI18N)
};

bool OccBRepReader::readFile(const FilePath& filepath, TaskProgress* progress)
{
    m_shape = TopoDS_Shape{};
    m_baseFilename = filepath.stem();

    auto indicator = makeOccHandle<OccProgressIndicator>(progress);

    char buff[2048] = {};
    auto probeInput = System::getFormatProbeInput(filepath, buff);
    if (isFormatAscii_OCCBREP(probeInput)) {
        BRep_Builder brepBuilder;
        return BRepTools::Read(
            m_shape, filepath.u8string().c_str(), brepBuilder, TKernelUtils::start(indicator)
        );
    }

    if (isFormatBinary_OCCBREP(probeInput)) {
        return BinTools::Read(
            m_shape,
            filepath.u8string().c_str()
#if OCC_VERSION_HEX >= 0x070600
            , TKernelUtils::start(indicator)
#endif
        );
    }

    this->messenger()->emitError(
        OccBRepI18N::textIdTr("Failed to guess OpenCascade BREP ascii/binary format")
    );
    return false;
}

NCollection_Sequence<TDF_Label> OccBRepReader::transfer(DocumentPtr doc, TaskProgress* /*progress*/)
{
    if (m_shape.IsNull())
        return {};

    const OccHandle<XCAFDoc_ShapeTool> shapeTool = doc->xcaf().shapeTool();
    const TDF_Label labelShape = shapeTool->NewShape();
    shapeTool->SetShape(labelShape, m_shape);
    TDataStd_Name::Set(labelShape, filepathTo<TCollection_ExtendedString>(m_baseFilename));
    return CafUtils::makeLabelSequence({ labelShape });
}

class OccBRepWriter::Properties : public PropertyGroup {
public:
    explicit Properties(PropertyGroup* parentGroup)
        : PropertyGroup(parentGroup)
    {
        this->targetFormat.mutableEnumeration().changeTrContext(OccBRepI18N::textIdContext());
        this->saveShapeTriangulation.setDescription(OccBRepI18N::textIdTr(
            "Specifies whether to save shape with or without triangles.\n"
            "Has no effect on triangulation-only geometry"
        ));
        this->saveShapeTriangulationNormals.setDescription(OccBRepI18N::textIdTr(
            "Specifies whether to save triangulation with or without normals.\n"
            "Has no effect on triangulation-only geometry"
        ));
    }

    void restoreDefaults() override {
        const OccBRepWriter::Parameters defaultParams;
        this->targetFormat.setValue(defaultParams.format);
        this->saveShapeTriangulation.setValue(defaultParams.saveShapeTriangulation);
        this->saveShapeTriangulationNormals.setValue(defaultParams.saveShapeTriangulationNormals);
    }

    PropertyEnum<OccBRepWriter::Format> targetFormat{ this, OccBRepI18N::textId("targetFormat") };
    PropertyBool saveShapeTriangulation{ this, OccBRepI18N::textId("saveShapeTriangulation") };
    PropertyBool saveShapeTriangulationNormals{ this, OccBRepI18N::textId("saveShapeTriangulationNormals") };
};

bool OccBRepWriter::transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* /*progress*/)
{
    m_shape = TopoDS_Shape{};

    std::vector<TopoDS_Shape> vecShape;
    vecShape.reserve(appItems.size());
    System::visitUniqueItems(appItems, [&](const ApplicationItem& item) {
        if (item.isDocument()) {
            for (const TDF_Label& label : item.document()->xcaf().topLevelFreeShapes())
                vecShape.push_back(XCaf::shape(label));
        }
        else if (item.isDocumentTreeNode()) {
            const TDF_Label labelNode = item.documentTreeNode().label();
            vecShape.push_back(XCaf::shape(labelNode));
        }
    });

    if (vecShape.size() > 1) {
        m_shape = BRepUtils::makeEmptyCompound();
        for (const TopoDS_Shape& subShape : vecShape)
            BRepUtils::addShape(&m_shape, subShape);
    }
    else if (vecShape.size() == 1) {
        m_shape = vecShape.front();
    }

    return true;
}

bool OccBRepWriter::writeFile(const FilePath& filepath, TaskProgress* progress)
{
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    if (m_params.format == Format::Ascii) {
        return BRepTools::Write(
            m_shape,
            filepath.u8string().c_str(),
#if OCC_VERSION_HEX >= 0x070600
            m_params.saveShapeTriangulation,
            m_params.saveShapeTriangulationNormals,
            TopTools_FormatVersion_CURRENT,
#endif
            TKernelUtils::start(indicator)
        );
    }
    else {
        return BinTools::Write(
            m_shape,
            filepath.u8string().c_str()
#if OCC_VERSION_HEX >= 0x070600
            , m_params.saveShapeTriangulation,
            m_params.saveShapeTriangulationNormals,
            BinTools_FormatVersion_CURRENT
#endif
#if OCC_VERSION_HEX >= 0x070500
            , TKernelUtils::start(indicator)
#endif
        );
    }
}

std::unique_ptr<PropertyGroup> OccBRepWriter::createProperties(PropertyGroup* parentGroup)
{
    return std::make_unique<Properties>(parentGroup);
}

void OccBRepWriter::applyProperties(const PropertyGroup* params)
{
    auto ptr = dynamic_cast<const Properties*>(params);
    if (ptr) {
        m_params.format = ptr->targetFormat;
        m_params.saveShapeTriangulation = ptr->saveShapeTriangulation;
        m_params.saveShapeTriangulationNormals = ptr->saveShapeTriangulationNormals;
    }
}

} // namespace Mayo::IO

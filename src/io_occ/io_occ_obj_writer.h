/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"

#include <RWMesh_CoordinateSystemConverter.hxx>

namespace Mayo::IO {

// OpenCascade-based writer for OBJ format
// Requires OpenCascade >= v7.6.0
class OccObjWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    struct Parameters {
        RWMesh_CoordinateSystem inputCoordinateSystem = RWMesh_CoordinateSystem_Undefined;
        RWMesh_CoordinateSystem outputCoordinateSystem = RWMesh_CoordinateSystem_glTF;
    };
    Parameters& parameters() { return m_params; }
    const Parameters& constParameters() const { return m_params; }

private:
    class Properties;
    Parameters m_params;
    DocumentPtr m_document;
    NCollection_Sequence<TDF_Label> m_seqRootLabel;
};

} // namespace Mayo::IO


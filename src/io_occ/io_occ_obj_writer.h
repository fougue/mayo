/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_writer.h"
#include "../base/property_enumeration.h"

#include <RWMesh_CoordinateSystemConverter.hxx>

namespace Mayo::IO {

// OpenCascade-based writer for OBJ format
// Requires OpenCascade >= v7.6.0
class OccObjWriter : public Writer {
public:
    bool transfer(gsl::span<const ApplicationItem> spanAppItem, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;

    // Parameters
    struct Parameters : public PropertyGroup {
        PropertyEnum<RWMesh_CoordinateSystem> inputCoordinateSystem{ this, textId("inputCoordinateSystem") };
        PropertyEnum<RWMesh_CoordinateSystem> outputCoordinateSystem{ this, textId("outputCoordinateSystem") };
        Parameters();
        void restoreDefaults() override;
    };
    Parameters& parameters() override { return m_params; }
    const Parameters& constParameters() const override { return m_params; }

private:
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccObjWriter)

    Parameters m_params;
    DocumentPtr m_document;
    NCollection_Sequence<TDF_Label> m_seqRootLabel;
};

} // namespace Mayo::IO


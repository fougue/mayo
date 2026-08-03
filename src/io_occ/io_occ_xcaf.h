/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/tkernel_utils.h"

#include <Standard_Version.hxx>

#include <type_traits>

class TDocStd_Application;

namespace Mayo::IO {

// Opencascade-based reader for XCAF file format
class OccXCafReader : public Reader {
public:
    OccXCafReader();
    OccXCafReader(const OccXCafReader&) = delete; // Not copyable
    OccXCafReader& operator=(const OccXCafReader&) = delete; // Not copyable

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    OccHandle<TDocStd_Application> m_app;
    OccHandle<TDocStd_Document> m_doc;
};

} // namespace Mayo::IO

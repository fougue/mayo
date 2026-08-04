/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/io_reader.h"

class TDocStd_Application;

namespace Mayo::IO {

// OpenCascade-based reader for XCAF file format
// Requires OpenCascade >= v7.6.0
// NOTE: XCAF persistence exists for a long time in OCCT, but required tool XCAFDoc_Editor::Extract()
//       has been added in 7.6.0 version
class OccXCafReader : public Reader {
public:
    ~OccXCafReader();

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    NCollection_Sequence<TDF_Label> transfer(DocumentPtr doc, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup*) override {}

private:
    void clearInternals();

    OccHandle<TDocStd_Application> m_app;
    OccHandle<TDocStd_Document> m_doc;
};

} // namespace Mayo::IO

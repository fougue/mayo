/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io.h"

#include <IGESCAFControl_Reader.hxx>
#include <STEPCAFControl_Reader.hxx>

namespace Mayo {
namespace IO {

class OccFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    Format findFormatFromContents(
            QByteArray contentsBegin,
            uint64_t hintFullContentsSize) const override;

    std::unique_ptr<Reader> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const override;
    bool applyParameters(const PropertyGroup& params, Reader* reader) const override;
};

class OccFactoryWriter : public FactoryWriter {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Writer> create(const Format& format) const override;
    std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const override;
    bool applyParameters(const PropertyGroup& params, Writer* writer) const override;
};

//class OccStlReader : public Reader {
//public:
//    bool readFile(const QString& filepath, TaskProgress* progress) override
//    {
//        Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
//        m_baseFilename = QFileInfo(filepath).baseName();
//        m_mesh = RWStl::ReadFile(OSD_Path(filepath.toLocal8Bit().constData()), indicator);
//        return !m_mesh.IsNull();
//    }

//    bool transfer(DocumentPtr doc, TaskProgress* /*progress*/) override
//    {
//        if (m_mesh.IsNull())
//            return false;

//        SingleScopeImport import(doc);
//        TDataXtd_Triangulation::Set(import.entityLabel(), m_mesh);
//        CafUtils::setLabelAttrStdName(import.entityLabel(), m_baseFilename);
//        return true;
//    }

//private:
//    Handle_Poly_Triangulation m_mesh;
//    QString m_baseFilename;
//};

} // namespace IO
} // namespace Mayo

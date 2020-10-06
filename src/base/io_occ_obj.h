/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io.h"
#include <RWObj_CafReader.hxx>

namespace Mayo {
namespace IO {

class OccObjReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

    static std::unique_ptr<PropertyGroup> createParameters(PropertyGroup* parentGroup);
    void applyParameters(const PropertyGroup* params) override;

    // Parameters

    bool isSinglePrecisionVertexCoords() const { return m_reader.IsSinglePrecision(); }
    void setSinglePrecisionVertexCoords(bool on) { m_reader.SetSinglePrecision(on); }

    QString rootPrefix() const;
    void setRootPrefix(const QString& prefix);

private:
    QString m_filepath;
    RWObj_CafReader m_reader;
};

} // namespace IO
} // namespace Mayo

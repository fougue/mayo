/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWObj_CafReader.hxx>

namespace Mayo {
namespace IO {

// OpenCascade-based reader for Wavefront OBJ format
// Requires OpenCascade >= v7.4.0
class OccObjReader : public OccBaseMeshReader {
public:
    OccObjReader();

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    bool isSinglePrecisionVertexCoords() const { return m_reader.IsSinglePrecision(); }
    void setSinglePrecisionVertexCoords(bool on) { m_reader.SetSinglePrecision(on); }

private:
    RWObj_CafReader m_reader;
};

} // namespace IO
} // namespace Mayo

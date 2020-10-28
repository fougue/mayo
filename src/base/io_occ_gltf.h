/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_base_mesh.h"
#include <RWGltf_CafReader.hxx>

namespace Mayo {
namespace IO {

// OpenCascade-based reader for glTF format
// Requires OpenCascade >= v7.4.0
class OccGltfReader : public OccBaseMeshReader {
public:
    OccGltfReader();

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup* parentGroup);
    void applyProperties(const PropertyGroup* params) override;

    // Parameters

    bool skipEmptyNodesOn() const { return m_reader.ToSkipEmptyNodes(); }
    void setSkipEmptyNodes(bool on) { m_reader.SetSkipEmptyNodes(on); }

    bool meshNameAsFallbackOn() const { return m_reader.ToUseMeshNameAsFallback(); }
    void setMeshNameAsFallback(bool on) { m_reader.SetMeshNameAsFallback(on); }

private:
    mutable RWGltf_CafReader m_reader;
};

} // namespace IO
} // namespace Mayo

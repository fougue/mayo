/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "command_system_information_occopengl.h"

#include "../base/meta_enum.h"
#include "../base/occt_ncollection_indexed_datamap_of_stringstring.h"
#include "../base/string_conv.h"

#include <OpenGl_Context.hxx>
#include <Standard_Version.hxx>

namespace Mayo::Internal {

OccOpenGlInfos getOccOpenGlInfos()
{
    OpenGl_Context occContext;
    if (!occContext.Init())
        throw std::runtime_error("Unable to initialize OpenGl_Context object");

    OccOpenGlInfos infos;

    NCollection_IndexedDataMapOfStringString dict;
    occContext.DiagnosticInformation(dict, Graphic3d_DiagnosticInfo_Basic);
    for (NCollection_IndexedDataMapOfStringString::Iterator it(dict); it.More(); it.Next())
        infos[to_stdString(it.Key())] = to_stdString(it.Value());

    infos["MaxDegreeOfAnisotropy"] = occContext.MaxDegreeOfAnisotropy();
    infos["MaxDrawBuffers"] = occContext.MaxDrawBuffers();
    infos["MaxClipPlanes"] = occContext.MaxClipPlanes();
    infos["HasRayTracing"] = occContext.HasRayTracing();
    infos["HasRayTracingTextures"] = occContext.HasRayTracingTextures();
    infos["HasRayTracingAdaptiveSampling"] = occContext.HasRayTracingAdaptiveSampling();
    infos["UseVBO"] = occContext.ToUseVbo();

#if OCC_VERSION_HEX >= 0x070400
    infos["MaxDumpSizeX"] = occContext.MaxDumpSizeX();
    infos["MaxDumpSizeY"] = occContext.MaxDumpSizeY();
    infos["HasRayTracingAdaptiveSamplingAtomic"] = occContext.HasRayTracingAdaptiveSamplingAtomic();
#endif

#if OCC_VERSION_HEX >= 0x070500
    infos["HasTextureBaseLevel"] = occContext.HasTextureBaseLevel();
    infos["HasSRGB"] = occContext.HasSRGB();
    infos["RenderSRGB"] = occContext.ToRenderSRGB();
    infos["IsWindowSRGB"] = occContext.IsWindowSRGB();
    infos["HasPBR"] = occContext.HasPBR();
#endif

#if OCC_VERSION_HEX >= 0x070700
    infos["GraphicsLibrary"] = std::string{MetaEnum::name(occContext.GraphicsLibrary())};
    infos["HasTextureMultisampling"] = occContext.HasTextureMultisampling();
#endif

    return infos;
}

} // namespace Mayo::Internal

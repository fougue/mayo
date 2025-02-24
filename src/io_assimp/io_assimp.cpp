/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp.h"
#include "io_assimp_reader.h"

#include <assimp/version.h>

namespace Mayo::IO {

Span<const Format> AssimpFactoryReader::formats() const
{
    static const Format array[] = {
        Format_AMF,
        Format_3DS,
        Format_3MF,
        Format_COLLADA,
        Format_FBX,
        Format_X3D,
        Format_Blender,
        Format_DirectX
    };
    return array;
}

std::unique_ptr<Reader> AssimpFactoryReader::create(Format format) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return std::make_unique<AssimpReader>();

    return {};
}

std::unique_ptr<PropertyGroup> AssimpFactoryReader::createProperties(Format format, PropertyGroup* parentGroup) const
{
    auto itFound = std::find(this->formats().begin(), this->formats().end(), format);
    if (itFound != this->formats().end())
        return AssimpReader::createProperties(parentGroup);

    return {};
}

std::string_view AssimpLib::strVersion()
{
    static std::string str;

    if (str.empty()) {
        str += std::to_string(aiGetVersionMajor())
               + "." + std::to_string(aiGetVersionMinor())
#ifdef MAYO_HAVE_ASSIMP_aiGetVersionPatch
               + "." + std::to_string(aiGetVersionPatch())
#else
               + ".?"
#endif
            ;
    }

    return str;
}

std::string_view AssimpLib::strVersionDetails()
{
    static std::string str;

    if (str.empty()) {
        const std::string strBranchName = aiGetBranchName() ? aiGetBranchName() : "";

        std::string strCompileFlags;
        {
            const unsigned compileFlags = aiGetCompileFlags();
            auto addFlagIfDefined = [&](unsigned flag, const char* strFlag) {
                if (compileFlags & flag) {
                    if (!strCompileFlags.empty())
                        strCompileFlags += "|";
                    strCompileFlags += strFlag;
                }
            };
            addFlagIfDefined(ASSIMP_CFLAGS_SHARED, "shared");
            addFlagIfDefined(ASSIMP_CFLAGS_STLPORT, "stlport");
            addFlagIfDefined(ASSIMP_CFLAGS_DEBUG, "debug");
            addFlagIfDefined(ASSIMP_CFLAGS_NOBOOST, "no-boost");
            addFlagIfDefined(ASSIMP_CFLAGS_SINGLETHREADED, "single-threaded");
#ifdef ASSIMP_CFLAGS_DOUBLE_SUPPORT
            addFlagIfDefined(ASSIMP_CFLAGS_DOUBLE_SUPPORT, "double-support");
#endif
        }

        str += "rev:" + std::to_string(aiGetVersionRevision())
               + " branch:" + (!strBranchName.empty() ? strBranchName : "?")
               + " flags:" + strCompileFlags
            ;
    }

    return str;
}

} // namespace Mayo::IO

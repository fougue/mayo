/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include <stb/stb_image_write.h>

#include "../base/filepath.h"
#include "../graphics/graphics_utils.h"

#include <Image_PixMap.hxx>

#include <locale>

namespace Mayo {

bool saveImage_stb(const Image_PixMap& pixmap, const FilePath& filepath)
{
    const auto width = static_cast<int>(pixmap.SizeX());
    const auto height = static_cast<int>(pixmap.SizeY());
    const auto pixels = static_cast<const unsigned char*>(pixmap.Data());
    const auto stride = static_cast<int>(pixmap.SizeRowBytes());

    const std::string strFilePath = filepath.u8string();
    std::string extension = filepath.extension().u8string();
    for (auto& c : extension)
        c = std::tolower(c, std::locale::classic());

    if (extension == ".png")
        return stbi_write_png(strFilePath.c_str(), width, height, 3, pixels, stride) != 0;

    if (extension == ".jpg" || extension == ".jpeg")
        return stbi_write_jpg(strFilePath.c_str(), width, height, 3, pixels, 95) != 0;

    if (extension == ".bmp")
        return stbi_write_bmp(strFilePath.c_str(), width, height, 3, pixels) != 0;

    if (extension == ".tga")
        return stbi_write_tga(strFilePath.c_str(), width, height, 3, pixels) != 0;

    return false;
}

} // namespace Mayo

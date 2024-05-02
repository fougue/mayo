/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "tkernel_utils.h"

#include <Message_ProgressIndicator.hxx>
#include <cmath>

namespace Mayo {

TKernelUtils::ReturnType_StartProgressIndicator
TKernelUtils::start(const OccHandle<Message_ProgressIndicator>& progress)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    return Message_ProgressIndicator::Start(progress);
#else
    return progress;
#endif
}

std::string TKernelUtils::colorToHex(const Quantity_Color& color)
{
//#if OCC_VERSION_HEX >= 0x070400
//    constexpr bool hashPrefix = true;
//    return to_QString(Quantity_Color::ColorToHex(this->value(), hashPrefix));
//#endif

    // Converts a decimal digit to hexadecimal character
    auto fnHexDigit = [](uint8_t v) {
        if (v < 10)
            return '0' + v;
        else
            return 'A' + (v - 10);
    };

    // Adds to 'str' the hexadecimal representation of 'v' belonging to [0, 255]
    auto fnAddHexColorComponent = [&](std::string& str, double v) {
        double iv = 0.; // Integral part of 'v'
        const double fv = std::modf(v / 16., &iv); // Fractional part of 'v'
        const auto a1 = uint8_t(iv);
        const auto a0 = uint8_t(fv * 16);
        str += fnHexDigit(a1);
        str += fnHexDigit(a0);
    };

    std::string strHex;
    strHex += '#';
    fnAddHexColorComponent(strHex, color.Red() * 255);
    fnAddHexColorComponent(strHex, color.Green() * 255);
    fnAddHexColorComponent(strHex, color.Blue() * 255);
    return strHex;
}

bool TKernelUtils::colorFromHex(std::string_view strHex, Quantity_Color* color)
{
//#if OCC_VERSION_HEX >= 0x070400
//    Quantity_Color color;
//    return Quantity_Color::ColorFromHex(strHexHex.c_str(), color);
//#else

    if (!color)
        return true;

    if (strHex.empty())
        return false;

    if (strHex.at(0) != '#')
        return false;

    if (strHex.size() != 7)
        return false;

    // Converts an hexadecimal digit to decimal digit
    auto fnFromHex = [](char c) {
        if (c >= '0' && c <= '9')
            return int(c - '0');
        else if (c >= 'A' && c <= 'F')
            return int(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f')
            return int(c - 'a' + 10);
        else
            return -1;
    };

    // Decodes a string containing an hexadecimal number to a decimal number
    auto fnHex2Int = [&](std::string_view str) {
        int result = 0;
        for (char c : str) {
            result = result * 16;
            const int h = fnFromHex(c);
            if (h < 0)
                return -1;

            result += h;
        }

        return result;
    };

    const int r = fnHex2Int(strHex.substr(1, 2));
    const int g = fnHex2Int(strHex.substr(3, 2));
    const int b = fnHex2Int(strHex.substr(5, 2));
    color->SetValues(r / 255., g / 255., b / 255., Quantity_TOC_RGB);
    return true;
}

Quantity_TypeOfColor TKernelUtils::preferredRgbColorType()
{
    // See https://dev.opencascade.org/content/occt-3d-viewer-becomes-srgb-aware
#if OCC_VERSION_HEX >= 0x070500
    return Quantity_TOC_sRGB;
#else
    return Quantity_TOC_RGB;
#endif
}

Quantity_Color TKernelUtils::toLinearRgbColor(const Quantity_Color& color)
{
#if OCC_VERSION_HEX >= 0x070500
    return Quantity_Color{ Quantity_Color::Convert_LinearRGB_To_sRGB(color.Rgb()) };
#else
    return color;
#endif
}

} // namespace Mayo

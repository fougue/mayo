/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/global.h"
#include "../src/base/string_conv.h"

#include <clocale>
#include <locale>
#include <optional>

namespace Mayo {

namespace {

struct frlike_numpunct : public std::numpunct<char> {
    char do_thousands_sep() const override { return ' '; }
    char do_decimal_point() const override { return ','; }
    std::string do_grouping() const override { return "\3"; }
};

std::locale getFrLocale()
{
    auto fnGetLocale = [](const char* name) -> std::optional<std::locale> {
        try {
            return std::locale(name);
        } catch (...) {
            qWarning().noquote() << QString("Locale '%1' not available").arg(name);
        }

        return {};
    };

    // Tests with "fr_FR" locale which is likely to be Windows-1252 or ISO8859-1 on Unix
    std::vector<const char*> frLocaleNames = { "fr_FR.ISO8859-15", "fr_FR.ISO-8859-15" };
#ifndef MAYO_OS_WINDOWS
    // No native utf8 support on Windows(or requires Windows 10 november 2019 update)
    frLocaleNames.push_back("fr_FR.utf8");
#endif
    frLocaleNames.push_back("fr_FR");

    std::optional<std::locale> frLocale;
    for (const char* localeName : frLocaleNames) {
        if (!frLocale)
            frLocale = fnGetLocale(localeName);
    }

    if (!frLocale) {
        frLocale = std::locale(std::cout.getloc(), new frlike_numpunct);
    }
    else {
        const auto& facet = std::use_facet<std::numpunct<char>>(frLocale.value());
        if (facet.decimal_point() != ',' || !std::isspace(facet.thousands_sep(), frLocale.value()))
            frLocale = std::locale(frLocale.value(), new frlike_numpunct);
    }

    assert(frLocale.has_value());
    return frLocale.value();
}

} // namespace

void TestBase::StringConv_DoubleToString_test()
{
    const std::locale frLocale = getFrLocale();
    qInfo() << "frLocale:" << frLocale.name().c_str();
    // 1258.
    {
        const std::string str = to_stdString(1258.).locale(frLocale).toUtf8(false);
        QCOMPARE(str.at(0), '1');
        QCOMPARE(str.at(1), std::use_facet<std::numpunct<char>>(frLocale).thousands_sep());
        QCOMPARE(str.substr(2, 3), "258");
    }

    // 57.89
    {
        QCOMPARE(to_stdString(57.89).locale(frLocale).get(), "57,89");
    }

    // Tests with "C" locale
    const std::locale& cLocale = std::locale::classic();
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(4).get(), "0.5578");
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(6).get(), "0.5578");
    QCOMPARE(to_stdString(0.5578).locale(cLocale).decimalCount(6).removeTrailingZeroes(false).get(), "0.557800");
    QCOMPARE(to_stdString(0.0).locale(cLocale).decimalCount(6).get(), "0");
    QCOMPARE(to_stdString(-45.6789).locale(cLocale).decimalCount(6).get(), "-45.6789");
}

void TestBase::StringConv_test()
{
    const std::string stdStr = to_stdString(14758.5).locale(getFrLocale());
    const auto occExtStr = to_OccExtString(stdStr);
    QCOMPARE(stdStr, to_stdString(occExtStr));
}

} // namespace Mayo

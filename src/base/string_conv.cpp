/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "global.h"
#if defined(MAYO_OS_WINDOWS)
#  include <Windows.h>
#else
#  include <iconv.h>
#  include <langinfo.h>
#  include <locale.h>
#endif

#include "string_conv.h"

#include "cpp_utils.h"
#include "math_utils.h"
#include <gsl/util>
#include <algorithm>
#include <sstream>
#include <vector>

namespace Mayo {

namespace {

#if defined(MAYO_OS_WINDOWS)
// See page https://devblogs.microsoft.com/oldnewthing/20161007-00/?p=94475
// Returns 0 in case of error
UINT getAnsiCodePageForLocale(LCID lcid)
{
    UINT acp = 0;
    const LCTYPE infoFlags = LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER;
    auto acpAsChars = reinterpret_cast<LPTSTR>(&acp);
    const int sizeInChars = sizeof(acp) / sizeof(TCHAR);
    if (GetLocaleInfo(lcid, infoFlags, acpAsChars, sizeInChars) != sizeInChars)
        return 0;

    return acp;
}

bool toUtf16String(std::string_view str, UINT localeAcp, std::vector<wchar_t>& utf16)
{
    // Compute length of utf16 string for memory allocation
    const int lenStr = CppUtils::safeStaticCast<int>(str.size());
    const int lenUtf16 = MultiByteToWideChar(localeAcp, MB_ERR_INVALID_CHARS, str.data(), lenStr, nullptr, 0);
    if (lenUtf16 == 0)
        return {};

    // Encode to utf16 string
    utf16.resize(lenUtf16 + 1);
    const int convCount = MultiByteToWideChar(localeAcp, MB_ERR_INVALID_CHARS, str.data(), lenStr, utf16.data(), lenUtf16);
    if (convCount == 0)
        utf16.resize(1);

    utf16.back() = L'\0';
    return convCount != 0;
}

LCID getLocaleIdFromName(const char* localeName)
{
    std::vector<wchar_t> wchars;
    toUtf16String(localeName, CP_UTF8, wchars);
    return LocaleNameToLCID(wchars.data(), 0/*LOCALE_ALLOW_NEUTRAL_NAMES*/);
}
#endif

// Convert to UTF8 the string 'str' encoded with 'locale'
std::string toUtf8String(std::string_view str, const std::locale& locale)
{
    if (str.empty())
        return std::string{str};

#if defined(MAYO_OS_WINDOWS)
    // Retrieve the ANSI code page corresponding to the input locale
    const LCID localeId = getLocaleIdFromName(locale.name().c_str());
    const UINT localeAcp = getAnsiCodePageForLocale(localeId);
    if (localeAcp == 0)
        return std::string{str}; // Assume utf8

    if (localeAcp == CP_UTF8)
        return std::string{str}; // Target locale is already utf8

    // Encode to intermediate utf16 string
    thread_local std::vector<wchar_t> utf16;
    if (!toUtf16String(str, localeAcp, utf16))
        return {};

    // Encode intermediate utf16 string to utf8
    const int lenUtf16 = Cpp::safeStaticCast<int>(utf16.size()) - 1;
    const int lenUtf8 = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), lenUtf16, nullptr, 0, nullptr, nullptr);
    thread_local std::vector<char> utf8;
    utf8.resize(lenUtf8 + 1);
    WideCharToMultiByte(CP_UTF8, 0, utf16.data(), lenUtf16, utf8.data(), lenUtf8, nullptr, nullptr);
    utf8.back() = '\0';

    return utf8.data();
#else
    // Helper function to check 'lhs' and 'rhs' strings are case-insensitive equal
    auto fnStringIEqual_c = [](std::string_view lhs, std::string_view rhs) {
        if (lhs.size() != rhs.size())
            return false;

        const std::locale& clocale = std::locale::classic();
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (std::toupper(lhs.at(i), clocale) != std::toupper(rhs.at(i), clocale))
                return false;
        }

        return true;
    };

    // Try to find charset with nl_langinfo_l() POSIX function
    const std::string localeName = locale.name();
    std::string charset;
    {
        const locale_t nullLocale = reinterpret_cast<locale_t>(0);
        const locale_t loc = newlocale(LC_CTYPE_MASK, localeName.c_str(), nullLocale);
        if (loc != nullLocale) {
            const char* zcharset = nl_langinfo_l(CODESET, loc);
            charset = zcharset ? zcharset : "";
            freelocale(loc);
        }
    }

    // Try to find the charset from the locale name(eg UTF-8 in en_US.UTF-8)
    if (charset.empty()) {
        const auto dotPos = localeName.find('.');
        if (dotPos != std::string::npos)
            charset = localeName.substr(dotPos + 1);
    }

    // Couldn't find locale charset...
    if (charset.empty())
        return std::string{str};

    // If locale charset is already utf8 encoded then directly return input string
    if (fnStringIEqual_c(charset, "UTF-8") || fnStringIEqual_c(charset, "UTF8"))
        return std::string{str};

    // Allocate conversion descriptor
    iconv_t convd = iconv_open("UTF-8", charset.c_str());
    [[maybe_unused]] auto _ = gsl::finally([=]{ iconv_close(convd); });
    if (convd == reinterpret_cast<iconv_t>(-1))
        return std::string{str};

    // Attempt to convert input string to utf8(limit attempt count)
    thread_local std::vector<char> utf8;
    utf8.resize(str.size());
    std::fill(utf8.begin(), utf8.end(), '\0');
    constexpr size_t iconvError = -1;
    size_t iconvRes = 0;
    constexpr int maxAttemptCount = 10;
    int attemptCount = 0;
    do {
        char* strData = const_cast<char*>(str.data());
        size_t lenStr = str.size();
        utf8.resize(utf8.size() + (utf8.size() * 0.25) + 1); // Grow size by 25% each attempt
        std::fill(utf8.begin(), utf8.end(), '\0');
        char* utf8Data = utf8.data();
        size_t lenUtf8 = utf8.size();
        iconvRes = iconv(convd, &strData, &lenStr, &utf8Data, &lenUtf8);
        ++attemptCount;
    } while (iconvRes == iconvError && attemptCount < maxAttemptCount);

    if (iconvRes == iconvError)
        return std::string{str};

    return utf8.data();
#endif
}

} // namespace

std::string to_stdString(double value, const DoubleToStringOptions& opts)
{
    // Helper function to return the last character of string 'str'
    auto fnLastChar = [](const std::string& str) {
        return !str.empty() ? str.at(str.size() - 1) : char{};
    };
    // Helper function to erase the last character of the string pointed to by 'str'
    auto fnEraseLastChar = [](std::string* str) {
        if (!str->empty())
            str->erase(str->size() - 1);
    };

    value = opts.roundToZero && MathUtils::fuzzyIsNull(value) ? 0. : value;
    std::ostringstream sstr;
    sstr.imbue(opts.locale);
    sstr << std::setprecision(opts.decimalCount) << std::fixed << value;
    std::string str = sstr.str();
    if (opts.removeTrailingZeroes) {
        const char chDecPnt = std::use_facet<std::numpunct<char>>(opts.locale).decimal_point();
        if (str.find(chDecPnt) != std::string::npos) { // Remove useless trailing zeroes
            while (fnLastChar(str) == '0')
                fnEraseLastChar(&str);

            if (fnLastChar(str) == chDecPnt)
                fnEraseLastChar(&str);
        }
    }

    return opts.toUtf8 ? toUtf8String(str, opts.locale) : str;
}

DoubleToStringOperation::DoubleToStringOperation(double value)
    : m_value(value)
{
}

DoubleToStringOperation& DoubleToStringOperation::locale(const std::locale& l)
{
    m_opts.locale = l;
    return *this;
}

DoubleToStringOperation& DoubleToStringOperation::decimalCount(int c)
{
    m_opts.decimalCount = c;
    return *this;
}

DoubleToStringOperation& DoubleToStringOperation::removeTrailingZeroes(bool on)
{
    m_opts.removeTrailingZeroes = on;
    return *this;
}

DoubleToStringOperation& DoubleToStringOperation::roundToZero(bool on)
{
    m_opts.roundToZero = on;
    return *this;
}

DoubleToStringOperation& DoubleToStringOperation::toUtf8(bool on)
{
    m_opts.toUtf8 = on;
    return *this;
}

DoubleToStringOperation::operator std::string()
{
    return to_stdString(m_value, m_opts);
}

std::string DoubleToStringOperation::get() const
{
    return to_stdString(m_value, m_opts);
}

DoubleToStringOperation to_stdString(double value)
{
    DoubleToStringOperation op(value);
    return op;
}

} // namespace Mayo

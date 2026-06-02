/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#if defined(__cpp_lib_filesystem)
#  define MAYO_HAS_STD_FILESYSTEM
#elif defined(__has_include)
#  if __has_include(<filesystem>)
#    define MAYO_HAS_STD_FILESYSTEM
#  endif
#endif

#ifndef Q_MOC_RUN // Workaround bug https://bugreports.qt.io/browse/QTBUG-73263 which affects GCC builds
#  ifdef MAYO_HAS_STD_FILESYSTEM
#    include <filesystem>
namespace std_filesystem = std::filesystem;
#  else
#    include <experimental/filesystem>
namespace std_filesystem = std::experimental::filesystem;
#  endif
#endif

namespace Mayo {

using FilePath = std_filesystem::path;

// Exception-safe version of std::filesystem::file_size()
inline uintmax_t filepathFileSize(const FilePath& fp)
{
    // NOTE fs::file_size(fp) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto size = std_filesystem::file_size(fp, ec);
    return ec ? 0u : size;
}

// Exception-safe version of std::filesystem::canonical()
inline FilePath filepathCanonical(const FilePath& fp)
{
    // NOTE fs::canonical(fp) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto cfp = std_filesystem::canonical(fp, ec);
    return ec ? fp : cfp;
}

// Exception-safe version of std::filesystem::equivalent()
inline bool filepathExists(const FilePath& fp)
{
    // NOTE fs::exists(fp) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto exists = std_filesystem::exists(fp, ec);
    return ec ? false : exists;
}

// Exception-safe version of std::filesystem::equivalent()
inline bool filepathEquivalent(const FilePath& lhs, const FilePath& rhs)
{
    if (lhs == rhs)
        return true;

    // NOTE fs::equivalent(f1,f2) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto equiv = std_filesystem::equivalent(lhs, rhs, ec);
    return ec ? false : equiv;
}

// Exception-safe version of std::filesystem::is_regular_file()
inline bool filepathIsRegularFile(const FilePath& fp)
{
    // NOTE fs::is_regular_file(fp) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto isregf = std_filesystem::is_regular_file(fp, ec);
    return ec ? false : isregf;
}

// Exception-safe version of std::filesystem::last_write_time()
inline std_filesystem::file_time_type filepathLastWriteTime(const FilePath& fp)
{
    // NOTE fs::last_write_time(fp) might throw on non-existing files, use overload with std::error_code
    std::error_code ec;
    const auto lwt = std_filesystem::last_write_time(fp, ec);
    return ec ? std_filesystem::file_time_type{} : lwt;
}

} // namespace Mayo

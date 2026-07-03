/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define MAYO_OS_WINDOWS
#elif defined(__APPLE__)
#  define MAYO_OS_MAC
#elif defined(__linux__) || defined(__linux)
#  define MAYO_OS_LINUX
#elif defined(__ANDROID__) || defined(ANDROID)
#  define MAYO_OS_ANDROID
#  define MAYO_OS_LINUX
#elif defined(__WEBOS__)
#  define MAYO_OS_WEBOS
#  define MAYO_OS_LINUX
#elif defined(__EMSCRIPTEN__)
#  define MAYO_OS_WASM
#endif

#if !defined(MAYO_OS_WINDOWS)
#  define MAYO_OS_UNIX
#endif

namespace Mayo {

enum class CheckState { Off, Partially, On };

} // namespace Mayo

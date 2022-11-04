/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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

// Avoid "unused parameter" warnings
#define MAYO_UNUSED(x) (void)x;

namespace Mayo {

enum class CheckState { Off, Partially, On };

} // namespace Mayo

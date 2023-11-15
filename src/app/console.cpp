/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "console.h"

#include <QtCore/QtGlobal>
#ifdef Q_OS_WIN
#  include <io.h>
#  include <windows.h>
#else
#  include <sys/ioctl.h> // ioctl() and TIOCGWINSZ
#  include <unistd.h>    // for STDOUT_FILENO
#  include <iostream>
#endif

#include "../base/global.h"
#include "qstring_conv.h"

namespace Mayo {

void consoleSetTextColor(ConsoleColor color)
{
    constexpr bool isBrightText = true;
#ifdef Q_OS_WIN
    auto fnColorFlags = [](ConsoleColor color) {
        switch (color) {
        case ConsoleColor::Black:   return 0;
        case ConsoleColor::Red:     return FOREGROUND_RED;
        case ConsoleColor::Green:   return FOREGROUND_GREEN;
        case ConsoleColor::Blue:    return FOREGROUND_BLUE;
        case ConsoleColor::Yellow:  return FOREGROUND_RED | FOREGROUND_GREEN;
        case ConsoleColor::Cyan:    return FOREGROUND_GREEN | FOREGROUND_BLUE;
        case ConsoleColor::Magenta: return FOREGROUND_RED | FOREGROUND_BLUE;
        case ConsoleColor::Default:
        case ConsoleColor::White:
        default: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    };

    // There is no difference between STD_OUTPUT_HANDLE/STD_ERROR_HANDLE for std::cout/std::cerr
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout) {
        const bool isDefaultColor = color == ConsoleColor::Default;
        const WORD brightFlag = isBrightText && !isDefaultColor ? FOREGROUND_INTENSITY : 0;
        const WORD flags = fnColorFlags(color) | brightFlag;
        SetConsoleTextAttribute(hStdout, flags);
    }
#elif defined(__EMSCRIPTEN__)
    // Terminal capabilities are undefined on this platform.
    // std::cout could be redirected to HTML page, into terminal or somewhere else.
    MAYO_UNUSED(color);
    MAYO_UNUSED(isBrightText);
#else
    auto fnCode = [](ConsoleColor color, bool isBrightText) {
        switch (color) {
        case ConsoleColor::Black:   return isBrightText ? "\e[30;1m" : "\e[30m";
        case ConsoleColor::Red:     return isBrightText ? "\e[31;1m" : "\e[31m";
        case ConsoleColor::Green:   return isBrightText ? "\e[32;1m" : "\e[32m";
        case ConsoleColor::Yellow:  return isBrightText ? "\e[33;1m" : "\e[33m";
        case ConsoleColor::Blue:    return isBrightText ? "\e[34;1m" : "\e[34m";
        case ConsoleColor::Magenta: return isBrightText ? "\e[35;1m" : "\e[35m";
        case ConsoleColor::Cyan:    return isBrightText ? "\e[36;1m" : "\e[36m";
        case ConsoleColor::White:   return isBrightText ? "\e[37;1m" : "\e[37m";
        case ConsoleColor::Default:
        default: return "\e[0m";
        }
    };
    std::cout << fnCode(color, isBrightText);
#endif
}

void consoleCursorMoveUp(int lines)
{
    if (lines == 0)
        return;

#ifdef Q_OS_WIN
    auto hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout) {
        CONSOLE_SCREEN_BUFFER_INFO buffInfo;
        GetConsoleScreenBufferInfo(hStdout, &buffInfo);
        COORD cursor;
        cursor.X = buffInfo.dwCursorPosition.X;
        cursor.Y = buffInfo.dwCursorPosition.Y - lines;
        SetConsoleCursorPosition(hStdout, cursor);
    }
#else
    std::cout << "\033[" << lines << "A";
#endif
}

void consoleCursorShow(bool on)
{
#ifdef Q_OS_WIN
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout) {
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hStdout, &cursorInfo);
        cursorInfo.bVisible = on;
        SetConsoleCursorInfo(hStdout, &cursorInfo);
    }
#else
    std::fputs(on ? "\033[?25h" : "\033[?25l", stdout);
#endif
}

std::pair<int, int> consoleSize()
{
#ifdef Q_OS_WIN
    CONSOLE_SCREEN_BUFFER_INFO buffInfo;
    int cols, rows;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffInfo);
    cols = buffInfo.srWindow.Right - buffInfo.srWindow.Left + 1;
    rows = buffInfo.srWindow.Bottom - buffInfo.srWindow.Top + 1;
    return { rows, cols };
#else
    struct winsize size{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    return { size.ws_row, size.ws_col };
#endif
}

int consoleWidth() {
    return consoleSize().second;
}

void consoleSendEnterKey()
{
#ifdef Q_OS_WIN
    HWND consoleWnd = GetConsoleWindow();
    if (IsWindow(consoleWnd))
        PostMessage(consoleWnd, WM_KEYUP, VK_RETURN, 0);
#endif
}

std::string consoleToPrintable(const QString& str)
{
#ifdef Q_OS_WIN
    const auto codepage = GetConsoleOutputCP();
    const wchar_t* source = reinterpret_cast<const wchar_t*>(str.utf16());
    const int dstSize = WideCharToMultiByte(codepage, 0, source, -1, nullptr, 0, nullptr, nullptr);
    std::string dst;
    dst.resize(dstSize + 1);
    WideCharToMultiByte(codepage, 0, source, -1, dst.data(), dstSize, nullptr, nullptr);
    dst.back() = '\0';
    return dst;
#else
    return str.toStdString(); // utf8
#endif
}

std::string consoleToPrintable(std::string_view str)
{
#ifdef Q_OS_WIN
    return consoleToPrintable(to_QString(str));
#else
    return std::string(str); // utf8
#endif
}

} // namespace Mayo

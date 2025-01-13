/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <utility>

namespace Mayo {

// Color for console/terminal output(limited palette)
enum class ConsoleColor {
  Default, Black, White, Red, Blue, Green, Yellow, Cyan, Magenta
};

// Note: excerpted from OpenCascade's Message_PrinterOStream::SetConsoleTextColor()
//       This function appeared with v7.5.0
void consoleSetTextColor(ConsoleColor color);

// Note: excerpted from p-ranav/indicators/cursor_movement.hpp
void consoleCursorMoveUp(int lines);

// Note: excerpted from p-ranav/indicators/cursor_control.hpp
void consoleCursorShow(bool on);

// Note: excerpted from p-ranav/indicators/terminal_size.hpp
std::pair<int, int> consoleSize();
int consoleWidth();

// Sends "Enter" to the console
//     Useful to release the command prompt on the parent console when using AttachConsole()
void consoleSendEnterKey();

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include <Standard_Version.hxx>

#if OCC_VERSION_HEX >= 0x070400
#  include <Aspect_VKeyFlags.hxx>
#else
// Excerpted from OpenCascade/inc/Aspect_VKeyFlags.hxx
// This header was introduced with OpenCascade v7.4.0

//! Mouse buttons, for combining with Aspect_VKey and Aspect_VKeyFlags.
typedef unsigned int Aspect_VKeyMouse;

//! Mouse button bitmask
enum
{
  Aspect_VKeyMouse_NONE         = 0,       //!< no buttons

  Aspect_VKeyMouse_LeftButton   = 1 << 13, //!< mouse left   button
  Aspect_VKeyMouse_MiddleButton = 1 << 14, //!< mouse middle button (scroll)
  Aspect_VKeyMouse_RightButton  = 1 << 15, //!< mouse right  button

  Aspect_VKeyMouse_MainButtons = Aspect_VKeyMouse_LeftButton | Aspect_VKeyMouse_MiddleButton | Aspect_VKeyMouse_RightButton
};
#endif

constexpr unsigned int Aspect_VKeyMouse_UNKNOWN = 1 << 24;

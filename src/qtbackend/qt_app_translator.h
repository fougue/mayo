/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/text_id.h"

#include <string_view>

namespace Mayo {

// Function called by the Application i18n system, see Application::addTranslator()
std::string_view qtAppTranslate(const TextId& text, int n);

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"

#include <gsl/span>
#include <vector>

namespace Mayo {

// Stores the UI state of the main application widgets
struct AppUiState {
    std::vector<uint8_t> mainWindowGeometry; // Provided by QWidget::saveGeometry()
    bool pageDocuments_isLeftSideBarVisible = true;
    double pageDocuments_widgetLeftSideBarWidthFactor = 0.25;

    // Serialization functions
    static std::vector<uint8_t> toBlob(const AppUiState& state);
    static AppUiState fromBlob(gsl::span<const uint8_t> blob, bool* ok = nullptr);
};
using PropertyAppUiState = GenericProperty<AppUiState>;

} // namespace Mayo

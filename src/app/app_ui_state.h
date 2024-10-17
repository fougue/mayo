/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include <QtCore/QByteArray>

namespace Mayo {

// Stores the UI state of the main application widgets
struct AppUiState {
    QByteArray mainWindowGeometry; // Provided by QWidget::saveGeometry()
    bool pageDocuments_isLeftSideBarVisible = true;
    double pageDocuments_widgetLeftSideBarWidthFactor = 0.25;

    // Serialization functions
    static QByteArray toBlob(const AppUiState& state);
    static AppUiState fromBlob(const QByteArray& blob, bool* ok = nullptr);
};
using PropertyAppUiState = GenericProperty<AppUiState>;

} // namespace Mayo

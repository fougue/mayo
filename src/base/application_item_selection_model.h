/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "signal.h"

#include <gsl/span>

namespace Mayo {

// Keeps track of the items selected in an Application object
class ApplicationItemSelectionModel {
public:
    gsl::span<const ApplicationItem> selectedItems() const;

    bool isSelected(const ApplicationItem& item);

    void add(const ApplicationItem& item);
    void add(gsl::span<ApplicationItem> vecItem);
    void remove(const ApplicationItem& item);
    void remove(gsl::span<ApplicationItem> vecItem);
//    void toggle(const ApplicationItem& item);
//    void toggle(gsl::span<ApplicationItem> item);

    void clear();

    Signal<gsl::span<const ApplicationItem>, gsl::span<const ApplicationItem>> signalChanged;

private:
    std::vector<ApplicationItem> m_vecSelectedItem;
};

} // namespace Mayo

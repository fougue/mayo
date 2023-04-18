/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "signal.h"
#include "span.h"

namespace Mayo {

// Keeps track of the items selected in an Application object
class ApplicationItemSelectionModel {
public:
    Span<const ApplicationItem> selectedItems() const;

    bool isSelected(const ApplicationItem& item);

    void add(const ApplicationItem& item);
    void add(Span<ApplicationItem> vecItem);
    void remove(const ApplicationItem& item);
    void remove(Span<ApplicationItem> vecItem);
//    void toggle(const ApplicationItem& item);
//    void toggle(Span<ApplicationItem> item);

    void clear();

    Signal<Span<const ApplicationItem>, Span<const ApplicationItem>> signalChanged;

private:
    std::vector<ApplicationItem> m_vecSelectedItem;
};

} // namespace Mayo

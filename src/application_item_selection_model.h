/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "span.h"
#include <vector>

namespace Mayo {

class ApplicationItemSelectionModel : public QObject {
    Q_OBJECT
public:
    ApplicationItemSelectionModel(QObject* parent = nullptr);

    Span<const ApplicationItem> selectedItems() const;

    bool hasSelectedDocumentItems() const;
    std::vector<DocumentItem*> selectedDocumentItems() const;

    void add(const ApplicationItem& item);
    void add(Span<ApplicationItem> vecItem);
    void remove(const ApplicationItem& item);
    void remove(Span<ApplicationItem> vecItem);
//    void toggle(const ApplicationItem& item);
//    void toggle(Span<ApplicationItem> item);

    void clear();

signals:
    void cleared();
    void changed(Span<ApplicationItem> selected,
                 Span<ApplicationItem> deselected);

private:
    std::vector<ApplicationItem> m_vecSelectedItem;
};

} // namespace Mayo

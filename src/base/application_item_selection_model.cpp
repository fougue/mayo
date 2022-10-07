/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application_item_selection_model.h"

namespace Mayo {

namespace Internal {

static bool hasApplicationItem(Span<ApplicationItem> vec, const ApplicationItem& item)
{
    auto itFound = std::find(vec.begin(), vec.end(), item);
    return itFound != vec.end();
}

static std::vector<ApplicationItem>::iterator findApplicationItem(
        std::vector<ApplicationItem>& vec, const ApplicationItem& item)
{
    return std::find(vec.begin(), vec.end(), item);
}

} // namespace Internal

Span<const ApplicationItem> ApplicationItemSelectionModel::selectedItems() const
{
    return m_vecSelectedItem;
}

bool ApplicationItemSelectionModel::isSelected(const ApplicationItem& item)
{
    return Internal::hasApplicationItem(m_vecSelectedItem, item);
}

void ApplicationItemSelectionModel::add(const ApplicationItem& item)
{
    if (!Internal::hasApplicationItem(m_vecSelectedItem, item)) {
        m_vecSelectedItem.push_back(item);
        std::vector<ApplicationItem> vecItem = { item };
        this->signalChanged.send(vecItem, {});
    }
}

void ApplicationItemSelectionModel::add(Span<ApplicationItem> vecItem)
{
    std::vector<ApplicationItem> signalVecItem;
    for (const ApplicationItem& item : vecItem) {
        if (!Internal::hasApplicationItem(m_vecSelectedItem, item)) {
            m_vecSelectedItem.push_back(item);
            signalVecItem.push_back(item);
        }
    }

    if (!signalVecItem.empty())
        this->signalChanged.send(signalVecItem, {});
}

void ApplicationItemSelectionModel::remove(const ApplicationItem& item)
{
    auto itFound = Internal::findApplicationItem(m_vecSelectedItem, item);
    if (itFound != m_vecSelectedItem.end()) {
        m_vecSelectedItem.erase(itFound);
        std::vector<ApplicationItem> vecItem = { item };
        this->signalChanged.send({}, vecItem);
    }
}

void ApplicationItemSelectionModel::remove(Span<ApplicationItem> vecItem)
{
    std::vector<ApplicationItem> signalVecItem;
    for (const ApplicationItem& item : vecItem) {
        auto itFound = Internal::findApplicationItem(m_vecSelectedItem, item);
        if (itFound != m_vecSelectedItem.end()) {
            m_vecSelectedItem.erase(itFound);
            signalVecItem.push_back(item);
        }
    }

    if (!signalVecItem.empty())
        this->signalChanged.send({}, signalVecItem);
}

void ApplicationItemSelectionModel::clear()
{
    if (!m_vecSelectedItem.empty()) {
        // Warning: slots connected to changed() signal may indirectly access m_vecSelectedItem
        const auto vecDeselectedItem = m_vecSelectedItem;
        m_vecSelectedItem.clear();
        this->signalChanged.send({}, vecDeselectedItem);
    }
}

} // namespace Mayo

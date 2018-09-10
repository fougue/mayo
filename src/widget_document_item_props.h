/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "span.h"

#include <QtWidgets/QWidget>
#include <vector>

class QTreeWidgetItem;

namespace Mayo {

class DocumentItem;
class GpxDocumentItem;

class WidgetDocumentItemProps : public QWidget {
public:
    WidgetDocumentItemProps(QWidget* parent = nullptr);
    ~WidgetDocumentItemProps();

    void editDocumentItem(DocumentItem* docItem);
    void editProperties(Span<HandleProperty> spanHndProp);

private:
    void createQtProperties(
            const std::vector<Property*>& properties, QTreeWidgetItem* parentItem);
    void createQtProperty(
            Property* property, QTreeWidgetItem* parentItem);
    void refreshAllQtProperties();

    class Ui_WidgetDocumentItemProps* m_ui = nullptr;

    DocumentItem* m_currentDocItem = nullptr;
    GpxDocumentItem* m_currentGpxDocItem = nullptr;
    std::vector<HandleProperty> m_currentVecHndProperty;
};

} // namespace Mayo

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

class Document;
class DocumentItem;
class GpxDocumentItem;

namespace Internal { class PropertyItemDelegate; }

class WidgetPropertiesEditor : public QWidget {
public:
    WidgetPropertiesEditor(QWidget* parent = nullptr);
    ~WidgetPropertiesEditor();

    void editProperties(PropertyOwner* propertyOwner);
    void editProperties(Span<HandleProperty> spanHndProp);
    void clear();

    void addLineWidget(QWidget* widget);

    double rowHeightFactor() const;
    void setRowHeightFactor(double v);

private:
    void createQtProperties(
            const std::vector<Property*>& properties, QTreeWidgetItem* parentItem);
    void createQtProperty(
            Property* property, QTreeWidgetItem* parentItem);
    void refreshAllQtProperties();
    void releaseObjects();

    class Ui_WidgetPropertiesEditor* m_ui = nullptr;

    PropertyOwner* m_currentPropertyOwner = nullptr;
    PropertyOwner* m_currentGpxPropertyOwner = nullptr;
    std::vector<HandleProperty> m_currentVecHndProperty;
    Internal::PropertyItemDelegate* m_itemDelegate = nullptr;
};

} // namespace Mayo

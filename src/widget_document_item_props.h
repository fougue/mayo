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

class QtProperty;
class QtVariantPropertyManager;
class QtVariantProperty;

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
    void connectPropertyValueChangeSignals(bool on);
    void onQVariantPropertyValueChanged(
            QtProperty *qtProp, const QVariant &value);

    void createQtProperties(
            const std::vector<Property*>& properties, QtProperty* parentProp);
    void createQtProperty(
            Property* property, QtProperty* parentProp);
    void mapProperty(QtVariantProperty* qtProp, Property* prop);
    void refreshAllQtProperties();

    struct QtProp_Prop {
        QtVariantProperty* qtProp;
        Property* prop;
    };

    class Ui_WidgetDocumentItemProps* m_ui = nullptr;

    DocumentItem* m_currentDocItem = nullptr;
    GpxDocumentItem* m_currentGpxDocItem = nullptr;
    std::vector<HandleProperty> m_currentVecHndProperty;

    QtVariantPropertyManager* m_varPropMgr = nullptr;
    std::vector<QtProp_Prop> m_vecQtPropProp;
};

} // namespace Mayo

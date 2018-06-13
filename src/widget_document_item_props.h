/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
class GuiApplication;
class GpxDocumentItem;

class WidgetDocumentItemProps : public QWidget
{
public:
    WidgetDocumentItemProps(QWidget* parent = nullptr);
    ~WidgetDocumentItemProps();

    void setGuiApplication(GuiApplication* guiApp);

    void editDocumentItem(DocumentItem* docItem);
    void editProperties(Span<HandleProperty> vecHndProp);

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

    GuiApplication* m_guiApp = nullptr;
    class Ui_WidgetDocumentItemProps* m_ui = nullptr;

    DocumentItem* m_currentDocItem = nullptr;
    GpxDocumentItem* m_currentGpxDocItem = nullptr;
    std::vector<HandleProperty> m_currentVecHndProperty;

    QtVariantPropertyManager* m_varPropMgr = nullptr;
    std::vector<QtProp_Prop> m_vecQtPropProp;
};

} // namespace Mayo

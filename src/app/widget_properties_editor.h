/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_item_delegate.h"
#include "../base/property.h"
#include "../base/span.h"

#include <QtWidgets/QWidget>

namespace Mayo {

// Provides UI edition of properties
class WidgetPropertiesEditor : public QWidget {
public:
    WidgetPropertiesEditor(QWidget* parent = nullptr);
    ~WidgetPropertiesEditor();

    using GroupId = int;
    GroupId addGroup(const QString& name);
    QString groupName(GroupId grpId) const;
    void setGroupName(GroupId grpId, const QString& name);

    void editProperties(PropertyGroup* propGroup, GroupId grpId = -1);
    void editProperty(Property* prop, GroupId grpId = -1);
    void clear();

    void setPropertyEnabled(const Property* prop, bool on);
    void setPropertySelectable(const Property* prop, bool on);

    void addLineSpacer(int height);
    void addLineWidget(QWidget* widget, int height = -1);
    Span<QWidget* const> lineWidgets() const;

    double rowHeightFactor() const;
    void setRowHeightFactor(double v);

    void fitToContents();

    using UnitTranslation = PropertyItemDelegate::UnitTranslation;
    bool overridePropertyUnitTranslation(const BasePropertyQuantity* prop, UnitTranslation unitTr);

private:
    class Private;
    Private* const d = nullptr;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_item_delegate.h"
#include "../base/property.h"
#include "../base/span.h"
#include "../base/unit.h"

#include <QtWidgets/QWidget>

namespace Mayo {

class WidgetPropertiesEditor : public QWidget {
public:
    WidgetPropertiesEditor(QWidget* parent = nullptr);
    ~WidgetPropertiesEditor();

    struct Group;
    Group* addGroup(const QString& name);
    void setGroupName(Group* group, const QString& name);

    void editProperties(PropertyGroup* propGroup, Group* grp = nullptr);
    void editProperty(Property* prop, Group* grp = nullptr);
    void clear();

    void setPropertyEnabled(const Property* prop, bool on);
    void setPropertySelectable(const Property* prop, bool on);

    void addLineSpacer(int height);
    void addLineWidget(QWidget* widget, int height = -1);
    Span<QWidget* const> lineWidgets() const;

    double rowHeightFactor() const;
    void setRowHeightFactor(double v);

    using UnitTranslation = PropertyItemDelegate::UnitTranslation;
    bool overridePropertyUnitTranslation(const BasePropertyQuantity* prop, UnitTranslation unitTr);

private:
    class Private;
    Private* const d = nullptr;
};

} // namespace Mayo

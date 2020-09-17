/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
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

    struct UnitTranslation {
      Unit unit;
      const char* strUnit; // UTF8
      double factor;
    };
    bool overridePropertyUnitTranslation(const BasePropertyQuantity* prop, UnitTranslation unitTr);

private:
    class Private;
    Private* const d = nullptr;
};

} // namespace Mayo

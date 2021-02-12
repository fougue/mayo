/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_properties_editor.h"
#include "ui_widget_properties_editor.h"

#include <vector>

namespace Mayo {

struct WidgetPropertiesEditor::Group {
    QTreeWidgetItem* treeItem;
};

class WidgetPropertiesEditor::Private {
public:
    void createQtProperty(Property* property, QTreeWidgetItem* parentItem);
    QTreeWidgetItem* addLineWidgetItem(QWidget* widget, int height);
    QTreeWidgetItem* findTreeItem(const Property* property) const;
    bool hasGroup(const WidgetPropertiesEditor::Group* group) const;

    Ui_WidgetPropertiesEditor* ui = nullptr;
    PropertyItemDelegate* itemDelegate = nullptr;
    std::vector<Property*> vecProperty;
    std::vector<QWidget*> vecLineWidget;
    std::vector<WidgetPropertiesEditor::Group> vecGroup;
};

WidgetPropertiesEditor::WidgetPropertiesEditor(QWidget *parent)
    : QWidget(parent),
      d(new Private)
{
    d->ui = new Ui_WidgetPropertiesEditor;
    d->ui->setupUi(this);
    d->ui->treeWidget_Browser->setIndentation(0);
    d->ui->treeWidget_Browser->setItemsExpandable(false);
    d->ui->treeWidget_Browser->setRootIsDecorated(false);
    d->itemDelegate = new PropertyItemDelegate(d->ui->treeWidget_Browser);
    d->ui->treeWidget_Browser->setItemDelegate(d->itemDelegate);
}

WidgetPropertiesEditor::~WidgetPropertiesEditor()
{
    delete d->ui;
    delete d;
}

WidgetPropertiesEditor::Group* WidgetPropertiesEditor::addGroup(const QString& name)
{
    Group grp = {};
    grp.treeItem = new QTreeWidgetItem;
    grp.treeItem->setText(0, name);
    grp.treeItem->setFirstColumnSpanned(true);
    d->ui->treeWidget_Browser->addTopLevelItem(grp.treeItem);
    grp.treeItem->setExpanded(true);
    d->vecGroup.push_back(grp);
    return &d->vecGroup.back();
}

void WidgetPropertiesEditor::setGroupName(Group* group, const QString& name)
{
    if (d->hasGroup(group))
        group->treeItem->setText(0, name);
}

void WidgetPropertiesEditor::editProperties(PropertyGroup* propGroup, Group* grp)
{
    if (propGroup) {
        d->ui->stack_Browser->setCurrentWidget(d->ui->page_BrowserDetails);
        QTreeWidgetItem* parentTreeItem = d->hasGroup(grp) ? grp->treeItem : nullptr;
        for (Property* prop : propGroup->properties())
            d->createQtProperty(prop, parentTreeItem);

        d->ui->treeWidget_Browser->resizeColumnToContents(0);
        d->ui->treeWidget_Browser->resizeColumnToContents(1);
    }
}

void WidgetPropertiesEditor::editProperty(Property* prop, Group* grp)
{
    if (prop) {
        d->ui->stack_Browser->setCurrentWidget(d->ui->page_BrowserDetails);
        QTreeWidgetItem* parentTreeItem = d->hasGroup(grp) ? grp->treeItem : nullptr;
        d->createQtProperty(prop, parentTreeItem);
        d->ui->treeWidget_Browser->resizeColumnToContents(0);
        d->ui->treeWidget_Browser->resizeColumnToContents(1);
    }
}

void WidgetPropertiesEditor::clear()
{
    d->vecProperty.clear();
    d->vecLineWidget.clear();
    d->vecGroup.clear();
    d->ui->treeWidget_Browser->clear();
}

void WidgetPropertiesEditor::setPropertyEnabled(const Property* prop, bool on)
{
    QTreeWidgetItem* treeItem = d->findTreeItem(prop);
    if (treeItem) {
        if (on)
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsEnabled);
        else
            treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsEnabled);
    }
}

void WidgetPropertiesEditor::setPropertySelectable(const Property* prop, bool on)
{
    QTreeWidgetItem* treeItem = d->findTreeItem(prop);
    if (treeItem) {
        if (on)
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsSelectable);
        else
            treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsSelectable);
    }
}

void WidgetPropertiesEditor::addLineSpacer(int height)
{
    auto widget = new QWidget;
    d->addLineWidgetItem(widget, height);
}

void WidgetPropertiesEditor::addLineWidget(QWidget* widget, int height)
{
    d->addLineWidgetItem(widget, height);
}

Span<QWidget* const> WidgetPropertiesEditor::lineWidgets() const
{
    return d->vecLineWidget;
}

double WidgetPropertiesEditor::rowHeightFactor() const
{
    return d->itemDelegate->rowHeightFactor();
}

void WidgetPropertiesEditor::setRowHeightFactor(double v)
{
    d->itemDelegate->setRowHeightFactor(v);
}

bool WidgetPropertiesEditor::overridePropertyUnitTranslation(
        const BasePropertyQuantity* prop, UnitTranslation unitTr)
{
    return d->itemDelegate->overridePropertyUnitTranslation(prop, unitTr);
}

void WidgetPropertiesEditor::Private::createQtProperty(
        Property* property, QTreeWidgetItem* parentItem)
{
    this->vecProperty.push_back(property);
    auto itemProp = new QTreeWidgetItem;
    const QString labelSpacer = parentItem ? "       " : "";
    itemProp->setText(0, labelSpacer + property->label());
    itemProp->setData(1, Qt::DisplayRole, QVariant::fromValue<Property*>(property));
    itemProp->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    if (parentItem)
        parentItem->addChild(itemProp);
    else
        this->ui->treeWidget_Browser->addTopLevelItem(itemProp);
}

QTreeWidgetItem* WidgetPropertiesEditor::Private::addLineWidgetItem(QWidget* widget, int height)
{
    widget->setAutoFillBackground(true);
    auto treeItem = new QTreeWidgetItem(this->ui->treeWidget_Browser);
    treeItem->setFlags(Qt::ItemIsEnabled);
    if (height > 0) {
        treeItem->setSizeHint(0, QSize(100, height));
        treeItem->setSizeHint(1, QSize(100, height));
    }
    this->vecLineWidget.push_back(widget);
    this->ui->treeWidget_Browser->setFirstItemColumnSpanned(treeItem, true);
    this->ui->treeWidget_Browser->setItemWidget(treeItem, 0, widget);
    return treeItem;
}

QTreeWidgetItem* WidgetPropertiesEditor::Private::findTreeItem(const Property* property) const
{
    for (QTreeWidgetItemIterator it(this->ui->treeWidget_Browser); *it; ++it) {
        QTreeWidgetItem* treeItem = *it;
        const QVariant value = treeItem->data(1, Qt::DisplayRole);
        if (value.canConvert<Property*>()
                && qvariant_cast<Property*>(value) == property)
        {
            return treeItem;
        }
    }
    return nullptr;
}

bool WidgetPropertiesEditor::Private::hasGroup(const Group *group) const
{
    return group
            && group->treeItem
            && group->treeItem->treeWidget() == ui->treeWidget_Browser;
}

} // namespace Mayo

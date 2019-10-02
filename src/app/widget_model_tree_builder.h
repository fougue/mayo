/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include <vector>
#include <QtCore/QString>
class QAction;
class QObject;
class QSettings;
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

class Document;
class DocumentItem;
struct DocumentItemNode;

// TODO Rename Builder -> Extension ?
class WidgetModelTreeBuilder {
public:
    virtual ~WidgetModelTreeBuilder();

    virtual bool supports(const Document*) const { return true; }
    virtual bool supports(const DocumentItem*) const { return true; }

    virtual void refreshTextTreeItem(const Document* doc, QTreeWidgetItem* treeItem);
    virtual void refreshTextTreeItem(const DocumentItem* docItem, QTreeWidgetItem* treeItem);
    virtual void refreshTextTreeItem(const DocumentItemNode& node, QTreeWidgetItem* treeItemDocItem);

    virtual void fillTreeItem(QTreeWidgetItem* treeItem, Document* doc);
    virtual void fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem);

    QTreeWidget* treeWidget() const { return m_treeWidget; }
    void setTreeWidget(QTreeWidget* tree) { m_treeWidget = tree; }

    virtual void loadConfiguration(const QSettings* settings, const QString& keyGroup);
    virtual void saveConfiguration(QSettings* settings, const QString& keyGroup);

    virtual std::vector<QAction*> createConfigurationActions(QObject* parent);

    virtual WidgetModelTreeBuilder* clone() const;

protected:
    static QString labelText(const PropertyQString& propLabel);

private:
    QTreeWidget* m_treeWidget = nullptr;
};

} // namespace Mayo

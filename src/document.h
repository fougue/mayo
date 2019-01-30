/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application.h"
#include "property_builtins.h"
#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class DocumentItem;

class Document : public PropertyOwnerSignals {
    Q_OBJECT
public:
    Document(QObject* parent = Application::instance());
    virtual ~Document();

    const QString& label() const;
    void setLabel(const QString& v);

    const QString& filePath() const;
    void setFilePath(const QString& filepath);

    void addRootItem(DocumentItem* item);
    bool eraseRootItem(DocumentItem* docItem);

    Span<DocumentItem* const> rootItems() const;
    bool isEmpty() const;

    PropertyQString propertyLabel;
    PropertyQString propertyFilePath;

    static const char TypeName[];
    virtual const char* dynTypeName() const;

signals:
    void itemAdded(DocumentItem* docItem);
    void itemErased(const DocumentItem* docItem);
    void itemPropertyChanged(DocumentItem* docItem, Property* prop);

private:
    friend class Application;
    friend class DocumentItem;

    std::vector<DocumentItem*> m_rootItems;
};

} // namespace Mayo

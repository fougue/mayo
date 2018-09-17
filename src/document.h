/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_builtins.h"
#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Application;
class DocumentItem;
class PartItem;
class Property;

class Document : public QObject, public PropertyOwner {
    Q_OBJECT
public:
    const Application* application() const;
    Application* application();

    const QString& label() const;
    void setLabel(const QString& v);

    const QString& filePath() const;
    void setFilePath(const QString& filepath);

    bool eraseRootItem(DocumentItem* docItem);

    const std::vector<DocumentItem*>& rootItems() const;
    bool isEmpty() const;

    PropertyQString propertyLabel;
    PropertyQString propertyFilePath;

signals:
    void itemAdded(DocumentItem* docItem);
    void itemErased(const DocumentItem* docItem);
    void itemPropertyChanged(const DocumentItem* docItem, const Property* prop);

private:
    friend class Application;
    friend class DocumentItem;
    Document(Application* app);
    ~Document();

    void addRootItem(DocumentItem* item);

    Application* m_app = nullptr;
    std::vector<DocumentItem*> m_rootItems;
};

} // namespace Mayo

#pragma once

#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Application;
class DocumentItem;
class PartItem;
class Property;

class Document : public QObject
{
    Q_OBJECT

public:
    const Application* application() const;
    Application* application();

    const QString& label() const;
    void setLabel(const QString& v);

    bool eraseRootItem(DocumentItem* docItem);

    const std::vector<DocumentItem*>& rootItems() const;
    bool isEmpty() const;

signals:
    void itemAdded(DocumentItem* docItem);
    void itemErased(const DocumentItem* docItem);
    void itemPropertyChanged(const DocumentItem* docItem, const Property* prop);

private:
    friend class Application;
    friend class DocumentItem;
    Document(Application* app);
    ~Document();

    void addPartItem(PartItem* part);

    Application* m_app = nullptr;
    std::vector<DocumentItem*> m_rootItems;
    QString m_label;
};

} // namespace Mayo

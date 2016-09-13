#pragma once

#include <QtCore/QObject>
#include <atomic>
#include <vector>

namespace qttask { class Progress; }

namespace Mayo {

class Application;
class DocumentItem;
class PartItem;
class Property;

class Document : public QObject
{
    Q_OBJECT

public:
    enum class PartFormat
    {
        Unknown,
        Iges,
        Step,
        OccBrep,
        Stl
    };

    const Application* application() const;
    Application* application();

    const QString& label() const;
    void setLabel(const QString& v);

    bool import(
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    bool eraseRootItem(DocumentItem* docItem);

    static const std::vector<PartFormat>& partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();

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

    bool importIges(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStep(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importOccBRep(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStl(
            const QString& filepath, qttask::Progress* progress = nullptr);

    Application* m_app = nullptr;
    std::vector<DocumentItem*> m_rootItems;
    QString m_label;
};

} // namespace Mayo

#pragma once

#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;
class GuiDocument;

class GuiApplication : public QObject
{
    Q_OBJECT

public:
    GuiApplication(QObject* parent = nullptr);

    GuiDocument* findGuiDocument(const Document* doc) const;

signals:
    void guiDocumentAdded(GuiDocument* guiDoc);
    void guiDocumentErased(const GuiDocument* guiDoc);

protected:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);

private:
    struct Doc_GuiDoc {
        Document* doc;
        GuiDocument* guiDoc;
    };

    std::vector<Doc_GuiDoc> m_vecDocGuiDoc;
};

} // namespace Mayo

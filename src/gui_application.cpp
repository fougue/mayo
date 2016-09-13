#include "gui_application.h"

#include "application.h"
#include "document.h"
#include "gui_document.h"

namespace Mayo {

GuiApplication::GuiApplication(QObject *parent)
    : QObject(parent)
{
    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &GuiApplication::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &GuiApplication::onDocumentErased);
}

GuiDocument *GuiApplication::findGuiDocument(const Document *doc) const
{
    auto itFound = std::find_if(
                m_vecDocGuiDoc.cbegin(),
                m_vecDocGuiDoc.cend(),
                [=](const Doc_GuiDoc& pair) { return pair.doc == doc; });
    return itFound != m_vecDocGuiDoc.cend() ? itFound->guiDoc : nullptr;
}

void GuiApplication::onDocumentAdded(Document *doc)
{
    const Doc_GuiDoc pair = { doc, new GuiDocument(doc) }; // TODO: set container widget
    m_vecDocGuiDoc.emplace_back(std::move(pair));
    emit guiDocumentAdded(pair.guiDoc);
}

void GuiApplication::onDocumentErased(const Document *doc)
{
    auto itFound = std::find_if(
                m_vecDocGuiDoc.begin(),
                m_vecDocGuiDoc.end(),
                [=](const Doc_GuiDoc& pair) { return pair.doc == doc; });
    if (itFound != m_vecDocGuiDoc.end()) {
        const GuiDocument* guiDoc = itFound->guiDoc;
        delete guiDoc;
        m_vecDocGuiDoc.erase(itFound);
        emit guiDocumentErased(guiDoc);
    }
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_context.h"

#include "../gui/gui_application.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widget_gui_document.h"
#include "widget_main_control.h"
#include "widget_main_home.h"

#include <cassert>

#include <QtCore/QtDebug>

namespace Mayo {

AppContext::AppContext(MainWindow* wnd)
    : IAppContext(wnd),
      m_wnd(wnd)
{
    assert(m_wnd != nullptr);
    assert(m_wnd->widgetPageDocuments() != nullptr);

    QObject::connect(
        m_wnd->widgetPageDocuments(), &WidgetMainControl::currentDocumentIndexChanged,
        this, &AppContext::onCurrentDocumentIndexChanged
    );
}

GuiApplication* AppContext::guiApp() const
{
    return m_wnd->m_guiApp;
}

TaskManager* AppContext::taskMgr() const
{
    return &m_wnd->m_taskMgr;
}

QWidget* AppContext::pageDocuments_widgetLeftSideBar() const
{
    const WidgetMainControl* pageDocs = m_wnd->widgetPageDocuments();
    return pageDocs ? pageDocs->widgetLeftSideBar() : nullptr;
}

QWidget* AppContext::widgetMain() const
{
    return m_wnd;
}

QWidget* AppContext::widgetPage(Page page) const
{
    if (page == Page::Home)
        return m_wnd->widgetPageHome();
    else if (page == Page::Documents)
        return m_wnd->widgetPageDocuments();
    else
        return nullptr;
}

IAppContext::Page AppContext::currentPage() const
{
    auto widget = m_wnd->m_ui->stack_Main->currentWidget();
    if (widget == m_wnd->widgetPageHome())
        return Page::Home;
    else if (widget == m_wnd->widgetPageDocuments())
        return Page::Documents;

    return Page::Unknown;
}

void AppContext::setCurrentPage(Page page)
{
    QWidget* widgetPage = this->widgetPage(page);
    assert(widgetPage);
    m_wnd->m_ui->stack_Main->setCurrentWidget(widgetPage);
}

V3dViewController* AppContext::v3dViewController(const GuiDocument* guiDoc) const
{
    auto widgetDoc = this->findWidgetGuiDocument([=](WidgetGuiDocument* candidate) {
        return candidate->guiDocument() == guiDoc;
    });
    return widgetDoc ? widgetDoc->controller() : nullptr;
}

int AppContext::findDocumentIndex(Document::Identifier docId) const
{
    const int guiDocumentCount = int(this->guiApp()->guiDocuments().size());
    for (int i = 0; i < guiDocumentCount; ++i) {
        if (GuiDocument::documentIdentifier(this->guiDocument(i)) == docId)
            return i;
    }

    return -1;
}

Document::Identifier AppContext::findDocumentFromIndex(int index) const
{
    return GuiDocument::documentIdentifier(this->guiDocument(index));
}

Document::Identifier AppContext::currentDocument() const
{
    const int index = m_wnd->widgetPageDocuments()->currentDocumentIndex();
    return GuiDocument::documentIdentifier(this->guiDocument(index));
}

void AppContext::setCurrentDocument(Document::Identifier docId)
{
    auto widgetDoc = this->findWidgetGuiDocument([=](WidgetGuiDocument* widgetDoc) {
        return widgetDoc->documentIdentifier() == docId;
    });
    const int docIndex = m_wnd->widgetPageDocuments()->indexOfWidgetGuiDocument(widgetDoc);
    m_wnd->widgetPageDocuments()->setCurrentDocumentIndex(docIndex);
}

void AppContext::updateControlsEnabledStatus()
{
    m_wnd->updateControlsActivation();
}

GuiDocument* AppContext::guiDocument(int idx) const
{
    auto spanGuiDocuments = this->guiApp()->guiDocuments();
    if (0 <= idx && idx < spanGuiDocuments.size())
        return spanGuiDocuments[idx];

    return nullptr;
}

WidgetGuiDocument* AppContext::widgetGuiDocument(int idx) const
{
    return m_wnd->widgetPageDocuments()->widgetGuiDocument(idx);
}

WidgetGuiDocument* AppContext::findWidgetGuiDocument(std::function<bool(WidgetGuiDocument*)> fn) const
{
    const int widgetCount = m_wnd->widgetPageDocuments()->widgetGuiDocumentCount();
    for (int i = 0; i < widgetCount; ++i) {
        auto candidate = this->widgetGuiDocument(i);
        if (candidate && fn(candidate))
            return candidate;
    }

    return nullptr;
}

void AppContext::onCurrentDocumentIndexChanged(int docIndex)
{
    auto guiDoc = this->guiDocument(docIndex);
    emit this->currentDocumentChanged(GuiDocument::documentIdentifier(guiDoc));
}

} // namespace Mayo

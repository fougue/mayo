/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_context.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widget_gui_document.h"

namespace Mayo {

AppContext::AppContext(MainWindow* wnd)
    : m_wnd(wnd)
{
    QObject::connect(
                m_wnd->m_ui->combo_GuiDocuments, qOverload<int>(&QComboBox::currentIndexChanged),
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

QWidget* AppContext::widgetMain() const
{
    return m_wnd;
}

QWidget* AppContext::widgetLeftSidebar() const
{
    return m_wnd->m_ui->widget_Left;
}

IAppContext::ModeWidgetMain AppContext::modeWidgetMain() const
{
    auto widget = m_wnd->m_ui->stack_Main->currentWidget();
    if (widget == m_wnd->m_ui->page_MainHome)
        return ModeWidgetMain::Home;
    else if (widget == m_wnd->m_ui->page_MainControl)
        return ModeWidgetMain::Documents;

    return ModeWidgetMain::Unknown;
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
    int index = -1;
    auto widgetDoc = this->findWidgetGuiDocument([&](WidgetGuiDocument* candidate) {
            ++index;
            return candidate->documentIdentifier() == docId;
    });
    return widgetDoc ? index : -1;
}

Document::Identifier AppContext::findDocumentFromIndex(int index) const
{
    auto widgetDoc = m_wnd->widgetGuiDocument(index);
    return widgetDoc ? widgetDoc->documentIdentifier() : -1;
}

Document::Identifier AppContext::currentDocument() const
{
    const int index = m_wnd->m_ui->combo_GuiDocuments->currentIndex();
    auto widgetDoc = m_wnd->widgetGuiDocument(index);
    return widgetDoc ? widgetDoc->documentIdentifier() : -1;
}

void AppContext::setCurrentDocument(Document::Identifier docId)
{
    auto widgetDoc = this->findWidgetGuiDocument([=](WidgetGuiDocument* widgetDoc) {
        return widgetDoc->documentIdentifier() == docId;
    });
    const int docIndex = m_wnd->m_ui->stack_GuiDocuments->indexOf(widgetDoc);
    m_wnd->m_ui->combo_GuiDocuments->setCurrentIndex(docIndex);
}

void AppContext::updateControlsEnabledStatus()
{
    m_wnd->updateControlsActivation();
}

void AppContext::deleteDocumentWidget(const DocumentPtr& doc)
{
    QWidget* widgetDoc = this->findWidgetGuiDocument([&](WidgetGuiDocument* widgetDoc) {
            return widgetDoc->documentIdentifier() == doc->identifier();
    });
    if (widgetDoc) {
        m_wnd->m_ui->stack_GuiDocuments->removeWidget(widgetDoc);
        widgetDoc->deleteLater();
    }
}

WidgetGuiDocument* AppContext::findWidgetGuiDocument(std::function<bool(WidgetGuiDocument*)> fn) const
{
    const int widgetCount = m_wnd->m_ui->stack_GuiDocuments->count();
    for (int i = 0; i < widgetCount; ++i) {
        auto candidate = m_wnd->widgetGuiDocument(i);
        if (candidate && fn(candidate))
            return candidate;
    }

    return nullptr;
}

void AppContext::onCurrentDocumentIndexChanged(int docIndex)
{
    auto widgetDoc = m_wnd->widgetGuiDocument(docIndex);
    emit this->currentDocumentChanged(widgetDoc ? widgetDoc->documentIdentifier() : -1);
}

} // namespace Mayo

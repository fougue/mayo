/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../base/filepath.h"
#include "../base/text_id.h"

#include <QtCore/QObject>
#include <QAction> // WARNING Qt5 <QtWidgets/...> / Qt6 <QtGui/...>
class QWidget;

namespace Mayo {

class Application;
class GuiApplication;
class GuiDocument;
class V3dViewController;
class TaskManager;

// Provides interface to access/interact with application
class IAppContext : public QObject {
    Q_OBJECT
public:
    enum class ModeWidgetMain { Unknown, Home, Documents };

    IAppContext(QObject* parent = nullptr);

    virtual GuiApplication* guiApp() const = 0;
    virtual TaskManager* taskMgr() const = 0;
    virtual QWidget* widgetMain() const = 0;
    virtual QWidget* widgetLeftSidebar() const = 0;
    virtual ModeWidgetMain modeWidgetMain() const = 0;
    virtual V3dViewController* v3dViewController(const GuiDocument* guiDoc) const = 0;

    virtual Document::Identifier currentDocument() const = 0;
    virtual void setCurrentDocument(Document::Identifier docId) = 0;

    virtual int findDocumentIndex(Document::Identifier docId) const = 0;
    virtual Document::Identifier findDocumentFromIndex(int index) const = 0;

    virtual void updateControlsEnabledStatus() = 0;
    virtual void deleteDocumentWidget(const DocumentPtr& doc) = 0;

signals:
    void currentDocumentChanged(Mayo::Document::Identifier docId);
};

// Represents a single action in the application
class Command : public QObject {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Command)
public:
    Command(IAppContext* context);
    virtual ~Command() = default;

    virtual void execute() = 0;

    IAppContext* context() const { return m_context; }

    QAction* action() const { return m_action; }
    virtual bool getEnabledStatus() const { return true; }

protected:
    Application* app() const;
    GuiApplication* guiApp() const { return m_context->guiApp(); }
    TaskManager* taskMgr() const { return m_context->taskMgr(); }
    QWidget* widgetMain() const { return m_context->widgetMain(); }
    Document::Identifier currentDocument() const { return m_context->currentDocument(); }
    GuiDocument* currentGuiDocument() const;
    int currentDocumentIndex() const;

    void setCurrentDocument(const DocumentPtr& doc);
    void setAction(QAction* action);

private:
    IAppContext* m_context = nullptr;
    QAction* m_action = nullptr;
};

} // namespace Mayo

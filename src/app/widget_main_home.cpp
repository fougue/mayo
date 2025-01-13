/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_main_home.h"
#include "ui_widget_main_home.h"

#include "../qtcommon/filepath_conv.h"
#include "commands_file.h"

namespace Mayo {

WidgetMainHome::WidgetMainHome(QWidget* parent)
    : IWidgetMainPage(parent),
      m_ui(new Ui::WidgetMainHome)
{
    m_ui->setupUi(this);
}

WidgetMainHome::~WidgetMainHome()
{
    delete m_ui;
}

void WidgetMainHome::initialize(const CommandContainer* cmdContainer)
{
    assert(cmdContainer != nullptr);
    IAppContext* appContext = cmdContainer->appContext();

    QObject::connect(
        m_ui->widget_HomeFiles, &WidgetHomeFiles::newDocumentRequested,
        cmdContainer->findCommand(CommandNewDocument::Name), &Command::execute
    );
    QObject::connect(
        m_ui->widget_HomeFiles, &WidgetHomeFiles::openDocumentsRequested,
        cmdContainer->findCommand(CommandOpenDocuments::Name), &Command::execute
    );
    QObject::connect(
        m_ui->widget_HomeFiles, &WidgetHomeFiles::recentFileOpenRequested,
        this, [=](const QFileInfo& fp) { FileCommandTools::openDocument(appContext, filepathFrom(fp)); }
    );
}

void WidgetMainHome::updatePageControlsActivation()
{
}

} // namespace Mayo

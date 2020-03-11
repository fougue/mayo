/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QStringListModel>
#include <vector>

namespace Mayo {

class GuiApplication;
class GuiDocument;

class GuiDocumentListModel : public QStringListModel {
public:
    GuiDocumentListModel(GuiApplication* app);

    QVariant data(const QModelIndex& index, int role) const override;

private:
    void appendGuiDocument(const GuiDocument* guiDoc);
    void removeGuiDocument(const GuiDocument* guiDoc);

    std::vector<const GuiDocument*> m_vecGuiDocument;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QStringListModel>
#include <vector>

namespace Mayo {

class Application;
class Document;

class DocumentListModel : public QStringListModel {
public:
    DocumentListModel(Application* app);

    QVariant data(const QModelIndex& index, int role) const override;

private:
    void appendDocument(const Document* doc);
    void removeDocument(const Document* doc);

    std::vector<const Document*> m_docs;
};

} // namespace Mayo

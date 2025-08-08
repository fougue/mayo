/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/io_format.h"
#include "../base/property.h"
#include "../base/signal.h"

#include "script_tree_node.h"

#include <QtCore/QObject>
#include <QtCore/QJsonValue>
#include <QtQml/QJSValue>

#include <functional>

// Notes
//     Pour accéder aux éléments de l'assemblage il faut passer par TDF_Label
//     * Dans l'environnement JS on peut employer une représentation string obtenue par CafUtils::labelTag()
//       La valeur TDF_Label d'origine est retrouvée via TDF_Tool::Label() prenant en 1er argument
//       TDF_Data. Cet objet est contenu dans TDocStd_Document (voir function GetData()). TDF_Data utilise
//       en interne une map string->TDF_Label servant de cache. La fonction TDF_Data::RegisterLabel() doit
//       éventuellement être appelée explicitement (à déterminer)
//     * Au lieu d'une représentation string, il doit aussi être possible d'utiliser TreeNodeId qui est
//       simplement un alias de int.
//       TreeNodeId serait employé pour le parcours de l'arborescence assemblage (dans le Document)
//       Il faudra tout de même manipuler les données XCAF via TDF_Label (voir XCaf)
//       Cette 2ème solution semble la plus appropriée
//
// Fonctions :
//     ScriptDocument::entityCount()
//     ScriptDocument::entityTreeNodeId(int index) -> TreeNodeId(int)
//     ScriptDocument::label(TreeNodeId nodeId) -> string(taglist)
//     ScriptDocument::isShape(string tags) -> bool
//     ScriptDocument::isShapeAssembly(string tags) -> bool
//     ScriptDocument::isShapeReference(string tags) -> bool
//     ScriptDocument::isShapeSimple(string tags) -> bool
//     ScriptDocument::isShapeSub(string tags) -> bool
//     ScriptDocument::productShapeLabel(string tags) -> string(taglist)
//     ScriptDocument::hasShapeColor(string tags) -> bool
//     ScriptDocument::shapeColor(string tags) -> string("#RRGGBBAA")

namespace Mayo {

class ScriptApplication;

class ScriptDocument : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(int entityCount READ entityCount NOTIFY entityCountChanged)
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::ScriptDocument)
public:
    int id() const;

    QString name() const;
    void setName(const QString& str);

    QString filePath() const;
    void setFilePath(const QString& str);

    int entityCount() const;
    unsigned entityTreeNodeId(int index) const;

    Q_INVOKABLE void traverseModelTree(QJSValue fn);
    Q_INVOKABLE QVariant_ScriptTreeNode treeNode(unsigned treeNodeId) const;
#if 0
    Q_INVOKABLE bool tagHasShapeColor(const QString& tag) const;
    Q_INVOKABLE QColor tagShapeColor(const QString& tag) const;
#endif

    Q_INVOKABLE void traverseShape(QJSValue shape, ScriptShapeType shapeTypeFilter, QJSValue fn);

    // TODO Provide both sync/async importFile functions?
    //Q_INVOKABLE bool importFile(QString strFilepath, QJSValue jsonOptions, QJSValue fnCallbacks);
    Q_INVOKABLE quint32 asyncImportFile(QString strFilepath, QJSValue jsonOptions, QJSValue fnCallbacks);

signals:
    void nameChanged();
    void filePathChanged();
    void entityCountChanged();

private:
    ScriptDocument(const DocumentPtr& doc, ScriptApplication* jsApp);

    const DocumentPtr& baseDocument() const { return m_doc; }

    std::unique_ptr<PropertyGroup> createReaderParametersFromJson(
        const QJsonValue& jsonOptions,
        IO::Format format,
        const std::function<void(std::string_view)>& errorCallback = nullptr
    ) const;

    friend class ScriptApplication;
    ScriptApplication* m_jsApp = nullptr;
    DocumentPtr m_doc;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo

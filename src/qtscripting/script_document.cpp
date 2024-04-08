/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_document.h"

#include "../base/brep_utils.h"
#include "../base/document.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "../app/qtgui_utils.h"
#include "script_application.h"
#include "script_shape.h"
#include "script_tree_node.h"

#include <TDF_Tool.hxx>

#include <QtQml/QJSEngine>
#include <QtCore/QtDebug>

namespace Mayo {

int ScriptDocument::id() const
{
    return m_doc ? m_doc->identifier() : -1;
}

QString ScriptDocument::name() const
{
    return m_doc ? to_QString(m_doc->name()) : QString{};
}

void ScriptDocument::setName(const QString& str)
{
    if (m_doc)
        m_doc->setName(to_stdString(str));
}

QString ScriptDocument::filePath() const
{
    return m_doc ? filepathTo<QString>(m_doc->filePath()) : QString{};
}

int ScriptDocument::entityCount() const
{
    return m_doc ? m_doc->entityCount() : 0;
}

unsigned ScriptDocument::entityTreeNodeId(int index) const
{
    return m_doc ? m_doc->entityTreeNodeId(index) : 0;
}

void ScriptDocument::traverseModelTree(QJSValue fn)
{
    if (!fn.isCallable())
        return;

    const Tree<TDF_Label>& modelTree = m_doc->modelTree();
    traverseTree(modelTree, [&](TreeNodeId nodeId) {
#if 0
        const TreeNodeId parentNodeId = modelTree.nodeParent(nodeId);
        if (parentNodeId != 0) {
            const TDF_Label& parentNodeLabel = modelTree.nodeData(parentNodeId);
            if (XCaf::isShapeReference(parentNodeLabel))
                return; // Skip: tree node is a product(or "referred" shape)
        }
#endif
        fn.call({ QJSValue{nodeId} });
    });
}

QColor ScriptDocument::tagShapeColor(const QString& tag) const
{
    if (!m_doc)
        return {};

    TDF_Label label;
    TDF_Tool::Label(m_doc->GetData(), to_OccAsciiString(tag), label);
    if (label.IsNull())
        return {};

    return QtGuiUtils::toQColor(m_doc->xcaf().shapeColor(label));
}

QVariant ScriptDocument::treeNode(unsigned int treeNodeId) const
{
    return QVariant::fromValue(ScriptTreeNode(m_doc, treeNodeId));
}

void ScriptDocument::traverseShape(QJSValue shape, unsigned shapeTypeFilter, QJSValue fn)
{
    if (!fn.isCallable())
        return;

    const auto shapeTypeEnum = static_cast<TopAbs_ShapeEnum>(shapeTypeFilter);
    const auto scriptShape = m_jsApp->jsEngine()->fromScriptValue<ScriptShape>(shape);
    BRepUtils::forEachSubShape(scriptShape.shape(), shapeTypeEnum, [&](const TopoDS_Shape& subShape) {
        auto jsSubShape = m_jsApp->jsEngine()->toScriptValue(ScriptShape(subShape));
        fn.call({ jsSubShape });
    });
}

ScriptDocument::ScriptDocument(const DocumentPtr& doc, ScriptApplication* jsApp)
    : QObject(jsApp),
      m_jsApp(jsApp),
      m_doc(doc)
{
    m_sigConns
        << doc->signalNameChanged.connectSlot([=](const std::string&) { emit this->nameChanged(); })
        << doc->signalFilePathChanged.connectSlot([=](const FilePath&) { emit this->filePathChanged(); })
        << doc->signalEntityAdded.connectSlot([=](TreeNodeId) { emit this->entityCountChanged(); })
        << doc->signalEntityAboutToBeDestroyed.connectSlot([=](TreeNodeId) { emit this->entityCountChanged(); })
        ;
}

} // namespace Mayo

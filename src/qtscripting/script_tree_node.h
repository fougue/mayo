/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/document_ptr.h"
#include "../base/libtree.h"

#include <QtCore/QObject>

namespace Mayo {

class ScriptTreeNode {
    Q_GADGET
    Q_PROPERTY(unsigned id READ id)
    Q_PROPERTY(unsigned parentId READ parentId)
    Q_PROPERTY(QString tag READ tag)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool isAssembly READ isAssembly)
    Q_PROPERTY(bool isInstance READ isInstance)
    Q_PROPERTY(bool isProduct READ isProduct)
    Q_PROPERTY(bool isComponent READ isComponent)
    Q_PROPERTY(bool hasSubShapes READ hasSubShapes)
    Q_PROPERTY(QStringList subShapeTags READ subShapeTags)
    Q_PROPERTY(QVariant shape READ shape)
public:
    ScriptTreeNode() = default;
    ScriptTreeNode(const DocumentPtr& doc, TreeNodeId nodeId);

    unsigned id() const { return m_nodeId; }
    unsigned parentId() const;

    QString tag() const; // TODO Could be TDF_Label wrapped in QVariant
    QString name() const;

    bool isAssembly() const;
    bool isInstance() const;
    bool isProduct() const;
    bool isComponent() const;
    bool hasSubShapes() const;

    QStringList subShapeTags() const;

    QVariant shape() const; // ->ScriptShape

private:
    DocumentPtr m_doc;
    TreeNodeId m_nodeId = 0;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptTreeNode)

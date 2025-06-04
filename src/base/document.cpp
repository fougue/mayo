/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document.h"

#include "application.h"
#include "caf_utils.h"
#include "cpp_utils.h"

#include <TDF_ChildIterator.hxx>
#include <TDF_TagSource.hxx>
#include <XCAFDoc_DocumentTool.hxx>

#include <unordered_set>

namespace Mayo {

Document::Document(const ApplicationPtr& app)
    : TDocStd_Document(NameFormatBinary),
      m_app(app)
{
    TDF_TagSource::Set(this->rootLabel());
}

Document::~Document()
{
}

void Document::initXCaf()
{
    m_xcaf.setLabelMain(this->Main());
    m_xcaf.setModelTree(m_modelTree);
}

const std::string& Document::name() const
{
    return m_name;
}

void Document::setName(std::string_view name)
{
    m_name = name;
    this->signalNameChanged.send(m_name);
}

const FilePath& Document::filePath() const
{
    return m_filePath;
}

void Document::setFilePath(const FilePath& fp)
{
    m_filePath = fp;
    this->signalFilePathChanged.send(fp);
}

const char* Document::toNameFormat(Document::Format format)
{
    switch (format) {
    case Format::Binary: return Document::NameFormatBinary;
    case Format::Xml: return Document::NameFormatXml;
    }

    return nullptr;
}

const char Document::NameFormatBinary[] = "BinDocMayo";
const char Document::NameFormatXml[] = "XmlDocMayo";
const char Document::TypeName[] = "Mayo::Document";

bool Document::isXCafDocument() const
{
    return XCAFDoc_DocumentTool::IsXCAFDocument(this);
}

TDF_Label Document::rootLabel() const
{
    return this->GetData()->Root();
}

bool Document::isEntity(TreeNodeId nodeId)
{
    return m_modelTree.nodeIsRoot(nodeId);
}

int Document::entityCount() const
{
    return CppUtils::safeStaticCast<int>(m_modelTree.roots().size());
}

TDF_Label Document::entityLabel(int index) const
{
    return m_modelTree.nodeData(this->entityTreeNodeId(index));
}

TreeNodeId Document::entityTreeNodeId(int index) const
{
    return m_modelTree.roots()[index];
}

DocumentTreeNode Document::entityTreeNode(int index) const
{
    return { DocumentPtr(this), this->entityTreeNodeId(index) };
}

void Document::rebuildModelTree()
{
    m_modelTree.clear();
    const bool xcafIsNull = m_xcaf.isNull();
    if (!xcafIsNull) {
        for (const TDF_Label& label : m_xcaf.topLevelFreeShapes())
            m_xcaf.deepBuildAssemblyTree(0, label);
    }

    constexpr bool allLevels = true;
    for (TDF_ChildIterator it(this->rootLabel(), !allLevels); it.More(); it.Next()) {
        const TDF_Label childLabel = it.Value();
        if (!CafUtils::isNullOrEmpty(childLabel)
                && (xcafIsNull || childLabel != this->Main())) // Not XCAF Main label
        {
            m_modelTree.appendChild(0, childLabel);
        }
    }
}

DocumentPtr Document::findFrom(const TDF_Label& label)
{
    return DocumentPtr::DownCast(TDocStd_Document::Get(label));
}

TDF_Label Document::newEntityLabel()
{
    OccHandle<TDF_TagSource> tagSrc = CafUtils::findAttribute<TDF_TagSource>(this->rootLabel());
    Expects(!tagSrc.IsNull());
    if (tagSrc->Get() == 0)
        this->rootLabel().NewChild(); // Reserve label 0:1 for XCAF Main()

    return this->rootLabel().NewChild();
}

TDF_Label Document::newEntityShapeLabel()
{
    return m_xcaf.shapeTool()->NewShape();
}

TreeNodeId Document::findEntity(const TDF_Label& label) const
{
    for (int i = 0; i < this->entityCount(); ++i) {
        if (this->entityLabel(i) == label)
            return this->entityTreeNodeId(i);
    }

    return 0;
}

bool Document::containsLabel(const TDF_Label& label) const
{
    return Document::findFrom(label).get() == this;
}

void Document::addEntityTreeNode(const TDF_Label& label)
{
    // TODO Allow custom population of the model tree for the new entity
    if (this->containsLabel(label) && this->findEntity(label) == 0) {
        const TreeNodeId nodeId = m_xcaf.deepBuildAssemblyTree(0, label);
        this->signalEntityAdded.send(nodeId);
    }
}

void Document::addEntityTreeNodeSequence(const TDF_LabelSequence& seqLabel)
{
    std::vector<TreeNodeId> vecTreeNodeId;
    vecTreeNodeId.reserve(seqLabel.Size());
    for (const TDF_Label& label : seqLabel) {
        if (this->containsLabel(label) && this->findEntity(label) == 0) {
            const TreeNodeId treeNodeId = m_xcaf.deepBuildAssemblyTree(0, label);
            vecTreeNodeId.push_back(treeNodeId);
        }
    }

    for (TreeNodeId treeNodeId : vecTreeNodeId)
        this->signalEntityAdded.send(treeNodeId);
}

void Document::destroyEntity(TreeNodeId entityTreeNodeId)
{
    Expects(this->modelTree().nodeIsRoot(entityTreeNodeId));

    TDF_Label entityLabel = m_modelTree.nodeData(entityTreeNodeId);
    if (CafUtils::isNullOrEmpty(entityLabel))
        return;

    this->signalEntityAboutToBeDestroyed.send(entityTreeNodeId);

    std::unordered_set<TDF_Label> setSimpleShapeLabel;
    traverseTree_postOrder(entityTreeNodeId, m_modelTree, [&](TreeNodeId nodeId) {
        TDF_Label nodeLabel = m_modelTree.nodeData(nodeId);
        if (XCaf::isShapeSimple(nodeLabel))
            setSimpleShapeLabel.insert(nodeLabel);
        else if (XCaf::isShapeComponent(nodeLabel))
            m_xcaf.shapeTool()->RemoveComponent(nodeLabel);
        else if (XCaf::isShapeReference(nodeLabel))
            m_xcaf.shapeTool()->RemoveShape(nodeLabel, false/*!removeCompletely*/);
        else
            nodeLabel.ForgetAllAttributes();
    });

    for (const TDF_Label& label : setSimpleShapeLabel) {
        if (!XCaf::hasShapeUsers(label))
            m_xcaf.shapeTool()->RemoveShape(label, true/*removeCompletely*/);
    }

    entityLabel.ForgetAllAttributes();
    entityLabel.Nullify();
    m_modelTree.removeRoot(entityTreeNodeId);
}

void Document::BeforeClose()
{
    TDocStd_Document::BeforeClose();
    if (m_app)
        m_app->notifyDocumentAboutToClose(m_identifier);
}

void Document::ChangeStorageFormat(const TCollection_ExtendedString& newStorageFormat)
{
    // TODO: check format
    TDocStd_Document::ChangeStorageFormat(newStorageFormat);
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_ptr.h"
#include "document_ptr.h"
#include "document_tree_node.h"
#include "filepath.h"
#include "libtree.h"
#include "signal.h"
#include "xcaf.h"

#include <string>
#include <string_view>

namespace Mayo {

// Provides a data container, composed of labels and attributes
// It extends TDocStd_Document to provide an actualized model tree of its contents
// Entities are actually "root" data items
class Document : public TDocStd_Document {
public:
    using Identifier = int; // TODO alias TypedScalar<int, DocumentIdentifierTag>
    enum class Format { Binary, Xml };

    Identifier identifier() const { return m_identifier; }

    const std::string& name() const;
    void setName(std::string_view name);

    const FilePath& filePath() const;
    void setFilePath(const FilePath& fp);

    static const char NameFormatBinary[];
    static const char NameFormatXml[];
    static const char* toNameFormat(Format format);

    static const char TypeName[];
    virtual const char* dynTypeName() const { return Document::TypeName; }

    bool isXCafDocument() const;
    XCaf& xcaf() { return m_xcaf; }
    const XCaf& xcaf() const { return m_xcaf; }

    TDF_Label rootLabel() const;
    bool isEntity(TreeNodeId nodeId);
    int entityCount() const;
    TDF_Label entityLabel(int index) const;
    TreeNodeId entityTreeNodeId(int index) const;
    DocumentTreeNode entityTreeNode(int index) const;

    const Tree<TDF_Label>& modelTree() const { return m_modelTree; }
    void rebuildModelTree();

    static DocumentPtr findFrom(const TDF_Label& label);

    // Creates general-purpose entity, not bound to a specific type
    TDF_Label newEntityLabel();

    // Creates entity bound to a BRep shape and registered as top-level into XCAFDoc_ShapeTool
    TDF_Label newEntityShapeLabel();

    void addEntityTreeNode(const TDF_Label& label);
    void addEntityTreeNodeSequence(const TDF_LabelSequence& seqLabel);
    void destroyEntity(TreeNodeId entityTreeNodeId);

    // Signals
    Signal<const std::string&> signalNameChanged;
    Signal<const FilePath&> signalFilePathChanged;
    Signal<TreeNodeId> signalEntityAdded;
    Signal<TreeNodeId> signalEntityAboutToBeDestroyed;

public: // -- from TDocStd_Document
    void BeforeClose() override;
    void ChangeStorageFormat(const TCollection_ExtendedString& newStorageFormat) override;

    DEFINE_STANDARD_RTTI_INLINE(Document, TDocStd_Document)

private:
    Document(const ApplicationPtr& app);
    ~Document();

    friend class Application;
    class FormatBinaryRetrievalDriver;
    class FormatXmlRetrievalDriver;

    void initXCaf();
    void setIdentifier(Identifier ident) { m_identifier = ident; }
    TreeNodeId findEntity(const TDF_Label& label) const;
    bool containsLabel(const TDF_Label& label) const;

    ApplicationPtr m_app;
    Identifier m_identifier = -1;
    std::string m_name;
    FilePath m_filePath;
    XCaf m_xcaf;
    Tree<TDF_Label> m_modelTree;
};

} // namespace Mayo

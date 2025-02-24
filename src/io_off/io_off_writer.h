/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_tree_node.h"
#include "../base/io_writer.h"
#include "../base/io_single_format_factory.h"

#include <vector>

namespace Mayo::IO {

// Writer for OFF file format
class OffWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup* group) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup*)  { return {}; }

private:
    std::vector<DocumentTreeNode> m_vecTreeNode;
};

// Provides factory to create OffWriter objects
class OffFactoryWriter : public SingleFormatFactoryWriter<Format_OFF, OffWriter> {};

} // namespace Mayo::IO {

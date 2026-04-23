/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
    bool transfer(gsl::span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const FilePath& filepath, TaskProgress* progress) override;
    void applyProperties(const PropertyGroup* group) override;

    static std::unique_ptr<PropertyGroup> createProperties(PropertyGroup*)  { return {}; }

private:
    std::vector<DocumentTreeNode> m_vecTreeNode;
};

// Provides factory to create OffWriter objects
class OffFactoryWriter : public SingleFormatFactoryWriter<Format_OFF, OffWriter> {};

} // namespace Mayo::IO {

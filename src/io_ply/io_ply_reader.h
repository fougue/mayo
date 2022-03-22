/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/io_reader.h"

namespace miniply {
struct PLYElement;
class PLYReader;
} // namespace miniply

namespace Mayo {
namespace IO {

class PlyReader : public Reader {
public:
    ~PlyReader();

    bool readFile(const FilePath& filepath, TaskProgress* progress) override;
    TDF_LabelSequence transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    miniply::PLYReader* m_reader = nullptr;
    FilePath m_baseFilename;
};

class PlyFactoryReader : public FactoryReader {
public:
    Span<const Format> formats() const override;
    std::unique_ptr<Reader> create(Format format) const override;
    std::unique_ptr<PropertyGroup> createProperties(Format format, PropertyGroup* parentGroup) const override;
};

} // namespace IO
} // namespace Mayo

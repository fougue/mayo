/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_reader.h"
#include "io_writer.h"
#include <TopoDS_Shape.hxx>
#include <QtCore/QString>

namespace Mayo {
namespace IO {

// Reader for OpenCascade BRep file format
class OccBRepReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    TopoDS_Shape m_shape;
    QString m_baseFilename;
};

// Writer for OpenCascade BRep file format
class OccBRepWriter : public Writer {
public:
    bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) override;
    bool writeFile(const QString& filepath, TaskProgress* progress) override;

private:
    TopoDS_Shape m_shape;
};

} // namespace IO
} // namespace Mayo

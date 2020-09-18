/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io.h"
#include <RWObj_CafReader.hxx>

namespace Mayo {
namespace IO {

class OccObjReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override;
    bool transfer(DocumentPtr doc, TaskProgress* progress) override;

private:
    QString m_filepath;
    RWObj_CafReader m_reader;
};

} // namespace IO
} // namespace Mayo

/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_iges.h"
#include "io_occ_caf.h"
#include "task_progress.h"

namespace Mayo {
namespace IO {

OccIgesReader::OccIgesReader()
{
    m_reader.SetColorMode(true);
    m_reader.SetNameMode(true);
    m_reader.SetLayerMode(true);
}

bool OccIgesReader::readFile(const QString& filepath, TaskProgress* progress) {
    return cafReadFile(m_reader, filepath, progress);
}

bool OccIgesReader::transfer(DocumentPtr doc, TaskProgress* progress) {
    return cafTransfer(m_reader, doc, progress);
}

bool OccIgesWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) {
    return cafTransfer(m_writer, appItems, progress);
}

bool OccIgesWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(cafGlobalMutex()); Q_UNUSED(lock);
    m_writer.ComputeModel();
    const bool ok = m_writer.Write(filepath.toLocal8Bit().constData());
    progress->setValue(100);
    return ok;
}

} // namespace IO
} // namespace Mayo

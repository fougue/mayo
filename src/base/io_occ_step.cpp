/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_step.h"
#include "io_occ_caf.h"
#include "task_progress.h"

namespace Mayo {
namespace IO {

OccStepReader::OccStepReader()
{
    m_reader.SetColorMode(true);
    m_reader.SetNameMode(true);
    m_reader.SetLayerMode(true);
    m_reader.SetPropsMode(true);
}

bool OccStepReader::readFile(const QString& filepath, TaskProgress* progress) {
    return cafReadFile(m_reader, filepath, progress);
}

bool OccStepReader::transfer(DocumentPtr doc, TaskProgress* progress) {
    return cafTransfer(m_reader, doc, progress);
}

bool OccStepWriter::transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) {
    //Interface_Static::SetIVal("write.stepcaf.subshapes.name", 1);
    return cafTransfer(m_writer, appItems, progress);
}

bool OccStepWriter::writeFile(const QString& filepath, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(cafGlobalMutex()); Q_UNUSED(lock);
    const IFSelect_ReturnStatus err = m_writer.Write(filepath.toLocal8Bit().constData());
    progress->setValue(100);
    return err == IFSelect_RetDone;
}

} // namespace IO
} // namespace Mayo

/****************************************************************************
**  FougTools
**  Copyright Fougue (30 Mar. 2015)
**  contact@fougue.pro
**
** This software is a computer program whose purpose is to provide utility
** tools for the C++ language and the Qt toolkit.
**
** This software is governed by the CeCILL-C license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-C
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
****************************************************************************/

#include "progress.h"

#include "base_runner.h"

namespace qttask {

Progress::Progress(BaseRunner* runner)
    : m_runner(runner)
{ }

Progress::~Progress()
{ }

int Progress::value() const
{
    return m_value;
}

void Progress::setValue(int pct)
{
    m_value.fetch_add(pct);
    m_runner->qtSignals()->emitProgress(pct);
}

const QString& Progress::step() const
{
    return m_step;
}

void Progress::setStep(const QString& title)
{
    m_step = title;
    m_runner->qtSignals()->emitProgressStep(title);
}

void Progress::outputMessage(const QString &msg)
{
    m_runner->qtSignals()->emitMessage(msg);
}

/*! \brief Returns this progress' custom data for the key \p key as a QVariant
 *
 * Custom progress data is useful for storing arbitrary properties in any progress.\n
 * TaskManager does not use this feature for storing data; it is provided solely for the convenience
 * of the user.
 */
QVariant Progress::data(int key) const
{
    auto it = m_dataHash.find(key);
    return it != m_dataHash.end() ? (*it).second : QVariant();
}

/*! \brief Set this progress' custom data for the key \p key to \p value
 *
 * Custom progress data is useful for storing arbitrary properties for any item. TaskManager does
 * not use this feature for storing data; it is provided solely for the convenience of the user.
 */
void Progress::setData(int key, const QVariant& value)
{
    m_dataHash.emplace(key, value);
}

bool Progress::isAbortRequested() const
{
    return m_runner->isAbortRequested();
}

} // namespace qttask

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

#pragma once

#include <QtCore/QVariant>
#include <QtCore/QString>

#include <unordered_map>

namespace qttask {

class BaseRunner;

/*! \brief Provides feedback on the progress of an executing operation
 */
class Progress
{
public:
    ~Progress();

    int value() const;
    void setValue(int pct);

    const QString& step() const;
    void setStep(const QString& title);

    void outputMessage(const QString& msg);

    QVariant data(int key) const;
    void setData(int key, const QVariant& value);

    bool isAbortRequested() const;

private:
    friend class Manager;
    friend class BaseRunner;

    Progress(BaseRunner* runner);

    BaseRunner* m_runner = nullptr;
    std::unordered_map<int, QVariant> m_dataHash;
    int m_value = 0;
    QString m_step;
};

} // namespace qttask

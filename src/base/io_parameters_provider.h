/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "io_format.h"

namespace Mayo::IO {

class ReaderProperties;
class WriterProperties;

// Abstract mechanism to provide reader/writer parameters for a format
class ParametersProvider {
public:
    virtual ~ParametersProvider() = default;
    virtual const ReaderProperties* findReaderParameters(Format format) const = 0;
    virtual const WriterProperties* findWriterParameters(Format format) const = 0;
};

} // namespace Mayo::IO

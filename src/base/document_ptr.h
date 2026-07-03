/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include <TDocStd_Document.hxx>

namespace Mayo {

class Document;
DEFINE_STANDARD_HANDLE(Document, TDocStd_Document)
using DocumentPtr = OccHandle<Document>;

} // namespace Mayo

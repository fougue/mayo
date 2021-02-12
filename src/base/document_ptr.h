/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TDocStd_Document.hxx>

namespace Mayo {

class Document;
DEFINE_STANDARD_HANDLE(Document, TDocStd_Document)
using DocumentPtr = opencascade::handle<Document>;

} // namespace Mayo

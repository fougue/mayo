/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../src/base/application.h"
#include "../src/base/document.h"

namespace Mayo {

struct DocumentFixture {
    OccHandle<Application> app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    ~DocumentFixture()
    {
        this->app->closeDocument(this->doc);
    }
};

} // namespace Mayo

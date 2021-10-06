/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "text_id.h"
#include "application.h"

namespace Mayo {

std::string_view TextId::tr(int n) const
{
    return Application::instance()->translate(*this, n);
}

bool TextId::isEmpty() const
{
    return this->key.empty();
}

} // namespace Mayo

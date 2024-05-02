/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

// --
// NOTE
// This file isolates inclusion of <Aspect_DisplayConnection.hxx> which is problematic on X11/Linux
// <X.h> #defines constants like "None" which causes name clash with GuiDocument::ViewTrihedronMode::None
// --

#include "../base/occ_handle.h"

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <functional>

namespace Mayo {

using FunctionCreateGraphicsDriver = std::function<OccHandle<Graphic3d_GraphicDriver>()>;

static FunctionCreateGraphicsDriver& getFunctionCreateGraphicsDriver()
{
    static FunctionCreateGraphicsDriver fn = []{
        return makeOccHandle<OpenGl_GraphicDriver>(new Aspect_DisplayConnection);
    };
    return fn;
}

void setFunctionCreateGraphicsDriver(FunctionCreateGraphicsDriver fn)
{
    getFunctionCreateGraphicsDriver() = std::move(fn);
}

OccHandle<Graphic3d_GraphicDriver> graphicsCreateDriver()
{
    const auto& fn = getFunctionCreateGraphicsDriver();
    if (fn)
        return fn();

    return {};
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#cmakedefine MAYO_HAVE_ASSIMP
#cmakedefine MAYO_HAVE_ASSIMP_aiGetVersionPatch
#cmakedefine MAYO_HAVE_GMIO

#ifdef HAVE_RAPIDJSON
#  define OPENCASCADE_HAVE_RAPIDJSON
#endif

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

#ifndef QTTOOLS_GUI_H
#define QTTOOLS_GUI_H

#include <QtCore/QtGlobal>
#ifdef QTTOOLS_GUI_DLL
# ifdef QTTOOLS_GUI_MAKE_DLL
#  define QTTOOLS_GUI_EXPORT Q_DECL_EXPORT
# else
#  define QTTOOLS_GUI_EXPORT Q_DECL_IMPORT
# endif // QTTOOLS_GUI_MAKE_DLL
#else
# define QTTOOLS_GUI_EXPORT
#endif // QTTOOLS_GUI_DLL

#endif // QTTOOLS_GUI_H

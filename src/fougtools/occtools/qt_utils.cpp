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

#include "qt_utils.h"

namespace occ {

//! Conversion of the Quantity_Color \p c to a QColor
QColor QtUtils::toQColor(const Quantity_Color& c)
{
    return QColor(c.Red() * 255., c.Green() * 255., c.Blue() * 255.);
}

//! Conversion of the Quantity_NameOfColor \p c to a QColor
QColor QtUtils::toQColor(const Quantity_NameOfColor c)
{
    Quantity_Color qc(c);
    return QtUtils::toQColor(qc);
}

//! Conversion of the QColor \p c to a Quantity_Color
Quantity_Color QtUtils::toOccColor(const QColor& c)
{
    return Quantity_Color(
                c.red() / 255., c.green() / 255., c.blue() / 255.,
                Quantity_TOC_RGB);
}

//! Conversion of the QColor object \p c to a Quantity_NameOfColor
Quantity_NameOfColor QtUtils::toOccNameOfColor(const QColor& c)
{
    return QtUtils::toOccColor(c).Name();
}

//! Conversion of the QString \p str to an OCC TCollection_AsciiString
//! containing the latin1 representation of \p str
TCollection_AsciiString QtUtils::toOccLatin1String(const QString &str)
{
    return TCollection_AsciiString(str.toLatin1().constData());
}

//! Conversion of the QString \p str to an OCC TCollection_AsciiString
//! containing the local8bit representation of \p str
TCollection_AsciiString QtUtils::toOccLocal8BitString(const QString& str)
{
    return TCollection_AsciiString(str.toLocal8Bit().constData());
}

//! Conversion of the QString \p str to an OCC TCollection_AsciiString
//! containing the utf8 representation of \p str
TCollection_AsciiString QtUtils::toOccUtf8String(const QString &str)
{
    return TCollection_AsciiString(str.toUtf8().constData());
}

//! Conversion of the QString \p str to an OCC ExtString
Standard_ExtString QtUtils::toOccExtString(const QString& str)
{
    return reinterpret_cast<Standard_ExtString>(str.utf16());
}

//! Conversion of the QString \p str to an OCC TCollection_ExtendedString
TCollection_ExtendedString QtUtils::toOccExtendedString(const QString& str)
{
    return TCollection_ExtendedString(QtUtils::toOccExtString(str));
}

//! Conversion of the TCollection_AsciiString \p str to a QString.
//! \p str is assumed to contain latin1 characters
QString QtUtils::fromLatin1ToQString(const TCollection_AsciiString& str)
{
    return QString::fromLatin1(str.ToCString(), str.Length());
}

//! Conversion of the TCollection_AsciiString \p str to a QString.
//! \p str is assumed to contain local8bit characters
QString QtUtils::fromLocal8BitToQString(const TCollection_AsciiString &str)
{
    return QString::fromLocal8Bit(str.ToCString(), str.Length());
}

//! Conversion of the TCollection_AsciiString \p str to a QString.
//! \p str is assumed to contain utf8 characters
QString QtUtils::fromUtf8ToQString(const TCollection_AsciiString &str)
{
    return QString::fromUtf8(str.ToCString(), str.Length());
}

/*! \brief Conversion of the OCC Standard_ExtString \p str to a QString
 *  \param unicodeStr Contains unicode(utf16) characters
 *  \param size Length of \p str (count of characters).
 *              If -1 then \p str must be terminated with a 0
 */
QString QtUtils::toQString(Standard_ExtString unicodeStr, int size)
{
    return QString::fromUtf16(reinterpret_cast<const ushort*>(unicodeStr), size);
}

//! Conversion of the OCC TCollection_ExtendedString \p str to a QString
QString QtUtils::toQString(const TCollection_ExtendedString& str)
{
    return QtUtils::toQString(str.ToExtString(), str.Length());
}

} // namespace occ

/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include <QtCore/QSettings>
#include <QtGui/QColor>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Options
{
public:
    enum class StlIoLibrary {
        Gmio,
        OpenCascade
    };

    static Options* instance();

    StlIoLibrary stlIoLibrary() const;
    void setStlIoLibrary(StlIoLibrary lib);

    // BRep shape graphics

    QColor brepShapeDefaultColor() const;
    void setBrepShapeDefaultColor(const QColor& color);

    Graphic3d_NameOfMaterial brepShapeDefaultMaterial() const;
    void setBrepShapeDefaultMaterial(Graphic3d_NameOfMaterial material);

    // Mesh graphics

    QColor meshDefaultColor() const;
    void setMeshDefaultColor(const QColor& color);

    Graphic3d_NameOfMaterial meshDefaultMaterial() const;
    void setMeshDefaultMaterial(Graphic3d_NameOfMaterial material);

    bool meshDefaultShowEdges() const;
    void setMeshDefaultShowEdges(bool on);

    bool meshDefaultShowNodes() const;
    void setMeshDefaultShowNodes(bool on);

private:
    QSettings m_settings;
};

} // namespace Mayo

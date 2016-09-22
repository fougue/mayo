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

#include "options.h"

namespace Mayo {

static const char keyStlIoLibrary[] = "Core/stlIoLibrary";
static const char keyBrepShapeDefaultColor[] = "BRepShapeGpx/defaultColor";
static const char keyBrepShapeDefaultMaterial[] = "BRepShapeGpx/defaultMaterial";
static const char keyMeshDefaultColor[] = "MeshGpx/defaultColor";
static const char keyMeshDefaultMaterial[] = "MeshGpx/defaultMaterial";
static const char keyMeshDefaultShowEdges[] = "MeshGpx/defaultShowEdges";
static const char keyMeshDefaultShowNodes[] = "MeshGpx/defaultShowNodes";

Options *Options::instance()
{
    static Options opts;
    return &opts;
}

Options::StlIoLibrary Options::stlIoLibrary() const
{
    static const int defaultLib = static_cast<int>(StlIoLibrary::Gmio);
    const int stlIoLib = m_settings.value(keyStlIoLibrary, defaultLib).toInt();
    return static_cast<StlIoLibrary>(stlIoLib);
}

void Options::setStlIoLibrary(Options::StlIoLibrary lib)
{
    m_settings.setValue(keyStlIoLibrary, static_cast<int>(lib));
}

QColor Options::brepShapeDefaultColor() const
{
    static const QColor defaultColor(Qt::gray);
    return m_settings.value(
                keyBrepShapeDefaultColor, defaultColor).value<QColor>();
}

void Options::setBrepShapeDefaultColor(const QColor &color)
{
    m_settings.setValue(keyBrepShapeDefaultColor, color);
}

Graphic3d_NameOfMaterial Options::brepShapeDefaultMaterial() const
{
    static const int defaultMat = Graphic3d_NOM_PLASTIC;
    const int mat =
            m_settings.value(keyBrepShapeDefaultMaterial, defaultMat).toInt();
    return static_cast<Graphic3d_NameOfMaterial>(mat);
}

void Options::setBrepShapeDefaultMaterial(Graphic3d_NameOfMaterial material)
{
    m_settings.setValue(keyBrepShapeDefaultMaterial, static_cast<int>(material));
}

QColor Options::meshDefaultColor() const
{
    static const QColor defaultColor(Qt::gray);
    return m_settings.value(keyMeshDefaultColor, defaultColor).value<QColor>();
}

void Options::setMeshDefaultColor(const QColor &color)
{
    m_settings.setValue(keyMeshDefaultColor, color);
}

Graphic3d_NameOfMaterial Options::meshDefaultMaterial() const
{
    static const int defaultMat = Graphic3d_NOM_PLASTIC;
    const int mat =
            m_settings.value(keyMeshDefaultMaterial, defaultMat).toInt();
    return static_cast<Graphic3d_NameOfMaterial>(mat);
}

void Options::setMeshDefaultMaterial(Graphic3d_NameOfMaterial material)
{
    m_settings.setValue(keyMeshDefaultMaterial, static_cast<int>(material));
}

bool Options::meshDefaultShowEdges() const
{
    return m_settings.value(keyMeshDefaultShowEdges, false).toBool();
}

void Options::setMeshDefaultShowEdges(bool on)
{
    m_settings.setValue(keyMeshDefaultShowEdges, on);
}

bool Options::meshDefaultShowNodes() const
{
    return m_settings.value(keyMeshDefaultShowNodes, false).toBool();
}

void Options::setMeshDefaultShowNodes(bool on)
{
    m_settings.setValue(keyMeshDefaultShowNodes, on);
}

} // namespace Mayo

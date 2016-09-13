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

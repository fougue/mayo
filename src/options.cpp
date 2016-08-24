#include "options.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

static const char keyStlIoLibrary[] = "Core/stlIoLibrary";
static const char keyBrepShapeDefaultColor[] = "BRepShapeGpx/defaultColor";
static const char keyBrepShapeDefaultMaterial[] = "BRepShapeGpx/defaultMaterial";
static const char keyMeshDefaultColor[] = "MeshGpx/defaultColor";
static const char keyMeshDefaultMaterial[] = "MeshGpx/defaultMaterial";
static const char keyMeshDefaultShowEdges[] = "MeshGpx/defaultShowEdges";
static const char keyMeshDefaultShowNodes[] = "MeshGpx/defaultShowNodes";

const std::vector<Options::Material> &Options::materials()
{
    static std::vector<Options::Material> vecMaterials;
    if (vecMaterials.empty()) {
        const Options::Material materials[] = {
            { Graphic3d_NOM_BRASS,
              QCoreApplication::translate("Mayo::Options", "Brass") },
            { Graphic3d_NOM_BRONZE,
              QCoreApplication::translate("Mayo::Options", "Bronze") },
            { Graphic3d_NOM_COPPER,
              QCoreApplication::translate("Mayo::Options", "Copper") },
            { Graphic3d_NOM_GOLD,
              QCoreApplication::translate("Mayo::Options", "Gold") },
            { Graphic3d_NOM_PEWTER,
              QCoreApplication::translate("Mayo::Options", "Pewter") },
            { Graphic3d_NOM_PLASTER,
              QCoreApplication::translate("Mayo::Options", "Plaster") },
            { Graphic3d_NOM_PLASTIC,
              QCoreApplication::translate("Mayo::Options", "Plastic") },
            { Graphic3d_NOM_SILVER,
              QCoreApplication::translate("Mayo::Options", "Silver") },
            { Graphic3d_NOM_STEEL,
              QCoreApplication::translate("Mayo::Options", "Steel") },
            { Graphic3d_NOM_STONE,
              QCoreApplication::translate("Mayo::Options", "Stone") },
            { Graphic3d_NOM_SHINY_PLASTIC,
              QCoreApplication::translate("Mayo::Options", "Shiny plastic") },
            { Graphic3d_NOM_SATIN,
              QCoreApplication::translate("Mayo::Options", "Satin") },
            { Graphic3d_NOM_METALIZED,
              QCoreApplication::translate("Mayo::Options", "Metalized") },
            { Graphic3d_NOM_NEON_GNC,
              QCoreApplication::translate("Mayo::Options", "Neon gnc") },
            { Graphic3d_NOM_CHROME,
              QCoreApplication::translate("Mayo::Options", "Chrome") },
            { Graphic3d_NOM_ALUMINIUM,
              QCoreApplication::translate("Mayo::Options", "Aluminium") },
            { Graphic3d_NOM_OBSIDIAN,
              QCoreApplication::translate("Mayo::Options", "Obsidian") },
            { Graphic3d_NOM_NEON_PHC,
              QCoreApplication::translate("Mayo::Options", "Neon phc") },
            { Graphic3d_NOM_JADE,
              QCoreApplication::translate("Mayo::Options", "Jade") },
            { Graphic3d_NOM_DEFAULT,
              QCoreApplication::translate("Mayo::Options", "Default") }
        };
        vecMaterials.reserve(std::cend(materials) - std::cbegin(materials));
        for (const Material& mat : materials)
            vecMaterials.emplace_back(std::move(mat));
    }
    return vecMaterials;
}

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

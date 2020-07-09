/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "occt_enums.h"
#include <QtCore/QCoreApplication>

namespace Mayo {

const Enumeration& OcctEnums::Graphic3d_NameOfMaterial()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.addItem(
                    Graphic3d_NOM_BRASS,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Brass"));
        enumeration.addItem(
                    Graphic3d_NOM_BRONZE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Bronze"));
        enumeration.addItem(
                    Graphic3d_NOM_COPPER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Copper"));
        enumeration.addItem(
                    Graphic3d_NOM_GOLD,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Gold"));
        enumeration.addItem(
                    Graphic3d_NOM_PEWTER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Pewter"));
        enumeration.addItem(
                    Graphic3d_NOM_PLASTER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Plaster"));
        enumeration.addItem(
                    Graphic3d_NOM_PLASTIC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Plastic"));
        enumeration.addItem(
                    Graphic3d_NOM_SILVER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Silver"));
        enumeration.addItem(
                    Graphic3d_NOM_STEEL,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Steel"));
        enumeration.addItem(
                    Graphic3d_NOM_STONE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Stone"));
        enumeration.addItem(
                    Graphic3d_NOM_SHINY_PLASTIC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Shiny plastic"));
        enumeration.addItem(
                    Graphic3d_NOM_SATIN,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Satin"));
        enumeration.addItem(
                    Graphic3d_NOM_METALIZED,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Metalized"));
        enumeration.addItem(
                    Graphic3d_NOM_NEON_GNC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Neon gnc"));
        enumeration.addItem(
                    Graphic3d_NOM_CHROME,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Chrome"));
        enumeration.addItem(
                    Graphic3d_NOM_ALUMINIUM,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Aluminium"));
        enumeration.addItem(
                    Graphic3d_NOM_OBSIDIAN,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Obsidian"));
        enumeration.addItem(
                    Graphic3d_NOM_NEON_PHC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Neon phc"));
        enumeration.addItem(
                    Graphic3d_NOM_JADE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Jade"));
        enumeration.addItem(
                    Graphic3d_NOM_DEFAULT,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Default"));
    }

    return enumeration;
}

const Enumeration& OcctEnums::Aspect_HatchStyle()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.addItem(
                    Aspect_HS_SOLID,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Solid"));
        enumeration.addItem(
                    Aspect_HS_HORIZONTAL,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Horizontal"));
        enumeration.addItem(
                    Aspect_HS_HORIZONTAL_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Horizontal sparse"));
        enumeration.addItem(
                    Aspect_HS_VERTICAL,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Vertical"));
        enumeration.addItem(
                    Aspect_HS_VERTICAL_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Vertical sparse"));
        enumeration.addItem(
                    Aspect_HS_DIAGONAL_45,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Diagonal_45"));
        enumeration.addItem(
                    Aspect_HS_DIAGONAL_45_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Diagonal_45 sparse"));
        enumeration.addItem(
                    Aspect_HS_DIAGONAL_135,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Diagonal_135"));
        enumeration.addItem(
                    Aspect_HS_DIAGONAL_135_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Diagonal_135 sparse"));
        enumeration.addItem(
                    Aspect_HS_GRID,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Grid"));
        enumeration.addItem(
                    Aspect_HS_GRID_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Grid sparse"));
        enumeration.addItem(
                    Aspect_HS_GRID_DIAGONAL,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Grid diagonal"));
        enumeration.addItem(
                    Aspect_HS_GRID_DIAGONAL_WIDE,
                    QCoreApplication::translate("Mayo::EnumHatchStyle", "Grid diagonal sparse"));
    }

    return enumeration;
}

} // namespace Mayo

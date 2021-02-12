/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "occt_enums.h"

namespace Mayo {

namespace {
struct OccNameOfMaterial { MAYO_DECLARE_TEXT_ID_FUNCTIONS(OpenCascade::Graphic3d_NameOfMaterial) };
struct OccHatchStyle { MAYO_DECLARE_TEXT_ID_FUNCTIONS(OpenCascade::Aspect_HatchStyle) };
} // namespace

const Enumeration& OcctEnums::Graphic3d_NameOfMaterial()
{
    static const Enumeration enumeration = {
        { Graphic3d_NOM_BRASS,   OccNameOfMaterial::textId("Brass") },
        { Graphic3d_NOM_BRONZE,  OccNameOfMaterial::textId("Bronze") },
        { Graphic3d_NOM_COPPER,  OccNameOfMaterial::textId("Copper") },
        { Graphic3d_NOM_GOLD,    OccNameOfMaterial::textId("Gold") },
        { Graphic3d_NOM_PEWTER,  OccNameOfMaterial::textId("Pewter") },
        { Graphic3d_NOM_PLASTER, OccNameOfMaterial::textId("Plaster") },
        { Graphic3d_NOM_PLASTIC, OccNameOfMaterial::textId("Plastic") },
        { Graphic3d_NOM_SILVER,  OccNameOfMaterial::textId("Silver") },
        { Graphic3d_NOM_STEEL,   OccNameOfMaterial::textId("Steel") },
        { Graphic3d_NOM_STONE,   OccNameOfMaterial::textId("Stone") },
        { Graphic3d_NOM_SHINY_PLASTIC, OccNameOfMaterial::textId("ShinyPlastic") },
        { Graphic3d_NOM_SATIN,     OccNameOfMaterial::textId("Satin") },
        { Graphic3d_NOM_METALIZED, OccNameOfMaterial::textId("Metalized") },
        { Graphic3d_NOM_NEON_GNC,  OccNameOfMaterial::textId("NeonGnc") },
        { Graphic3d_NOM_CHROME,    OccNameOfMaterial::textId("Chrome") },
        { Graphic3d_NOM_ALUMINIUM, OccNameOfMaterial::textId("Aluminium") },
        { Graphic3d_NOM_OBSIDIAN,  OccNameOfMaterial::textId("Obsidian") },
        { Graphic3d_NOM_NEON_PHC,  OccNameOfMaterial::textId("NeonPhc") },
        { Graphic3d_NOM_JADE,      OccNameOfMaterial::textId("Jade") },
        { Graphic3d_NOM_DEFAULT,   OccNameOfMaterial::textId("Default") }
    };
    return enumeration;
}

const Enumeration& OcctEnums::Aspect_HatchStyle()
{
    static const Enumeration enumeration = {
        { Aspect_HS_SOLID,              OccHatchStyle::textId("Solid") },
        { Aspect_HS_HORIZONTAL,         OccHatchStyle::textId("Horizontal") },
        { Aspect_HS_HORIZONTAL_WIDE,    OccHatchStyle::textId("HorizontalSparse") },
        { Aspect_HS_VERTICAL,           OccHatchStyle::textId("Vertical") },
        { Aspect_HS_VERTICAL_WIDE,      OccHatchStyle::textId("VerticalSparse") },
        { Aspect_HS_DIAGONAL_45,        OccHatchStyle::textId("Diagonal45") },
        { Aspect_HS_DIAGONAL_45_WIDE,   OccHatchStyle::textId("Diagonal45Sparse") },
        { Aspect_HS_DIAGONAL_135,       OccHatchStyle::textId("Diagonal135") },
        { Aspect_HS_DIAGONAL_135_WIDE,  OccHatchStyle::textId("Diagonal135Sparse") },
        { Aspect_HS_GRID,               OccHatchStyle::textId("Grid") },
        { Aspect_HS_GRID_WIDE,          OccHatchStyle::textId("GridSparse") },
        { Aspect_HS_GRID_DIAGONAL,      OccHatchStyle::textId("GridDiagonal") },
        { Aspect_HS_GRID_DIAGONAL_WIDE, OccHatchStyle::textId("GridDiagonalSparse") }
    };
    return enumeration;
}

} // namespace Mayo

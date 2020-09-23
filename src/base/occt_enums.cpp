/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "occt_enums.h"

namespace Mayo {

const Enumeration& OcctEnums::Graphic3d_NameOfMaterial()
{
    static const Enumeration enumeration = {
        { Graphic3d_NOM_BRASS,   MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Brass") },
        { Graphic3d_NOM_BRONZE,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Bronze") },
        { Graphic3d_NOM_COPPER,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Copper") },
        { Graphic3d_NOM_GOLD,    MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Gold") },
        { Graphic3d_NOM_PEWTER,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Pewter") },
        { Graphic3d_NOM_PLASTER, MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Plaster") },
        { Graphic3d_NOM_PLASTER, MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Plastic") },
        { Graphic3d_NOM_SILVER,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Silver") },
        { Graphic3d_NOM_STEEL,   MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Steel") },
        { Graphic3d_NOM_STONE,   MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Stone") },
        { Graphic3d_NOM_SHINY_PLASTIC, MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Shiny plastic") },
        { Graphic3d_NOM_SATIN,     MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Satin") },
        { Graphic3d_NOM_METALIZED, MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Metalized") },
        { Graphic3d_NOM_NEON_GNC,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Neon gnc") },
        { Graphic3d_NOM_CHROME,    MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Chrome") },
        { Graphic3d_NOM_ALUMINIUM, MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Aluminium") },
        { Graphic3d_NOM_OBSIDIAN,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Obsidian") },
        { Graphic3d_NOM_NEON_PHC,  MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Neon phc") },
        { Graphic3d_NOM_JADE,      MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Jade") },
        { Graphic3d_NOM_DEFAULT,   MAYO_TEXT_ID("Graphic3d_NameOfMaterial", "Defaukt") }
    };
    return enumeration;
}

const Enumeration& OcctEnums::Aspect_HatchStyle()
{
    static const Enumeration enumeration = {
        { Aspect_HS_SOLID,              MAYO_TEXT_ID("Aspect_HatchStyle", "Solid") },
        { Aspect_HS_HORIZONTAL,         MAYO_TEXT_ID("Aspect_HatchStyle", "Horizontal") },
        { Aspect_HS_HORIZONTAL_WIDE,    MAYO_TEXT_ID("Aspect_HatchStyle", "HorizontalSparse") },
        { Aspect_HS_VERTICAL,           MAYO_TEXT_ID("Aspect_HatchStyle", "Vertical") },
        { Aspect_HS_VERTICAL_WIDE,      MAYO_TEXT_ID("Aspect_HatchStyle", "VerticalSparse") },
        { Aspect_HS_DIAGONAL_45,        MAYO_TEXT_ID("Aspect_HatchStyle", "Diagonal45") },
        { Aspect_HS_DIAGONAL_45_WIDE,   MAYO_TEXT_ID("Aspect_HatchStyle", "Diagonal45Sparse") },
        { Aspect_HS_DIAGONAL_135,       MAYO_TEXT_ID("Aspect_HatchStyle", "Diagonal135") },
        { Aspect_HS_DIAGONAL_135_WIDE,  MAYO_TEXT_ID("Aspect_HatchStyle", "Diagonal135Sparse") },
        { Aspect_HS_GRID,               MAYO_TEXT_ID("Aspect_HatchStyle", "Grid") },
        { Aspect_HS_GRID_WIDE,          MAYO_TEXT_ID("Aspect_HatchStyle", "GridSparse") },
        { Aspect_HS_GRID_DIAGONAL,      MAYO_TEXT_ID("Aspect_HatchStyle", "GridDiagonal") },
        { Aspect_HS_GRID_DIAGONAL_WIDE, MAYO_TEXT_ID("Aspect_HatchStyle", "GridDiagonalSparse") }
    };
    return enumeration;
}

} // namespace Mayo

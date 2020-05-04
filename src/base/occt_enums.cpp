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
        { Graphic3d_NOM_BRASS,   "BRASS",   tr("Brass", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_BRONZE,  "BRONZE",  tr("Bronze", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_COPPER,  "COPPER",  tr("Copper", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_GOLD,    "GOLD",    tr("Gold", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_PEWTER,  "PEWTER",  tr("Pewter", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_PLASTER, "PLASTER", tr("Plaster", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_PLASTER, "PLASTIC", tr("Plastic", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_SILVER,  "SILVER",  tr("Silver", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_STEEL,   "STEEL",   tr("Steel", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_STONE,   "STONE",   tr("Stone", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_SHINY_PLASTIC, "SHINY_PLASTIC", tr("Shiny plastic", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_SATIN,     "SATIN",     tr("Satin", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_METALIZED, "METALIZED", tr("Metalized", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_NEON_GNC,  "NEON_GNC",  tr("Neon gnc", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_CHROME,    "CHROME",    tr("Chrome", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_ALUMINIUM, "ALUMINIUM", tr("Aluminium", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_OBSIDIAN,  "OBSIDIAN",  tr("Obsidian", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_NEON_PHC,  "NEON_PHC",  tr("Neon phc", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_JADE,      "JADE",      tr("Jade", "Graphic3d_NameOfMaterial") },
        { Graphic3d_NOM_DEFAULT,   "DEFAULT",   tr("Defaukt", "Graphic3d_NameOfMaterial") }
    };
    return enumeration;
}

const Enumeration& OcctEnums::Aspect_HatchStyle()
{
    static const Enumeration enumeration = {
        { Aspect_HS_SOLID,              "SOLID",              tr("Solid", "Aspect_HatchStyle") },
        { Aspect_HS_HORIZONTAL,         "HORIZONTAL",         tr("Horizontal", "Aspect_HatchStyle") },
        { Aspect_HS_HORIZONTAL_WIDE,    "HORIZONTAL_WIDE",    tr("Horizontal sparse", "Aspect_HatchStyle") },
        { Aspect_HS_VERTICAL,           "VERTICAL",           tr("Vertical", "Aspect_HatchStyle") },
        { Aspect_HS_VERTICAL_WIDE,      "VERTICAL_WIDE",      tr("Vertical sparse", "Aspect_HatchStyle") },
        { Aspect_HS_DIAGONAL_45,        "DIAGONAL_45",        tr("Diagonal/45", "Aspect_HatchStyle") },
        { Aspect_HS_DIAGONAL_45_WIDE,   "DIAGONAL_45_WIDE",   tr("Diagonal/45 sparse", "Aspect_HatchStyle") },
        { Aspect_HS_DIAGONAL_135,       "DIAGONAL_135",       tr("Diagonal/135", "Aspect_HatchStyle") },
        { Aspect_HS_DIAGONAL_135_WIDE,  "DIAGONAL_135_WIDE",  tr("Diagonal/135 sparse", "Aspect_HatchStyle") },
        { Aspect_HS_GRID,               "GRID",               tr("Grid", "Aspect_HatchStyle") },
        { Aspect_HS_GRID_WIDE,          "GRID_WIDE",          tr("Grid sparse", "Aspect_HatchStyle") },
        { Aspect_HS_GRID_DIAGONAL,      "GRID_DIAGONAL",      tr("Grid diagonal", "Aspect_HatchStyle") },
        { Aspect_HS_GRID_DIAGONAL_WIDE, "GRID_DIAGONAL_WIDE", tr("Grid diagonal sparse", "Aspect_HatchStyle") }
    };
    return enumeration;
}

} // namespace Mayo

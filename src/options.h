/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "unit_system.h"
#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtGui/QColor>
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Options : public QObject {
    Q_OBJECT
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

    // Clip planes

    bool isClipPlaneCappingOn() const;
    void setClipPlaneCapping(bool on);

    Aspect_HatchStyle clipPlaneCappingHatch() const;
    void setClipPlaneCappingHatch(Aspect_HatchStyle hatch);

    // Units

    UnitSystem::Schema unitSystemSchema() const;
    void setUnitSystemSchema(UnitSystem::Schema schema);

    int unitSystemDecimals() const;
    void setUnitSystemDecimals(int count);

    template<Unit UNIT>
    static UnitSystem::TranslateResult unitSystemTranslate(const Quantity<UNIT>& qty) {
        return UnitSystem::translate(Options::instance()->unitSystemSchema(), qty);
    }

signals:
    void clipPlaneCappingToggled(bool on);
    void clipPlaneCappingHatchChanged(Aspect_HatchStyle hatch);

    void unitSystemSchemaChanged(UnitSystem::Schema schema);
    void unitSystemDecimalsChanged(int count);

private:
    QSettings m_settings;
};

} // namespace Mayo

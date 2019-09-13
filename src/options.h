/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "string_utils.h"
#include "unit_system.h"
#include <QtCore/QLocale>
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

    const QSettings* settings() const;
    QSettings* settings();

    const QLocale& locale() const;
    void setLocale(const QLocale& locale);

    StringUtils::TextOptions defaultTextOptions() const;

    StlIoLibrary stlIoLibrary() const;
    void setStlIoLibrary(StlIoLibrary lib);

    bool defaultShowOriginTrihedron() const;
    void setDefaultShowOriginTrihedron(bool on);

    // Recent files

    QStringList recentFiles() const;
    void setRecentFiles(const QStringList& files);

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
    static UnitSystem::TranslateResult unitSystemTranslate(Quantity<UNIT> qty) {
        return UnitSystem::translate(Options::instance()->unitSystemSchema(), qty);
    }
    UnitSystem::TranslateResult unitSystemTranslate(double value, Unit unit);

signals:
    void clipPlaneCappingToggled(bool on);
    void clipPlaneCappingHatchChanged(Aspect_HatchStyle hatch);

    void unitSystemSchemaChanged(UnitSystem::Schema schema);
    void unitSystemDecimalsChanged(int count);

private:
    Options();

    QSettings m_settings;
    QLocale m_locale;
};

} // namespace Mayo

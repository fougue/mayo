/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "options.h"

namespace Mayo {

static const char keyStlIoLibrary[] = "Core/stlIoLibrary";
static const char keyRecentFiles[] = "File/recentFiles";
static const char keyDefaultShowOriginTrihedron[] = "General/defaultShowOriginTrihedron";
static const char keyBrepShapeDefaultColor[] = "BRepShapeGpx/defaultColor";
static const char keyBrepShapeDefaultMaterial[] = "BRepShapeGpx/defaultMaterial";
static const char keyMeshDefaultColor[] = "MeshGpx/defaultColor";
static const char keyMeshDefaultMaterial[] = "MeshGpx/defaultMaterial";
static const char keyMeshDefaultShowEdges[] = "MeshGpx/defaultShowEdges";
static const char keyMeshDefaultShowNodes[] = "MeshGpx/defaultShowNodes";
static const char keyClipPlaneCappingOn[] = "ClipPlane/CappingOn";
static const char keyClipPlaneCappingHatch[] = "ClipPlane/CappingHatch";
static const char keyUnitSystemSchema[] = "UnitSystem/Schema";
static const char keyUnitSystemDecimals[] = "UnitSystem/Decimals";
static const char keyReferenceItemTextMode[] = "ModelTree/ReferenceItemTextMode";

Options *Options::instance()
{
    static Options opts;
    return &opts;
}

Options::StlIoLibrary Options::stlIoLibrary() const
{
#ifdef HAVE_GMIO
    static const int defaultVal = static_cast<int>(StlIoLibrary::Gmio);
    const int stlIoLib = m_settings.value(keyStlIoLibrary, defaultVal).toInt();
    return static_cast<StlIoLibrary>(stlIoLib);
#else
    return StlIoLibrary::OpenCascade;
#endif
}

void Options::setStlIoLibrary(Options::StlIoLibrary lib)
{
    m_settings.setValue(keyStlIoLibrary, static_cast<int>(lib));
}

bool Options::defaultShowOriginTrihedron() const
{
    return m_settings.value(keyDefaultShowOriginTrihedron, true).toBool();
}

void Options::setDefaultShowOriginTrihedron(bool on)
{
    m_settings.setValue(keyDefaultShowOriginTrihedron, on);
}

QStringList Options::recentFiles() const
{
    return m_settings.value(keyRecentFiles, QStringList()).toStringList();
}

void Options::setRecentFiles(const QStringList &files)
{
    m_settings.setValue(keyRecentFiles, files);
}

const QLocale &Options::locale() const
{
    return m_locale;
}

void Options::setLocale(const QLocale &locale)
{
    m_locale = locale;
}

StringUtils::TextOptions Options::defaultTextOptions() const
{
    StringUtils::TextOptions opt;
    opt.locale = m_locale;
    opt.unitDecimals = this->unitSystemDecimals();
    opt.unitSchema = this->unitSystemSchema();
    return opt;
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

bool Options::isClipPlaneCappingOn() const
{
    return m_settings.value(keyClipPlaneCappingOn, true).toBool();
}

void Options::setClipPlaneCapping(bool on)
{
    if (this->isClipPlaneCappingOn() != on) {
        m_settings.setValue(keyClipPlaneCappingOn, on);
        emit clipPlaneCappingToggled(on);
    }
}

Aspect_HatchStyle Options::clipPlaneCappingHatch() const
{
    const int hatch =
            m_settings.value(keyClipPlaneCappingHatch, Aspect_HS_SOLID).toInt();
    return static_cast<Aspect_HatchStyle>(hatch);
}

void Options::setClipPlaneCappingHatch(Aspect_HatchStyle hatch)
{
    if (this->clipPlaneCappingHatch() != hatch) {
        m_settings.setValue(keyClipPlaneCappingHatch, static_cast<int>(hatch));
        emit clipPlaneCappingHatchChanged(hatch);
    }
}

UnitSystem::Schema Options::unitSystemSchema() const
{
    const int intSchema =
            m_settings.value(keyUnitSystemSchema, UnitSystem::SI).toInt();
    return static_cast<UnitSystem::Schema>(intSchema);
}

void Options::setUnitSystemSchema(UnitSystem::Schema schema)
{
    if (schema != this->unitSystemSchema()) {
        m_settings.setValue(keyUnitSystemSchema, schema);
        emit this->unitSystemSchemaChanged(schema);
    }
}

int Options::unitSystemDecimals() const
{
    return m_settings.value(keyUnitSystemDecimals, 2).toInt();
}

void Options::setUnitSystemDecimals(int count)
{
    if (count != this->unitSystemDecimals()) {
        m_settings.setValue(keyUnitSystemDecimals, count);
        emit this->unitSystemDecimalsChanged(count);
    }
}

UnitSystem::TranslateResult Options::unitSystemTranslate(double value, Unit unit)
{
    return UnitSystem::translate(this->unitSystemSchema(), value, unit);
}

QString Options::toReferenceItemTextTemplate(Options::ReferenceItemTextMode mode)
{
    switch (mode) {
    case ReferenceItemTextMode::ReferenceOnly:
        return QStringLiteral("%instance");
    case ReferenceItemTextMode::ReferredOnly:
        return QStringLiteral("%referred");
    case ReferenceItemTextMode::ReferenceAndReferred:
        // UTF8 rightwards arrow : \xe2\x86\x92
        return QString::fromUtf8("%instance \xe2\x86\x92 %referred");
    }
    return QString();
}

QString Options::referenceItemTextTemplate() const
{
    return Options::toReferenceItemTextTemplate(this->referenceItemTextMode());
}

Options::ReferenceItemTextMode Options::referenceItemTextMode() const
{
    static const int defaultVal = static_cast<int>(ReferenceItemTextMode::ReferenceOnly);
    const int val = m_settings.value(keyReferenceItemTextMode, defaultVal).toInt();
    return static_cast<ReferenceItemTextMode>(val);
}

void Options::setReferenceItemTextMode(Options::ReferenceItemTextMode mode)
{
    m_settings.setValue(keyReferenceItemTextMode, static_cast<int>(mode));
}

Options::Options()
    : m_locale(QLocale::system())
{
}

} // namespace Mayo

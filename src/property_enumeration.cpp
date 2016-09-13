#include "property_enumeration.h"

#include "options.h"

#include <cassert>
#include <QtCore/QCoreApplication>

namespace Mayo {

void Enumeration::map(Value eval, const QString& str)
{
    const Mapping mapping = { eval, str };
    m_vecMapping.emplace_back(std::move(mapping));
}

std::size_t Enumeration::size() const
{
    return m_vecMapping.size();
}

std::size_t Enumeration::index(Value eval) const
{
    return this->findCppSql(eval) - m_vecMapping.cbegin();
}

Enumeration::Value Enumeration::valueAt(std::size_t i) const
{
    return m_vecMapping.at(i).value;
}

Enumeration::Value Enumeration::value(const QString& str) const
{
    auto it = std::find_if(
                m_vecMapping.cbegin(),
                m_vecMapping.cend(),
                [&](const Mapping& mapping) { return mapping.string == str; });
    assert(it != m_vecMapping.cend());
    return it->value;
}

const QString& Enumeration::string(Value eval) const
{
    auto it = this->findCppSql(eval);
    assert(it != m_vecMapping.cend());
    return it->string;
}

Enumeration::Mapping Enumeration::mapping(std::size_t i) const
{
    assert(i < m_vecMapping.size());
    return m_vecMapping.at(i);
}

const std::vector<Enumeration::Mapping> &Enumeration::mappings() const
{
    return m_vecMapping;
}

std::vector<Enumeration::Mapping>::const_iterator
Enumeration::findCppSql(Value eval) const
{
    auto it = std::find_if(
                m_vecMapping.cbegin(),
                m_vecMapping.cend(),
                [=] (const Mapping& map) { return map.value == eval; } );
    assert(it != m_vecMapping.cend());
    return it;
}

PropertyEnumeration::PropertyEnumeration(
        PropertyOwner* owner,
        const QString& label,
        const Enumeration* enumeration)
    : Property(owner, label),
      m_enumeration(enumeration)
{
    assert(m_enumeration != nullptr);
    assert(m_enumeration->size() > 0);
    m_value = m_enumeration->valueAt(0);
}

const Enumeration& PropertyEnumeration::enumeration() const
{
    return *m_enumeration;
}

const QString& PropertyEnumeration::string() const
{
    return m_enumeration->string(m_value);
}

Enumeration::Value PropertyEnumeration::value() const
{
    return m_value;
}

void PropertyEnumeration::setValue(Enumeration::Value v)
{
    // TODO: check v is an enumerated value of m_enumeration
    m_value = v;
    this->notifyChanged();
}

const char* PropertyEnumeration::dynTypeName() const
{
    return Property::EnumerationTypeName;
}

const Enumeration &enum_Graphic3dNameOfMaterial()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        enumeration.map(
                    Graphic3d_NOM_BRASS,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Brass"));
        enumeration.map(
                    Graphic3d_NOM_BRONZE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Bronze"));
        enumeration.map(
                    Graphic3d_NOM_COPPER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Copper"));
        enumeration.map(
                    Graphic3d_NOM_GOLD,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Gold"));
        enumeration.map(
                    Graphic3d_NOM_PEWTER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Pewter"));
        enumeration.map(
                    Graphic3d_NOM_PLASTER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Plaster"));
        enumeration.map(
                    Graphic3d_NOM_PLASTIC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Plastic"));
        enumeration.map(
                    Graphic3d_NOM_SILVER,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Silver"));
        enumeration.map(
                    Graphic3d_NOM_STEEL,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Steel"));
        enumeration.map(
                    Graphic3d_NOM_STONE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Stone"));
        enumeration.map(
                    Graphic3d_NOM_SHINY_PLASTIC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Shiny plastic"));
        enumeration.map(
                    Graphic3d_NOM_SATIN,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Satin"));
        enumeration.map(
                    Graphic3d_NOM_METALIZED,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Metalized"));
        enumeration.map(
                    Graphic3d_NOM_NEON_GNC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Neon gnc"));
        enumeration.map(
                    Graphic3d_NOM_CHROME,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Chrome"));
        enumeration.map(
                    Graphic3d_NOM_ALUMINIUM,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Aluminium"));
        enumeration.map(
                    Graphic3d_NOM_OBSIDIAN,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Obsidian"));
        enumeration.map(
                    Graphic3d_NOM_NEON_PHC,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Neon phc"));
        enumeration.map(
                    Graphic3d_NOM_JADE,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Jade"));
        enumeration.map(
                    Graphic3d_NOM_DEFAULT,
                    QCoreApplication::translate("Mayo::EnumGpxMaterial", "Default"));
    }
    return enumeration;
}

} // namespace Mayo

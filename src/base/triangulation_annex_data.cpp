/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "triangulation_annex_data.h"

#include <Standard_GUID.hxx>
#include <TDF_Label.hxx>
#include <algorithm>
#include <iterator>

namespace Mayo {

const Standard_GUID& TriangulationAnnexData::GetID()
{
    static const Standard_GUID TriangulationAnnexDataID("2a2c7df4-6d97-4c70-b20c-84cb149e26ed");
    return TriangulationAnnexDataID;
}

TriangulationAnnexDataPtr TriangulationAnnexData::Set(const TDF_Label& label)
{
    TriangulationAnnexDataPtr data;
    if (!label.FindAttribute(TriangulationAnnexData::GetID(), data)) {
        data = new TriangulationAnnexData;
        label.AddAttribute(data);
    }

    return data;
}

TriangulationAnnexDataPtr TriangulationAnnexData::Set(
        const TDF_Label& label, Span<const Quantity_Color> spanNodeColor)
{
    TriangulationAnnexDataPtr data = TriangulationAnnexData::Set(label);
    data->copyNodeColors(spanNodeColor);
    return data;
}

TriangulationAnnexDataPtr TriangulationAnnexData::Set(
        const TDF_Label& label, std::vector<Quantity_Color>&& vecNodeColor)
{
    TriangulationAnnexDataPtr data = TriangulationAnnexData::Set(label);
    data->m_vecNodeColor = std::move(vecNodeColor);
    return data;
}

const Standard_GUID& TriangulationAnnexData::ID() const
{
    return TriangulationAnnexData::GetID();
}

void TriangulationAnnexData::Restore(const OccHandle<TDF_Attribute>& attribute)
{
    auto data = TriangulationAnnexDataPtr::DownCast(attribute);
    if (data)
        this->copyNodeColors(data->m_vecNodeColor);
}

OccHandle<TDF_Attribute> TriangulationAnnexData::NewEmpty() const
{
    return new TriangulationAnnexData;
}

void TriangulationAnnexData::Paste(const OccHandle<TDF_Attribute>& into, const OccHandle<TDF_RelocationTable>&) const
{
    auto data = TriangulationAnnexDataPtr::DownCast(into);
    if (data)
        data->copyNodeColors(m_vecNodeColor);
}

Standard_OStream& TriangulationAnnexData::Dump(Standard_OStream& ostr) const
{
    ostr << "TriangulationAnnexData -- ";
    TDF_Attribute::Dump(ostr);
    return ostr;
}

void Mayo::TriangulationAnnexData::copyNodeColors(Span<const Quantity_Color> spanNodeColor)
{
    m_vecNodeColor.clear();
    std::copy(spanNodeColor.begin(), spanNodeColor.end(), std::back_inserter(m_vecNodeColor));
}

} // namespace Mayo

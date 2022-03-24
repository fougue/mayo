/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "data_triangulation.h"

#include <Standard_GUID.hxx>
#include <TDF_Label.hxx>
#include <algorithm>
#include <iterator>

namespace Mayo {

const Standard_GUID& DataTriangulation::GetID()
{
    static const Standard_GUID DataTriangulationID("3020FF98-2B49-4E0B-A414-949A534F24F7");
    return DataTriangulationID;
}

DataTriangulationPtr DataTriangulation::Set(const TDF_Label& label)
{
    DataTriangulationPtr dataMesh;
    if (!label.FindAttribute(DataTriangulation::GetID(), dataMesh)) {
        dataMesh = new DataTriangulation;
        label.AddAttribute(dataMesh);
    }

    return dataMesh;
}

DataTriangulationPtr DataTriangulation::Set(const TDF_Label& label, const Handle(Poly_Triangulation)& mesh)
{
    DataTriangulationPtr dataMesh = DataTriangulation::Set(label);
    dataMesh->TDataXtd_Triangulation::Set(mesh);
    return dataMesh;
}

DataTriangulationPtr DataTriangulation::Set(
        const TDF_Label& label, const Handle(Poly_Triangulation)& mesh, Span<const Quantity_Color> spanNodeColor)
{
    DataTriangulationPtr dataMesh = DataTriangulation::Set(label, mesh);
    dataMesh->copyNodeColors(spanNodeColor);
    return dataMesh;
}

const Standard_GUID& DataTriangulation::ID() const
{
    return DataTriangulation::GetID();
}

void DataTriangulation::Restore(const Handle(TDF_Attribute)& attribute)
{
    TDataXtd_Triangulation::Restore(attribute);
    auto dataMesh = DataTriangulationPtr::DownCast(attribute);
    if (dataMesh)
        this->copyNodeColors(dataMesh->m_vecNodeColor);
}

Handle(TDF_Attribute) DataTriangulation::NewEmpty() const
{
    return new DataTriangulation;
}

void DataTriangulation::Paste(const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& table) const
{
    TDataXtd_Triangulation::Paste(into, table);
    auto dataMesh = DataTriangulationPtr::DownCast(into);
    if (dataMesh)
        dataMesh->copyNodeColors(m_vecNodeColor);
}

Standard_OStream& DataTriangulation::Dump(Standard_OStream& ostr) const
{
    ostr << "DataTriangulation -- ";
    TDataXtd_Triangulation::Dump(ostr);
    return ostr;
}

void Mayo::DataTriangulation::copyNodeColors(Span<const Quantity_Color> spanNodeColor)
{
    m_vecNodeColor.clear();
    std::copy(spanNodeColor.begin(), spanNodeColor.end(), std::back_inserter(m_vecNodeColor));
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "point_cloud_data.h"

#include <Standard_GUID.hxx>
#include <TDF_Label.hxx>

namespace Mayo {

const Standard_GUID& PointCloudData::GetID()
{
    static const Standard_GUID PointCloudDataID("ca1b07ac-d6b7-4210-9728-6e4af6a0df50");
    return PointCloudDataID;
}

PointCloudDataPtr PointCloudData::Set(const TDF_Label& label)
{
    PointCloudDataPtr data;
    if (!label.FindAttribute(PointCloudData::GetID(), data)) {
        data = new PointCloudData;
        label.AddAttribute(data);
    }

    return data;
}

PointCloudDataPtr PointCloudData::Set(const TDF_Label& label, const OccHandle<Graphic3d_ArrayOfPoints>& points)
{
    PointCloudDataPtr data = PointCloudData::Set(label);
    data->m_points = points;
    return data;
}

const Standard_GUID& PointCloudData::ID() const
{
    return PointCloudData::GetID();
}

void PointCloudData::Restore(const OccHandle<TDF_Attribute>& attribute)
{
    auto data = PointCloudDataPtr::DownCast(attribute);
    if (data)
        m_points = data->m_points;
}

OccHandle<TDF_Attribute> PointCloudData::NewEmpty() const
{
    return new PointCloudData;
}

void PointCloudData::Paste(const OccHandle<TDF_Attribute>& into, const OccHandle<TDF_RelocationTable>&) const
{
    auto data = PointCloudDataPtr::DownCast(into);
    if (data)
        data->m_points = m_points;
}

Standard_OStream& PointCloudData::Dump(Standard_OStream& ostr) const
{
    ostr << "PointCloudData -- ";
    TDF_Attribute::Dump(ostr);
    return ostr;
}

} // namespace Mayo

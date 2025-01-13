/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"

#include <Graphic3d_ArrayOfPoints.hxx>
#include <TDF_Attribute.hxx>

namespace Mayo {

// Pre-declarations
class PointCloudData;
DEFINE_STANDARD_HANDLE(PointCloudData, TDF_Attribute)
using PointCloudDataPtr = OccHandle<PointCloudData>;

// Provides a label attribute to store point cloud data
class PointCloudData : public TDF_Attribute {
public:
    static const Standard_GUID& GetID();
    static PointCloudDataPtr Set(const TDF_Label& label);
    static PointCloudDataPtr Set(const TDF_Label& label, const OccHandle<Graphic3d_ArrayOfPoints>& points);

    const OccHandle<Graphic3d_ArrayOfPoints>& points() const { return m_points; }

    // -- from TDF_Attribute
    const Standard_GUID& ID() const override;
    void Restore(const OccHandle<TDF_Attribute>& attribute) override;
    OccHandle<TDF_Attribute> NewEmpty() const override;
    void Paste(const OccHandle<TDF_Attribute>& into, const OccHandle<TDF_RelocationTable>& table) const override;
    Standard_OStream& Dump(Standard_OStream& ostr) const override;

    DEFINE_STANDARD_RTTI_INLINE(PointCloudData, TDF_Attribute)

private:
    OccHandle<Graphic3d_ArrayOfPoints> m_points;
};

} // namespace Mayo

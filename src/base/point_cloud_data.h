/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Graphic3d_ArrayOfPoints.hxx>
#include <TDF_Attribute.hxx>

namespace Mayo {

class PointCloudData;
DEFINE_STANDARD_HANDLE(PointCloudData, TDF_Attribute)
using PointCloudDataPtr = Handle(PointCloudData);

class PointCloudData : public TDF_Attribute {
public:
    static const Standard_GUID& GetID();
    static PointCloudDataPtr Set(const TDF_Label& label);
    static PointCloudDataPtr Set(const TDF_Label& label, const Handle(Graphic3d_ArrayOfPoints)& points);

    const Handle(Graphic3d_ArrayOfPoints)& points() const { return m_points; }

    // -- from TDF_Attribute
    const Standard_GUID& ID() const override;
    void Restore(const Handle(TDF_Attribute)& attribute) override;
    Handle(TDF_Attribute) NewEmpty() const override;
    void Paste(const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& table) const override;
    Standard_OStream& Dump(Standard_OStream& ostr) const override;

    DEFINE_STANDARD_RTTI_INLINE(PointCloudData, TDF_Attribute)

private:
    Handle(Graphic3d_ArrayOfPoints) m_points;
};

} // namespace Mayo

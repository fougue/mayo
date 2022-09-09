/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"

#include <Quantity_Color.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <vector>

namespace Mayo {

class DataTriangulation;
DEFINE_STANDARD_HANDLE(DataTriangulation, TDataXtd_Triangulation)
using DataTriangulationPtr = Handle(DataTriangulation);

class DataTriangulation : public TDataXtd_Triangulation {
public:
    static const Standard_GUID& GetID();
    static DataTriangulationPtr Set(const TDF_Label& label);
    static DataTriangulationPtr Set(const TDF_Label& label, const Handle(Poly_Triangulation)& mesh);
    static DataTriangulationPtr Set(
            const TDF_Label& label,
            const Handle(Poly_Triangulation)& mesh,
            Span<const Quantity_Color> spanNodeColor
    );

    Span<const Quantity_Color> nodeColors() const { return m_vecNodeColor; }

    // -- from TDF_Attribute
    const Standard_GUID& ID() const override;
    void Restore(const Handle(TDF_Attribute)& attribute) override;
    Handle(TDF_Attribute) NewEmpty() const override;
    void Paste(const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& table) const override;
    Standard_OStream& Dump(Standard_OStream& ostr) const override;

    DEFINE_STANDARD_RTTI_INLINE(DataTriangulation, TDataXtd_Triangulation)

private:
    void copyNodeColors(Span<const Quantity_Color> spanNodeColor);

    std::vector<Quantity_Color> m_vecNodeColor;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"

#include <Quantity_Color.hxx>
#include <TDF_Attribute.hxx>
#include <vector>

namespace Mayo {

class TriangulationAnnexData;
DEFINE_STANDARD_HANDLE(TriangulationAnnexData, TDF_Attribute)
using TriangulationAnnexDataPtr = Handle(TriangulationAnnexData);

class TriangulationAnnexData : public TDF_Attribute {
public:
    static const Standard_GUID& GetID();
    static TriangulationAnnexDataPtr Set(const TDF_Label& label);
    static TriangulationAnnexDataPtr Set(const TDF_Label& label, Span<const Quantity_Color> spanNodeColor);

    Span<const Quantity_Color> nodeColors() const { return m_vecNodeColor; }

    // -- from TDF_Attribute
    const Standard_GUID& ID() const override;
    void Restore(const Handle(TDF_Attribute)& attribute) override;
    Handle(TDF_Attribute) NewEmpty() const override;
    void Paste(const Handle(TDF_Attribute)& into, const Handle(TDF_RelocationTable)& table) const override;
    Standard_OStream& Dump(Standard_OStream& ostr) const override;

    DEFINE_STANDARD_RTTI_INLINE(TriangulationAnnexData, TDF_Attribute)

private:
    void copyNodeColors(Span<const Quantity_Color> spanNodeColor);

    std::vector<Quantity_Color> m_vecNodeColor;
};

} // namespace Mayo

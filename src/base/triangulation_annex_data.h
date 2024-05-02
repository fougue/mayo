/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include "span.h"

#include <Quantity_Color.hxx>
#include <TDF_Attribute.hxx>
#include <vector>

namespace Mayo {

class TriangulationAnnexData;
DEFINE_STANDARD_HANDLE(TriangulationAnnexData, TDF_Attribute)
using TriangulationAnnexDataPtr = OccHandle<TriangulationAnnexData>;

class TriangulationAnnexData : public TDF_Attribute {
public:
    static const Standard_GUID& GetID();
    static TriangulationAnnexDataPtr Set(const TDF_Label& label);
    static TriangulationAnnexDataPtr Set(const TDF_Label& label, Span<const Quantity_Color> spanNodeColor);
    static TriangulationAnnexDataPtr Set(const TDF_Label& label, std::vector<Quantity_Color>&& vecNodeColor);

    Span<const Quantity_Color> nodeColors() const { return m_vecNodeColor; }

    // -- from TDF_Attribute
    const Standard_GUID& ID() const override;
    void Restore(const OccHandle<TDF_Attribute>& attribute) override;
    OccHandle<TDF_Attribute> NewEmpty() const override;
    void Paste(const OccHandle<TDF_Attribute>& into, const OccHandle<TDF_RelocationTable>& table) const override;
    Standard_OStream& Dump(Standard_OStream& ostr) const override;

    DEFINE_STANDARD_RTTI_INLINE(TriangulationAnnexData, TDF_Attribute)

private:
    void copyNodeColors(Span<const Quantity_Color> spanNodeColor);

    std::vector<Quantity_Color> m_vecNodeColor;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "xde_document_item.h"
#include "gpx_document_item.h"
#include <XCAFPrs_AISObject.hxx>

namespace Mayo {

class GpxXdeDocumentItem :
        public GpxCovariantDocumentItem<XdeDocumentItem, XCAFPrs_AISObject, Handle_XCAFPrs_AISObject>,
        public GpxBRepShapeCommonProperties
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxXdeDocumentItem)

public:
    GpxXdeDocumentItem(XdeDocumentItem* item);

protected:
    void onPropertyChanged(Property* prop) override;
};

} // namespace Mayo

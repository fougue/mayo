/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "gpx_document_item.h"
#include "../base/mesh_item.h"
#include <MeshVS_Mesh.hxx>
#include <QtGui/QColor>

namespace Mayo {

class GpxMeshItem : public GpxDocumentItem {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxMeshItem)
public:
    GpxMeshItem(MeshItem* item);
    ~GpxMeshItem();

    MeshItem* documentItem() const override;

    void setVisible(bool on) override;
    void activateSelection(int mode) override;
    std::vector<Handle_SelectMgr_EntityOwner> entityOwners(int mode) const override;
    Bnd_Box boundingBox() const override;

    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowEdges;
    PropertyBool propertyShowNodes;

    struct DefaultValues {
        bool showEdges = false;
        bool showNodes = false;
        Graphic3d_NameOfMaterial material = Graphic3d_NOM_PLASTIC;
        QColor color = Qt::gray;
    };

    static const DefaultValues& defaultValues();
    static void setDefaultValues(const DefaultValues& values);

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
    MeshItem* m_meshItem = nullptr;
    Handle_MeshVS_Mesh m_meshVisu;
};

} // namespace Mayo

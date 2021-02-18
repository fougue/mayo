/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/libtree.h"
#include "../graphics/graphics_object_ptr.h"
#include <gp_Pnt.hxx>
#include <QtWidgets/QWidget>
#include <vector>

namespace Mayo {

class GuiDocument;

class WidgetExplodeAssembly : public QWidget {
    Q_OBJECT
public:
    WidgetExplodeAssembly(GuiDocument* guiDoc, QWidget* parent = nullptr);
    ~WidgetExplodeAssembly();

private:
    void onFactorChanged(double t); // t within [0,1]

    void mapEntity(TreeNodeId entityNodeId);
    void unmapEntity(TreeNodeId entityNodeId);

    struct Movable {
        GraphicsObjectPtr gfxObject;
        Bnd_Box bndBox;
        gp_Trsf trsfOriginal;
    };

    struct Entity {
        TreeNodeId treeNodeId;
        Bnd_Box bndBox;
        std::vector<Movable> vecMovable;
    };

    class Ui_WidgetExplodeAssembly* m_ui= nullptr;
    GuiDocument* m_guiDoc = nullptr;
    std::vector<Entity> m_vecEntity;
};

} // namespace Mayo

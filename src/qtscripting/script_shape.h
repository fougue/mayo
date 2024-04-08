/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QObject>
#include <TopoDS_Shape.hxx>

namespace Mayo {

class ScriptShape {
    Q_GADGET
    Q_PROPERTY(unsigned type READ type)
    Q_PROPERTY(unsigned orientation READ orientation)
    Q_PROPERTY(bool hasTransformation READ hasTransformation)
    Q_PROPERTY(QVariantList transformation READ transformation)
    Q_PROPERTY(QVariant geometry READ geometry)
public:
    ScriptShape() = default;
    ScriptShape(const TopoDS_Shape& shape) : m_shape(shape) {}

    unsigned type() const { return m_shape.ShapeType(); }
    unsigned orientation() const { return m_shape.Orientation(); }

    bool hasTransformation() const;
    QVariantList transformation() const;

    QVariant geometry() const; // ->ScriptGeomSurface or ScriptGeomEdge

    const TopoDS_Shape& shape() const { return m_shape; }

private:
    TopoDS_Shape m_shape;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptShape)

/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TopoDS_Shape.hxx>

#include <QtCore/QObject>
#include <QtQml/QJSValue>
class QJSEngine;

namespace Mayo {

#ifndef _MAYO_DOCGEN_
using QVariant_ScriptShape = QVariant;
using ScriptShapeType = unsigned; // ->TopAbs_ShapeEnum
using ScriptShapeOrientation = unsigned; // ->TopAbs_Orientation
using QJSValue_ShapeTraverseCallback = QJSValue;
#endif

class ScriptShape {
    Q_GADGET
    Q_PROPERTY(ScriptShapeType type READ type)
    Q_PROPERTY(ScriptShapeOrientation orientation READ orientation)
    Q_PROPERTY(bool hasTransformation READ hasTransformation)
    Q_PROPERTY(QVariantList transformation READ transformation)
    Q_PROPERTY(QVariant geometry READ geometry)
public:
    ScriptShape() = default;
    ScriptShape(const TopoDS_Shape& shape, QJSEngine* jsEngine = nullptr);

    ScriptShapeType type() const { return m_shape.ShapeType(); }
    ScriptShapeOrientation orientation() const { return m_shape.Orientation(); }

    bool hasTransformation() const;
    QVariantList transformation() const;

    QVariant geometry() const; // ->ScriptGeomSurface OR ScriptGeomEdge OR Coords3D

    const TopoDS_Shape& shape() const { return m_shape; }

    Q_INVOKABLE void traverse(ScriptShapeType filter, QJSValue_ShapeTraverseCallback fn);

private:
    TopoDS_Shape m_shape;
    QJSEngine* m_jsEngine = nullptr;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptShape)

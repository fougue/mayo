#pragma once

#include <QtCore/QSettings>
#include <QtGui/QColor>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Options
{
public:
    enum class StlIoLibrary {
        Gmio,
        OpenCascade
    };

    static Options* instance();

    StlIoLibrary stlIoLibrary() const;
    void setStlIoLibrary(StlIoLibrary lib);

    // BRep shape graphics

    QColor brepShapeDefaultColor() const;
    void setBrepShapeDefaultColor(const QColor& color);

    Graphic3d_NameOfMaterial brepShapeDefaultMaterial() const;
    void setBrepShapeDefaultMaterial(Graphic3d_NameOfMaterial material);

    // Mesh graphics

    QColor meshDefaultColor() const;
    void setMeshDefaultColor(const QColor& color);

    Graphic3d_NameOfMaterial meshDefaultMaterial() const;
    void setMeshDefaultMaterial(Graphic3d_NameOfMaterial material);

    bool meshDefaultShowEdges() const;
    void setMeshDefaultShowEdges(bool on);

    bool meshDefaultShowNodes() const;
    void setMeshDefaultShowNodes(bool on);

private:
    QSettings m_settings;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_inspect_xde.h"

#include "../base/brep_utils.h"
#include "../base/caf_utils.h"
#include "../base/cpp_utils.h"
#include "../base/occ_handle.h"
#include "../base/meta_enum.h"
#include "../base/tkernel_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "app_module.h"
#include "qmeta_tdf_label.h"
#include "qstring_utils.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "ui_dialog_inspect_xde.h"

#include <Image_AlienPixMap.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TNaming_NamedShape.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ColorType.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GeomTolerance.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_Volume.hxx>

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <Image_Texture.hxx>
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include <XCAFDoc_VisMaterial.hxx>
#  include <XCAFDoc_VisMaterialCommon.hxx>
#  include <XCAFDoc_VisMaterialTool.hxx>
#endif

#include <QtCore/QBuffer>
#include <QtCore/QFileInfo>

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace Mayo {

namespace Internal {

enum TreeWidgetItemRole {
    TreeWidgetItem_TdfLabelRole = Qt::UserRole + 1
};

static QStringUtils::TextOptions appDefaultTextOptions()
{
    return AppModule::get()->defaultTextOptions();
}

static void loadLabelAttributes(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    for (TDF_AttributeIterator it(label); it.More(); it.Next()) {
        const OccHandle<TDF_Attribute> ptrAttr = it.Value();
        const Standard_GUID& attrId = ptrAttr->ID();
        QString text;
        QString value;
        if (attrId == TDataStd_Name::GetID()) {
            const auto& name = static_cast<const TDataStd_Name&>(*ptrAttr);
            text = "TDataStd_Name";
            value = to_QString(name.Get());
        }
#if 0
        else if (attrId == TDataStd_TreeNode::GetID()) {
            const auto& node = static_cast<const TDataStd_TreeNode&>(*ptrAttr);
            text = QString("TDataStd_TreeNode [%1]").arg(area.Get());
        }
#endif
        else if (attrId == XCAFDoc_Area::GetID()) {
            const auto& area = static_cast<const XCAFDoc_Area&>(*ptrAttr);
            text = "XCAFDoc_Area";
            value = QStringUtils::text(area.Get(), appDefaultTextOptions());
        }
        else if (attrId == XCAFDoc_Centroid::GetID()) {
            const auto& centroid = static_cast<const XCAFDoc_Centroid&>(*ptrAttr);
            text = "XCAFDoc_Centroid";
            value = QStringUtils::text(centroid.Get(), appDefaultTextOptions());
        }
        else if (attrId == XCAFDoc_Volume::GetID()) {
            const auto& volume = static_cast<const XCAFDoc_Volume&>(*ptrAttr);
            text = "XCAFDoc_Volume";
            value = QStringUtils::text(volume.Get(), appDefaultTextOptions());
        }
        else if (attrId == XCAFDoc_Color::GetID()) {
            const auto& color = static_cast<const XCAFDoc_Color&>(*ptrAttr);
            text = "XCAFDoc_Color";
            value = QStringUtils::text(color.GetColor());
        }
        else if (attrId == XCAFDoc_Location::GetID()) {
            const auto& location = static_cast<const XCAFDoc_Location&>(*ptrAttr);
            text = "XCAFDoc_Location";
            value = QStringUtils::text(location.Get().Transformation(), appDefaultTextOptions());
        }
        else if (attrId == TNaming_NamedShape::GetID()) {
            const auto& namedShape = static_cast<const TNaming_NamedShape&>(*ptrAttr);
            const TopoDS_Shape shape = namedShape.Get();
            text = "TNaming_NamedShape";
            value = DialogInspectXde::tr("ShapeType=%1, ShapeLocation=%2, Evolution=%3")
                    .arg(MetaEnum::name(shape.ShapeType()).data())
                    .arg(shape.Location().Transformation().Form() != gp_Identity ?
                             QStringUtils::text(shape.Location(), appDefaultTextOptions())
                             : QString("id")
                        )
                    .arg(MetaEnum::name(namedShape.Evolution()).data());
        }
        else {
            std::stringstream sstream;
            ptrAttr->Dump(sstream);
            QString strDump = QString::fromStdString(sstream.str());

            int i = 0;
            while (i < strDump.size() && strDump.at(i).isSpace()) ++i;

            const int wordStart = i;
            while (i < strDump.size() && !strDump.at(i).isSpace()) ++i;

            const int wordEnd = i;
            while (i < strDump.size() && strDump.at(i).isSpace()) ++i;

            const int dataStart = i;
            text = wordStart < strDump.size() ? strDump.mid(wordStart, wordEnd - wordStart) : QStringLiteral("??");
            strDump = strDump.right(strDump.size() - dataStart);
            value = strDump.replace(QChar('\n'), QString("  "));
        }

        auto attrTreeItem = new QTreeWidgetItem;
        attrTreeItem->setText(0, text);
        attrTreeItem->setText(1, value);
        treeItem->addChild(attrTreeItem);
    }
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text)
{
    auto itemProperty = new QTreeWidgetItem;
    itemProperty->setText(0, text);
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, bool isPropertyOn)
{
    auto itemProperty = createPropertyTreeItem(text);
    const QString strYes = DialogInspectXde::tr("Yes");
    const QString strNo = DialogInspectXde::tr("No");
    itemProperty->setText(1, isPropertyOn ? strYes : strNo);
    itemProperty->setForeground(1, isPropertyOn ? Qt::green : QColor(Qt::red).lighter());
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, int value)
{
    auto itemProperty = createPropertyTreeItem(text);
    itemProperty->setText(1, QString::number(value));
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(
        const QString& text, double value, const QStringUtils::TextOptions& options = appDefaultTextOptions())
{
    auto itemProperty = createPropertyTreeItem(text);
    itemProperty->setText(1, QStringUtils::text(value, options));
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const QString& value)
{
    auto itemProperty = createPropertyTreeItem(text);
    itemProperty->setText(1, value);
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const OccHandle<TCollection_HAsciiString>& value)
{
    return createPropertyTreeItem(text, to_QString(value));
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, std::string_view value)
{
    return createPropertyTreeItem(text, QString::fromUtf8(value.data()));
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const Quantity_Color& color)
{
    auto itemColor = createPropertyTreeItem(text);
    itemColor->setText(1, QStringUtils::text(color));
    QPixmap pixColor(24, 16);
    pixColor.fill(QtGuiUtils::toQColor(color));
    itemColor->setIcon(1, pixColor);
    return itemColor;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const Quantity_ColorRGBA& color)
{
    auto itemColor = createPropertyTreeItem(text, color.GetRGB());
    itemColor->setText(1, itemColor->text(1) + QString(" A:%1").arg(int(color.Alpha() * 255)));
    return itemColor;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const gp_Pnt& pnt)
{
    auto itemPnt = createPropertyTreeItem(text);
    itemPnt->setText(1, QStringUtils::text(pnt, appDefaultTextOptions()));
    return itemPnt;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const gp_Ax2& ax2)
{
    auto itemPnt = createPropertyTreeItem(text);
    const QString textAx2 =
            QString("Location %1 - Direction %2")
            .arg(QStringUtils::text(ax2.Location(), appDefaultTextOptions()))
            .arg(QStringUtils::text(ax2.Direction(), appDefaultTextOptions()));
    itemPnt->setText(1, textAx2);
    return itemPnt;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const TopoDS_Shape& shape)
{
    auto itemShape = createPropertyTreeItem(text);

    int vertexCount = 0;
    int edgeCount = 0;
    int faceCount = 0;
    BRepUtils::forEachSubShape(shape, TopAbs_VERTEX, [&](const TopoDS_Shape&) { ++vertexCount; });
    BRepUtils::forEachSubShape(shape, TopAbs_EDGE, [&](const TopoDS_Shape&) { ++edgeCount; });
    BRepUtils::forEachSubShape(shape, TopAbs_FACE, [&](const TopoDS_Shape&) { ++faceCount; });
    const QString textShape =
            QString("%1%2%3")
            .arg(vertexCount ? QString("%1 vertices ").arg(vertexCount) : QString())
            .arg(edgeCount ? QString("%1 edges ").arg(edgeCount) : QString())
            .arg(faceCount ? QString("%1 faces").arg(faceCount) : QString())
            ;
    itemShape->setText(1, textShape);

    return itemShape;
}

static void loadLabelMaterialProperties(
        const TDF_Label& label, const OccHandle<XCAFDoc_MaterialTool>& materialTool, QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    fnAddItem(createPropertyTreeItem("IsMaterial", materialTool->IsMaterial(label)));

    QStringUtils::TextOptions densityValueTextOptions = appDefaultTextOptions();
    densityValueTextOptions.unitDecimals = 6;

    OccHandle<TCollection_HAsciiString> name;
    OccHandle<TCollection_HAsciiString> description;
    double density;
    OccHandle<TCollection_HAsciiString> densityName;
    OccHandle<TCollection_HAsciiString> densityValueType;
    if (materialTool->GetMaterial(label, name, description, density, densityName, densityValueType)) {
        fnAddItem(createPropertyTreeItem("Name", to_QString(name)));
        fnAddItem(createPropertyTreeItem("Description", to_QString(description)));
        fnAddItem(createPropertyTreeItem("Density", density, densityValueTextOptions));
        fnAddItem(createPropertyTreeItem("DensityName", to_QString(densityName)));
        fnAddItem(createPropertyTreeItem("DensityValType", to_QString(densityValueType)));
    }
    else {
        density = materialTool->GetDensityForShape(label);
        if (!qFuzzyIsNull(density))
            fnAddItem(createPropertyTreeItem("Density", density, densityValueTextOptions));
    }

    treeItem->addChildren(listItemProp);
}

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)

// Load pixmap from file
static QPixmap loadPixmap(const FilePath& filePath)
{
    QPixmap pixmap;
    bool okLoad = pixmap.load(filepathTo<QString>(filePath));
    if (!okLoad || pixmap.isNull()) {
        // QPixmap::load() failed, try with OpenCascade Image_AlienPixMap::Load()
        Image_AlienPixMap occPixmap;
        okLoad = occPixmap.Load(filepathTo<TCollection_AsciiString>(filePath));
        if (okLoad)
            pixmap = QtGuiUtils::toQPixmap(occPixmap);
    }

    return pixmap;
}

// Load pixmap from data buffer
static QPixmap loadPixmap(const QByteArray& fileData)
{
    QPixmap pixmap;
    bool okLoad = pixmap.loadFromData(fileData);
    if (!okLoad || pixmap.isNull()) {
        // QPixmap::loadFromData() failed, try with OpenCascade Image_AlienPixMap::Load()
        Image_AlienPixMap occPixmap;
        okLoad = occPixmap.Load(
            reinterpret_cast<const Standard_Byte*>(fileData.constData()),
            fileData.size(),
            TCollection_AsciiString{}
        );
        if (okLoad)
            pixmap = QtGuiUtils::toQPixmap(occPixmap);
    }

    return pixmap;
}

// Provides a QTreeWidgetItem specialized to display an image file with a tooltip
// QTreeWidgetItem::setToolTip() could be used but it forces all image files to be loaded on
// tree item construction
// This helper allows "lazy" loading of the image files
class ImageFileTreeWidgetItem : public QTreeWidgetItem {
public:
    void setImage(int col, const FilePath& filePath)
    {
        const ItemData item{filePath, {}, {}};
        m_mapColumnItemData.insert({ col, item });
    }

    void setImage(int col, const QByteArray& data)
    {
        const ItemData item{{}, data, {}};
        m_mapColumnItemData.insert({ col, item });
    }

    QVariant data(int column, int role) const override
    {
        if (role != Qt::ToolTipRole)
            return QTreeWidgetItem::data(column, role);

        auto itItem = m_mapColumnItemData.find(column);
        ItemData* ptrItem = itItem != m_mapColumnItemData.end() ? &itItem->second : nullptr;
        if (!ptrItem)
            return {};

        if (ptrItem->strToolTip.isEmpty()) {
            const bool isFilePathDefined = !ptrItem->filePath.empty();
            const QPixmap pixmap = isFilePathDefined ? loadPixmap(ptrItem->filePath) : loadPixmap(ptrItem->fileData);
            const uintmax_t imageSize = isFilePathDefined ? filepathFileSize(ptrItem->filePath) : ptrItem->fileData.size();
            if (!pixmap.isNull()) {
                QBuffer bufferPixmap;
                const int pixmapWidth = std::min(pixmap.width(), int(400 * qGuiApp->devicePixelRatio()));
                const QPixmap pixmapClamped = pixmap.scaledToWidth(pixmapWidth);
                pixmapClamped.save(&bufferPixmap, "PNG");
                const QString strImageSize = QStringUtils::bytesText(imageSize, appDefaultTextOptions().locale);
                ptrItem->strToolTip =
                        QString("<img src=\"data:image/png;base64,%1\" width=\"%2\" height=\"%3\"><p>%4</p>")
                        .arg(QString::fromLatin1(bufferPixmap.data().toBase64()))
                        .arg(pixmapClamped.width())
                        .arg(pixmapClamped.height())
                        .arg(DialogInspectXde::tr("File Size: %1<br>Dimensions: %2x%3 Depth: %4")
                        .arg(strImageSize).arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth()))
                        ;
            }
            else {
                ptrItem->strToolTip = DialogInspectXde::tr("Error when loading texture file(invalid path?)");
            }
        }

        return ptrItem->strToolTip;
    }

private:
    struct ItemData {
        FilePath filePath;
        QByteArray fileData;
        QString strToolTip;
    };

    mutable std::unordered_map<int, ItemData> m_mapColumnItemData;
};

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const OccHandle<Image_Texture>& imgTexture)
{
    if (imgTexture.IsNull())
        return static_cast<QTreeWidgetItem*>(nullptr);

    auto item = new ImageFileTreeWidgetItem;
    item->setText(0, text);
    if (!imgTexture->FilePath().IsEmpty()) {
        // Texture is provided through a file reference
        const FilePath filePath = filepathCanonical(filepathFrom(imgTexture->FilePath()));
        const QString strFilePath = filepathTo<QString>(filePath);
        const auto fileOffset = imgTexture->FileOffset();
        if (fileOffset > 0) {
            // Texture is defined in a file portion
            item->setText(1, DialogInspectXde::tr("%1,offset:%2").arg(strFilePath).arg(fileOffset));
            QFile file(strFilePath);
            if (file.open(QIODevice::ReadOnly)) {
                file.seek(fileOffset);
                const QByteArray buff = file.read(imgTexture->FileLength());
                item->setImage(1, buff);
            }
        }
        else {
            // Texture is defined in a file
            item->setText(1, strFilePath);
            item->setImage(1, filePath);
        }
    }
    else if (imgTexture->DataBuffer() && !imgTexture->DataBuffer()->IsEmpty()) {
        // Texture is provided by some embedded data
        item->setText(1, DialogInspectXde::tr("<data>"));
        const char* buffData = reinterpret_cast<const char*>(imgTexture->DataBuffer()->Data());
        const int buffSize = Cpp::safeStaticCast<int>(imgTexture->DataBuffer()->Size());
        item->setImage(1, QByteArray::fromRawData(buffData, buffSize));
    }

    return item;
}
#endif

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
static void loadLabelVisMaterialProperties(
        const TDF_Label& label,
        const OccHandle<XCAFDoc_VisMaterialTool>& visMaterialTool,
        QTreeWidgetItem* treeItem
    )
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    fnAddItem(createPropertyTreeItem("IsMaterial", visMaterialTool->IsMaterial(label)));
    fnAddItem(createPropertyTreeItem("IsSetShapeMaterial", visMaterialTool->IsSetShapeMaterial(label)));

    auto fnCreateVisMaterialTreeItem = [](const QString& text, const OccHandle<XCAFDoc_VisMaterial>& material) {
        auto item = new QTreeWidgetItem;
        item->setText(0, text);
        item->addChild(createPropertyTreeItem("HasPbrMaterial", material->HasPbrMaterial()));
        item->addChild(createPropertyTreeItem("HasCommonMaterial", material->HasCommonMaterial()));
        item->addChild(createPropertyTreeItem("BaseColor", material->BaseColor()));
        item->addChild(createPropertyTreeItem("AlphaMode", MetaEnum::name(material->AlphaMode())));
        item->addChild(createPropertyTreeItem("AlphaCutOff", material->AlphaCutOff()));
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
        item->addChild(createPropertyTreeItem("FaceCulling", MetaEnum::name(material->FaceCulling())));
#else
        item->addChild(createPropertyTreeItem("IsDoubleSided", material->IsDoubleSided()));
#endif
        if (!material->RawName().IsNull())
            item->addChild(createPropertyTreeItem("RawName", to_QString(material->RawName())));

        if (material->HasPbrMaterial()) {
            const XCAFDoc_VisMaterialPBR& pbrMaterial = material->PbrMaterial();
            auto itemPbr = new QTreeWidgetItem;
            itemPbr->setText(0, "PbrMaterial");
            itemPbr->addChild(createPropertyTreeItem("BaseColorTexture", pbrMaterial.BaseColorTexture));
            itemPbr->addChild(createPropertyTreeItem("MetallicRoughnessTexture", pbrMaterial.MetallicRoughnessTexture));
            itemPbr->addChild(createPropertyTreeItem("EmissiveTexture", pbrMaterial.EmissiveTexture));
            itemPbr->addChild(createPropertyTreeItem("OcclusionTexture", pbrMaterial.OcclusionTexture));
            itemPbr->addChild(createPropertyTreeItem("NormalTexture", pbrMaterial.NormalTexture));
            itemPbr->addChild(createPropertyTreeItem("BaseColor", pbrMaterial.BaseColor));
            itemPbr->addChild(createPropertyTreeItem("Metallic", pbrMaterial.Metallic));
            itemPbr->addChild(createPropertyTreeItem("Roughness", pbrMaterial.Roughness));
            itemPbr->addChild(createPropertyTreeItem("RefractionIndex", pbrMaterial.RefractionIndex));
            itemPbr->addChild(createPropertyTreeItem("IsDefined", pbrMaterial.IsDefined));
            item->addChild(itemPbr);
        }

        if (material->HasCommonMaterial()) {
            const XCAFDoc_VisMaterialCommon& commonMaterial = material->CommonMaterial();
            auto itemCommon = new QTreeWidgetItem;
            itemCommon->setText(0, "CommonMaterial");
            itemCommon->addChild(createPropertyTreeItem("DiffuseTexture", commonMaterial.DiffuseTexture));
            itemCommon->addChild(createPropertyTreeItem("AmbientColor", commonMaterial.AmbientColor));
            itemCommon->addChild(createPropertyTreeItem("DiffuseColor", commonMaterial.DiffuseColor));
            itemCommon->addChild(createPropertyTreeItem("SpecularColor", commonMaterial.SpecularColor));
            itemCommon->addChild(createPropertyTreeItem("EmissiveColor", commonMaterial.EmissiveColor));
            itemCommon->addChild(createPropertyTreeItem("Shininess", commonMaterial.Shininess));
            itemCommon->addChild(createPropertyTreeItem("Transparency", commonMaterial.Transparency));
            itemCommon->addChild(createPropertyTreeItem("IsDefined", commonMaterial.IsDefined));
            item->addChild(itemCommon);
        }

        return item;
    };

    if (visMaterialTool->IsMaterial(label)) {
        OccHandle<XCAFDoc_VisMaterial> visMaterial = visMaterialTool->GetMaterial(label);
        if (!visMaterial.IsNull() && !visMaterial->IsEmpty())
            fnAddItem(fnCreateVisMaterialTreeItem("Material", visMaterial));
    }

    if (visMaterialTool->IsSetShapeMaterial(label)) {
        OccHandle<XCAFDoc_VisMaterial> visMaterial = visMaterialTool->GetShapeMaterial(label);
        if (!visMaterial.IsNull() && !visMaterial->IsEmpty())
            fnAddItem(fnCreateVisMaterialTreeItem("ShapeMaterial", visMaterial));
    }

    treeItem->addChildren(listItemProp);
}
#endif

static void loadLabelColorProperties(
        const TDF_Label& label,
        const OccHandle<XCAFDoc_ColorTool>& colorTool,
        QTreeWidgetItem* treeItem
    )
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    fnAddItem(createPropertyTreeItem("IsColor", colorTool->IsColor(label)));
    fnAddItem(createPropertyTreeItem("IsSet_ColorGen", colorTool->IsSet(label, XCAFDoc_ColorGen)));
    fnAddItem(createPropertyTreeItem("IsSet_ColorSurf", colorTool->IsSet(label, XCAFDoc_ColorSurf)));
    fnAddItem(createPropertyTreeItem("IsSet_ColorCurv", colorTool->IsSet(label, XCAFDoc_ColorCurv)));
    fnAddItem(createPropertyTreeItem("IsVisible", colorTool->IsColor(label)));

    Quantity_Color color;
    if (colorTool->GetColor(label, color))
        fnAddItem(createPropertyTreeItem("Color", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorGen, color))
        fnAddItem(createPropertyTreeItem("Color_ColorGen", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorSurf, color))
        fnAddItem(createPropertyTreeItem("Color_ColorSurf", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorCurv, color))
        fnAddItem(createPropertyTreeItem("Color_ColorCurv", color));

    treeItem->addChildren(listItemProp);
}

static void loadLabelShapeProperties(
        const TDF_Label& label,
        const OccHandle<XCAFDoc_ShapeTool>& shapeTool,
        QTreeWidgetItem* treeItem
    )
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    TopoDS_Shape shape;
    if (XCAFDoc_ShapeTool::GetShape(label, shape))
        fnAddItem(createPropertyTreeItem("ShapeType", MetaEnum::name(shape.ShapeType())));

    fnAddItem(createPropertyTreeItem("IsShape", shapeTool->IsShape(label)));
    fnAddItem(createPropertyTreeItem("IsTopLevel", shapeTool->IsTopLevel(label)));
    fnAddItem(createPropertyTreeItem("IsFree", shapeTool->IsFree(label)));
    fnAddItem(createPropertyTreeItem("IsAssembly", shapeTool->IsAssembly(label)));
    fnAddItem(createPropertyTreeItem("IsComponent", shapeTool->IsComponent(label)));
    fnAddItem(createPropertyTreeItem("IsSimpleShape", shapeTool->IsSimpleShape(label)));
    fnAddItem(createPropertyTreeItem("IsCompound", shapeTool->IsCompound(label)));
    fnAddItem(createPropertyTreeItem("IsSubShape", shapeTool->IsSubShape(label)));
    fnAddItem(createPropertyTreeItem("IsExternRef", shapeTool->IsExternRef(label)));
    fnAddItem(createPropertyTreeItem("IsReference", shapeTool->IsReference(label)));

    TDF_LabelSequence seqLabelUser;
    shapeTool->GetUsers(label, seqLabelUser);
    fnAddItem(createPropertyTreeItem("UserCount", seqLabelUser.Size()));

    if (XCAFDoc_ShapeTool::IsReference(label)) {
        TDF_Label labelRef;
        if (XCAFDoc_ShapeTool::GetReferredShape(label, labelRef)) {
            const QString textItemRefShape =
                    to_QString(CafUtils::labelTag(labelRef))
                    + " "
                    + to_QString(CafUtils::labelAttrStdName(labelRef))
                ;
            fnAddItem(createPropertyTreeItem("ReferredShape", textItemRefShape));
        }
    }

    treeItem->addChildren(listItemProp);
}

static void loadLabelDimensionProperties(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    auto fnAddChildItem = [&](QTreeWidgetItem* item) { listItemProp.back()->addChild(item); };
    auto dimAttr = CafUtils::findAttribute<XCAFDoc_Dimension>(label);
    if (!dimAttr)
        return;

    OccHandle<XCAFDimTolObjects_DimensionObject> dimObject = dimAttr->GetObject();
    fnAddItem(createPropertyTreeItem("SemanticName", dimObject->GetSemanticName()));
    fnAddItem(createPropertyTreeItem(
                  "Qualifier", MetaEnum::nameWithoutPrefix(dimObject->GetQualifier(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem(
                  "Type", MetaEnum::nameWithoutPrefix(dimObject->GetType(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem("Value", dimObject->GetValue()));

    fnAddItem(createPropertyTreeItem("IsDimWithRange", dimObject->IsDimWithRange()));
    if (dimObject->IsDimWithRange()) {
        fnAddChildItem(createPropertyTreeItem("UpperBound", dimObject->GetUpperBound()));
        fnAddChildItem(createPropertyTreeItem("LowerBound", dimObject->GetLowerBound()));
    }

    fnAddItem(createPropertyTreeItem("IsDimWithPlusMinusTolerance", dimObject->IsDimWithPlusMinusTolerance()));
    if (dimObject->IsDimWithPlusMinusTolerance()) {
        fnAddChildItem(createPropertyTreeItem("UpperTolValue", dimObject->GetUpperTolValue()));
        fnAddChildItem(createPropertyTreeItem("LowerTolValue", dimObject->GetLowerTolValue()));
    }

    fnAddItem(createPropertyTreeItem("IsDimWithClassOfTolerance", dimObject->IsDimWithClassOfTolerance()));
    if (dimObject->IsDimWithClassOfTolerance()) {
        Standard_Boolean hole;
        XCAFDimTolObjects_DimensionFormVariance formVariance;
        XCAFDimTolObjects_DimensionGrade grade;
        if (dimObject->GetClassOfTolerance(hole, formVariance, grade)) {
            fnAddChildItem(createPropertyTreeItem("IsHole", hole));
            fnAddChildItem(createPropertyTreeItem("DimensionFormVariance", MetaEnum::nameWithoutPrefix(formVariance, "XCAFDimTolObjects_")));
            fnAddChildItem(createPropertyTreeItem("DimensionGrade", MetaEnum::nameWithoutPrefix(grade, "XCAFDimTolObjects_")));
        }
    }

    fnAddItem(createPropertyTreeItem("Path", dimObject->GetPath()));

    fnAddItem(createPropertyTreeItem("HasTextPoint", dimObject->HasTextPoint()));
    if (dimObject->HasTextPoint())
        fnAddChildItem(createPropertyTreeItem("PointTextAttach", dimObject->GetPointTextAttach()));

    fnAddItem(createPropertyTreeItem("HasPlane", dimObject->HasPlane()));
    if (dimObject->HasPlane())
        fnAddChildItem(createPropertyTreeItem("Plane", dimObject->GetPlane()));

    fnAddItem(createPropertyTreeItem("HasPoint", dimObject->HasPoint()));
    if (dimObject->HasPoint())
        fnAddChildItem(createPropertyTreeItem("Point", dimObject->GetPoint()));

    fnAddItem(createPropertyTreeItem("HasPoint2", dimObject->HasPoint2()));
    if (dimObject->HasPoint2())
        fnAddChildItem(createPropertyTreeItem("Point2", dimObject->GetPoint2()));

    fnAddItem(createPropertyTreeItem("PresentationName", dimObject->GetPresentationName()));
    fnAddItem(createPropertyTreeItem("Presentation", dimObject->GetPresentation()));

    fnAddItem(createPropertyTreeItem("HasDescriptions", dimObject->HasDescriptions()));
    if (dimObject->HasDescriptions()) {
        for (int i = 1; i <= dimObject->NbDescriptions(); ++i)
            fnAddChildItem(createPropertyTreeItem("DescriptionName", dimObject->GetDescriptionName(i)));
    }

    treeItem->addChildren(listItemProp);
}

static void loadLabelDatumProperties(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    auto fnAddChildItem = [&](QTreeWidgetItem* item) { listItemProp.back()->addChild(item); };
    auto datumAttr = CafUtils::findAttribute<XCAFDoc_Datum>(label);
    if (!datumAttr)
        return;

    OccHandle<XCAFDimTolObjects_DatumObject> datumObject = datumAttr->GetObject();
    fnAddItem(createPropertyTreeItem("SemanticName", datumObject->GetSemanticName()));
    fnAddItem(createPropertyTreeItem("Name", datumObject->GetName()));
    {
        XCAFDimTolObjects_DatumModifWithValue modfValue;
        double value;
        datumObject->GetModifierWithValue(modfValue, value);
        fnAddItem(createPropertyTreeItem("Modifier", MetaEnum::nameWithoutPrefix(modfValue, "XCAFDimTolObjects_")));
        if (modfValue != XCAFDimTolObjects_DatumModifWithValue_None)
            fnAddChildItem(createPropertyTreeItem("ModifierValue", value));
    }

    fnAddItem(createPropertyTreeItem("DatumTarget", datumObject->GetDatumTarget()));
    fnAddItem(createPropertyTreeItem("Position", datumObject->GetPosition()));
    fnAddItem(createPropertyTreeItem("IsDatumTarget", datumObject->IsDatumTarget()));
    fnAddItem(createPropertyTreeItem(
                  "DatumTargetType",
                  MetaEnum::nameWithoutPrefix(datumObject->GetDatumTargetType(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem("HasDatumTargetParams", datumObject->HasDatumTargetParams()));
    if (datumObject->HasDatumTargetParams()) {
        fnAddChildItem(createPropertyTreeItem("DatumTargetAxis", datumObject->GetDatumTargetAxis()));
        fnAddChildItem(createPropertyTreeItem("DatumTargetLength", datumObject->GetDatumTargetLength()));
        fnAddChildItem(createPropertyTreeItem("DatumTargetWidth", datumObject->GetDatumTargetWidth()));
        fnAddChildItem(createPropertyTreeItem("DatumTargetNumber", datumObject->GetDatumTargetNumber()));
    }

    fnAddItem(createPropertyTreeItem("HasPlane", datumObject->HasPlane()));
    if (datumObject->HasPlane())
        fnAddChildItem(createPropertyTreeItem("Plane", datumObject->GetPlane()));

    fnAddItem(createPropertyTreeItem("HasPoint", datumObject->HasPoint()));
    if (datumObject->HasPoint())
        fnAddChildItem(createPropertyTreeItem("Point", datumObject->GetPoint()));

    fnAddItem(createPropertyTreeItem("HasTextPoint", datumObject->HasPointText()));
    if (datumObject->HasPointText())
        fnAddChildItem(createPropertyTreeItem("PointTextAttach", datumObject->GetPointTextAttach()));

    fnAddItem(createPropertyTreeItem("Presentation", datumObject->GetPresentation()));
    fnAddItem(createPropertyTreeItem("PresentationName", datumObject->GetPresentationName()));

    treeItem->addChildren(listItemProp);
}

static void loadLabelGeomToleranceProperties(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    auto fnAddItem = [&](QTreeWidgetItem* item) { listItemProp.push_back(item); };
    auto fnAddChildItem = [&](QTreeWidgetItem* item) { listItemProp.back()->addChild(item); };
    auto tolAttr = CafUtils::findAttribute<XCAFDoc_GeomTolerance>(label);
    if (!tolAttr)
        return;

    OccHandle<XCAFDimTolObjects_GeomToleranceObject> tolObject = tolAttr->GetObject();
    fnAddItem(createPropertyTreeItem("SemanticName", tolObject->GetSemanticName()));
    fnAddItem(createPropertyTreeItem(
                  "Type", MetaEnum::nameWithoutPrefix(tolObject->GetType(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem(
                  "TypeOfValue", MetaEnum::nameWithoutPrefix(tolObject->GetTypeOfValue(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem("Value", tolObject->GetValue()));
    fnAddItem(createPropertyTreeItem(
                  "MaterialRequirementModifier",
                  MetaEnum::nameWithoutPrefix(tolObject->GetMaterialRequirementModifier(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem(
                  "ZoneModifier",
                  MetaEnum::nameWithoutPrefix(tolObject->GetZoneModifier(), "XCAFDimTolObjects_"))
    );
    fnAddItem(createPropertyTreeItem("ValueOfZoneModifier", tolObject->GetValueOfZoneModifier()));
    fnAddItem(createPropertyTreeItem("MaxValueModifier", tolObject->GetMaxValueModifier()));
    fnAddItem(createPropertyTreeItem("HasAxis", tolObject->HasAxis()));
    if (tolObject->HasAxis())
        fnAddChildItem(createPropertyTreeItem("Axis", tolObject->GetAxis()));

    fnAddItem(createPropertyTreeItem("HasPlane", tolObject->HasPlane()));
    if (tolObject->HasPlane())
        fnAddChildItem(createPropertyTreeItem("Plane", tolObject->GetPlane()));

    fnAddItem(createPropertyTreeItem("HasPoint", tolObject->HasPoint()));
    if (tolObject->HasPoint())
        fnAddChildItem(createPropertyTreeItem("Point", tolObject->GetPoint()));

    fnAddItem(createPropertyTreeItem("HasPointText", tolObject->HasPointText()));
    if (tolObject->HasPointText())
        fnAddChildItem(createPropertyTreeItem("PointTextAttach", tolObject->GetPointTextAttach()));

    fnAddItem(createPropertyTreeItem("Presentation", tolObject->GetPresentation()));
    fnAddItem(createPropertyTreeItem("PresentationName", tolObject->GetPresentationName()));
    fnAddItem(createPropertyTreeItem("HasAffectedPlane", tolObject->HasAffectedPlane()));
    if (tolObject->HasAffectedPlane()) {
        fnAddChildItem(createPropertyTreeItem(
                           "AffectedPlaneType",
                           MetaEnum::nameWithoutPrefix(tolObject->GetAffectedPlaneType(), "XCAFDimTolObjects_"))
        );
        fnAddChildItem(createPropertyTreeItem(
                           "AffectedPlaneType", tolObject->GetAffectedPlane().Position().Ax2())
        );
    }

    treeItem->addChildren(listItemProp);
}

static void loadLabel(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    treeItem->setData(0, TreeWidgetItem_TdfLabelRole, QVariant::fromValue(label));
    treeItem->setText(0, to_QString(CafUtils::labelTag(label)));

    const QString stdName = to_QString(CafUtils::labelAttrStdName(label));
    if (!stdName.isEmpty())
        treeItem->setText(0, treeItem->text(0) + " " + stdName);
}

static void deepLoadChildrenLabels(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    for (TDF_ChildIterator it(label, false/*!allLevels*/); it.More(); it.Next()) {
        const TDF_Label childLabel = it.Value();
        if (!CafUtils::isNullOrEmpty(childLabel)) {
            auto childTreeItem = new QTreeWidgetItem(treeItem);
            loadLabel(childLabel, childTreeItem);
            deepLoadChildrenLabels(childLabel, childTreeItem);
        }
    }
}

} // namespace Internal

DialogInspectXde::DialogInspectXde(QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogInspectXde)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 1);
    m_ui->splitter->setStretchFactor(1, 4);
    QObject::connect(
        m_ui->treeWidget_Document, &QTreeWidget::itemClicked,
        this, &DialogInspectXde::onLabelTreeWidgetItemClicked
    );
}

DialogInspectXde::~DialogInspectXde()
{
    delete m_ui;
}

void DialogInspectXde::load(const OccHandle<TDocStd_Document>& doc)
{
    m_doc = doc;
    if (!XCAFDoc_DocumentTool::IsXCAFDocument(doc)) {
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), tr("This document is not suitable for XDE"));
        return;
    }

    if (!doc.IsNull()) {
        const TDF_Label label = doc->Main();
        auto treeItem = new QTreeWidgetItem;
        Internal::loadLabel(label, treeItem);
        Internal::deepLoadChildrenLabels(label, treeItem);
        m_ui->treeWidget_Document->addTopLevelItem(treeItem);
        treeItem->setExpanded(true);
    }
}

void DialogInspectXde::onLabelTreeWidgetItemClicked(QTreeWidgetItem* item, int /*column*/)
{
    const QVariant varLabel = item->data(0, Internal::TreeWidgetItem_TdfLabelRole);
    if (varLabel.isValid()) {
        m_ui->treeWidget_LabelProps->clear();
        const TDF_Label label = varLabel.value<TDF_Label>();
        if (label.HasAttribute()) {
            auto itemAttrs = new QTreeWidgetItem;
            itemAttrs->setText(0, tr("Attributes"));
            Internal::loadLabelAttributes(label, itemAttrs);
            m_ui->treeWidget_LabelProps->addTopLevelItem(itemAttrs);
        }

        auto fnCreateLabelTreeItem = [=](const QString& title) {
            auto treeItem = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            treeItem->setText(0, title);
            return treeItem;
        };

        auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_doc->Main());
        const bool isShapeLabel = XCAFDoc_ShapeTool::IsShape(label);
        if (isShapeLabel)
            Internal::loadLabelShapeProperties(label, shapeTool, fnCreateLabelTreeItem(tr("Shape")));

        auto colorTool = XCAFDoc_DocumentTool::ColorTool(m_doc->Main());
        if (colorTool->IsColor(label) || isShapeLabel)
            Internal::loadLabelColorProperties(label, colorTool, fnCreateLabelTreeItem(tr("Color")));

        auto materialTool = XCAFDoc_DocumentTool::MaterialTool(m_doc->Main());
        if (materialTool->IsMaterial(label) || isShapeLabel)
            Internal::loadLabelMaterialProperties(label, materialTool, fnCreateLabelTreeItem(tr("Material")));

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        auto visMaterialTool = XCAFDoc_DocumentTool::VisMaterialTool(m_doc->Main());
        if (visMaterialTool->IsMaterial(label) || isShapeLabel)
            Internal::loadLabelVisMaterialProperties(label, visMaterialTool, fnCreateLabelTreeItem(tr("VisMaterial")));
#endif

        auto pmiTool = XCAFDoc_DocumentTool::DimTolTool(m_doc->Main());
        if (pmiTool->IsDimension(label))
            Internal::loadLabelDimensionProperties(label, fnCreateLabelTreeItem(tr("Dimension")));

        if (pmiTool->IsDatum(label))
            Internal::loadLabelDatumProperties(label, fnCreateLabelTreeItem(tr("Datum")));

        if (pmiTool->IsGeomTolerance(label))
            Internal::loadLabelGeomToleranceProperties(label, fnCreateLabelTreeItem(tr("GeomTolerance")));
    }

    m_ui->treeWidget_LabelProps->expandAll();
    for (int i = 0; i < m_ui->treeWidget_LabelProps->columnCount(); ++i)
        m_ui->treeWidget_LabelProps->resizeColumnToContents(i);
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_inspect_xde.h"

#include "../base/application.h"
#include "../base/caf_utils.h"
#include "../base/meta_enum.h"
#include "../base/qmeta_tdf_label.h"
#include "../base/settings.h"
#include "../base/string_utils.h"
#include "../base/tkernel_utils.h"
#include "app_module.h"
#include "ui_dialog_inspect_xde.h"

#include <fougtools/qttools/gui/qwidget_utils.h>
#include <fougtools/occtools/qt_utils.h>

#include <Image_Texture.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
#include <TNaming_NamedShape.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_Color.hxx>
#include <XCAFDoc_ColorType.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_Volume.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include <XCAFDoc_VisMaterial.hxx>
#  include <XCAFDoc_VisMaterialCommon.hxx>
#  include <XCAFDoc_VisMaterialTool.hxx>
#endif

#include <sstream>

namespace Mayo {

namespace Internal {

enum TreeWidgetItemRole {
    TreeWidgetItem_TdfLabelRole = Qt::UserRole + 1
};

static void loadLabelAttributes(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    for (TDF_AttributeIterator it(label); it.More(); it.Next()) {
        const Handle_TDF_Attribute ptrAttr = it.Value();
        const Standard_GUID& attrId = ptrAttr->ID();
        QString text;
        QString value;
        if (attrId == TDataStd_Name::GetID()) {
            const auto& name = static_cast<const TDataStd_Name&>(*ptrAttr);
            text = "TDataStd_Name";
            value = occ::QtUtils::toQString(name.Get());
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
            value = StringUtils::text(
                        area.Get(), AppModule::get(Application::instance())->defaultTextOptions());
        }
        else if (attrId == XCAFDoc_Centroid::GetID()) {
            const auto& centroid = static_cast<const XCAFDoc_Centroid&>(*ptrAttr);
            text = "XCAFDoc_Centroid";
            value = StringUtils::text(
                        centroid.Get(), AppModule::get(Application::instance())->defaultTextOptions());
        }
        else if (attrId == XCAFDoc_Volume::GetID()) {
            const auto& volume = static_cast<const XCAFDoc_Volume&>(*ptrAttr);
            text = "XCAFDoc_Volume";
            value = StringUtils::text(
                        volume.Get(), AppModule::get(Application::instance())->defaultTextOptions());
        }
        else if (attrId == XCAFDoc_Color::GetID()) {
            const auto& color = static_cast<const XCAFDoc_Color&>(*ptrAttr);
            text = "XCAFDoc_Color";
            value = StringUtils::text(color.GetColor());
        }
        else if (attrId == XCAFDoc_Location::GetID()) {
            const auto& location = static_cast<const XCAFDoc_Location&>(*ptrAttr);
            text = "XCAFDoc_Location";
            value = StringUtils::text(
                        location.Get().Transformation(),
                        AppModule::get(Application::instance())->defaultTextOptions());
        }
        else if (attrId == TNaming_NamedShape::GetID()) {
            const auto& namedShape = static_cast<const TNaming_NamedShape&>(*ptrAttr);
            text = "TNaming_NamedShape";
            value = DialogInspectXde::tr("ShapeType=%1, Evolution=%2")
                    .arg(MetaEnum::name(namedShape.Get().ShapeType()).data())
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
            text = wordStart < strDump.size() ?
                        strDump.mid(wordStart, wordEnd - wordStart) :
                        QStringLiteral("??");
            strDump = strDump.right(strDump.size() - dataStart);
            value = strDump.replace(QChar('\n'), QString("  "));
        }

        auto attrTreeItem = new QTreeWidgetItem;
        attrTreeItem->setText(0, text);
        attrTreeItem->setText(1, value);
        treeItem->addChild(attrTreeItem);
    }
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, bool isPropertyOn)
{
    auto itemProperty = new QTreeWidgetItem;
    itemProperty->setText(0, text);
    const QString strYes = DialogInspectXde::tr("Yes");
    const QString strNo = DialogInspectXde::tr("No");
    itemProperty->setText(1, isPropertyOn ? strYes : strNo);
    itemProperty->setForeground(1, isPropertyOn ? Qt::green : QColor(Qt::red).lighter());
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, double propertyValue)
{
    auto itemProperty = new QTreeWidgetItem;
    itemProperty->setText(0, text);
    const StringUtils::TextOptions textOptions = AppModule::get(Application::instance())->defaultTextOptions();
    itemProperty->setText(1, StringUtils::text(propertyValue, textOptions));
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const QString& propertyValue)
{
    auto itemProperty = new QTreeWidgetItem;
    itemProperty->setText(0, text);
    itemProperty->setText(1, propertyValue);
    return itemProperty;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, std::string_view propertyValue)
{
    return createPropertyTreeItem(text, QString::fromUtf8(propertyValue.data()));
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const Quantity_Color& color)
{
    auto itemColor = new QTreeWidgetItem;
    itemColor->setText(0, text);
    itemColor->setText(1, StringUtils::text(color));
    QPixmap pixColor(24, 16);
    pixColor.fill(occ::QtUtils::toQColor(color));
    itemColor->setIcon(1, pixColor);
    return itemColor;
}

static QTreeWidgetItem* createPropertyTreeItem(const QString& text, const Quantity_ColorRGBA& color)
{
    auto itemColor = createPropertyTreeItem(text, color.GetRGB());
    itemColor->setText(1, itemColor->text(1) + QString(" A:%1").arg(int(color.Alpha() * 255)));
    return itemColor;
}

static QTreeWidgetItem* createPropertyTreeItem(
        const QString& text, const opencascade::handle<Image_Texture>& imgTexture)
{
    if (imgTexture.IsNull())
        return static_cast<QTreeWidgetItem*>(nullptr);

    auto item = new QTreeWidgetItem;
    item->setText(0, text);
    item->setText(1, occ::QtUtils::fromUtf8ToQString(imgTexture->FilePath()));
    return item;
}

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
static void loadLabelVisMaterialProperties(
        const TDF_Label& label,
        const Handle_XCAFDoc_VisMaterialTool& visMaterialTool,
        QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    listItemProp.push_back(createPropertyTreeItem("IsMaterial", visMaterialTool->IsMaterial(label)));
    listItemProp.push_back(createPropertyTreeItem("IsSetShapeMaterial", visMaterialTool->IsSetShapeMaterial(label)));

    auto fnCreateVisMaterialTreeItem = [](const QString& text, const Handle_XCAFDoc_VisMaterial& material) {
        auto item = new QTreeWidgetItem;
        item->setText(0, text);
        item->addChild(createPropertyTreeItem("HasPbrMaterial", material->HasPbrMaterial()));
        item->addChild(createPropertyTreeItem("HasCommonMaterial", material->HasCommonMaterial()));
        item->addChild(createPropertyTreeItem("BaseColor", material->BaseColor()));
        item->addChild(createPropertyTreeItem("AlphaMode", MetaEnum::name(material->AlphaMode())));
        item->addChild(createPropertyTreeItem("AlphaCutOff", material->AlphaCutOff()));
        item->addChild(createPropertyTreeItem("IsDoubleSided", material->IsDoubleSided()));
        if (!material->RawName().IsNull())
            item->addChild(createPropertyTreeItem("RawName", occ::QtUtils::fromUtf8ToQString(material->RawName()->String())));

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
        Handle_XCAFDoc_VisMaterial visMaterial = visMaterialTool->GetMaterial(label);
        if (!visMaterial.IsNull() && !visMaterial->IsEmpty())
            listItemProp.push_back(fnCreateVisMaterialTreeItem("Material", visMaterial));
    }

    if (visMaterialTool->IsSetShapeMaterial(label)) {
        Handle_XCAFDoc_VisMaterial visMaterial = visMaterialTool->GetShapeMaterial(label);
        if (!visMaterial.IsNull() && !visMaterial->IsEmpty())
            listItemProp.push_back(fnCreateVisMaterialTreeItem("ShapeMaterial", visMaterial));
    }

    treeItem->addChildren(listItemProp);
}
#endif

static void loadLabelColorProperties(
        const TDF_Label& label,
        const Handle_XCAFDoc_ColorTool& colorTool,
        QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;
    listItemProp.push_back(createPropertyTreeItem("IsColor", colorTool->IsColor(label)));
    listItemProp.push_back(createPropertyTreeItem("IsSet_ColorGen", colorTool->IsSet(label, XCAFDoc_ColorGen)));
    listItemProp.push_back(createPropertyTreeItem("IsSet_ColorSurf", colorTool->IsSet(label, XCAFDoc_ColorSurf)));
    listItemProp.push_back(createPropertyTreeItem("IsSet_ColorCurv", colorTool->IsSet(label, XCAFDoc_ColorCurv)));
    listItemProp.push_back(createPropertyTreeItem("IsVisible", colorTool->IsColor(label)));

    Quantity_Color color;
    if (colorTool->GetColor(label, color))
        listItemProp.push_back(createPropertyTreeItem("Color", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorGen, color))
        listItemProp.push_back(createPropertyTreeItem("Color_ColorGen", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorSurf, color))
        listItemProp.push_back(createPropertyTreeItem("Color_ColorSurf", color));

    if (colorTool->GetColor(label, XCAFDoc_ColorCurv, color))
        listItemProp.push_back(createPropertyTreeItem("Color_ColorCurv", color));

    treeItem->addChildren(listItemProp);
}

static void loadLabelShapeProperties(
        const TDF_Label& label,
        const Handle_XCAFDoc_ShapeTool& shapeTool,
        QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;

    TopoDS_Shape shape;
    if (XCAFDoc_ShapeTool::GetShape(label, shape))
        listItemProp.push_back(createPropertyTreeItem("ShapeType", MetaEnum::name(shape.ShapeType())));

    listItemProp.push_back(createPropertyTreeItem("IsShape", shapeTool->IsShape(label)));
    listItemProp.push_back(createPropertyTreeItem("IsTopLevel", shapeTool->IsTopLevel(label)));
    listItemProp.push_back(createPropertyTreeItem("IsFree", shapeTool->IsFree(label)));
    listItemProp.push_back(createPropertyTreeItem("IsAssembly", shapeTool->IsAssembly(label)));
    listItemProp.push_back(createPropertyTreeItem("IsComponent", shapeTool->IsComponent(label)));
    listItemProp.push_back(createPropertyTreeItem("IsSimpleShape", shapeTool->IsSimpleShape(label)));
    listItemProp.push_back(createPropertyTreeItem("IsCompound", shapeTool->IsCompound(label)));
    listItemProp.push_back(createPropertyTreeItem("IsSubShape", shapeTool->IsSubShape(label)));
    listItemProp.push_back(createPropertyTreeItem("IsExternRef", shapeTool->IsExternRef(label)));
    listItemProp.push_back(createPropertyTreeItem("IsReference", shapeTool->IsReference(label)));

    if (XCAFDoc_ShapeTool::IsReference(label)) {
        TDF_Label labelRef;
        if (XCAFDoc_ShapeTool::GetReferredShape(label, labelRef)) {
            const QString textItemRefShape =
                    CafUtils::labelTag(labelRef) + " " + CafUtils::labelAttrStdName(labelRef);
            listItemProp.push_back(createPropertyTreeItem("ReferredShape", textItemRefShape));
        }
    }

    treeItem->addChildren(listItemProp);
}

static void loadLabel(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    treeItem->setData(0, TreeWidgetItem_TdfLabelRole, QVariant::fromValue(label));
    treeItem->setText(0, CafUtils::labelTag(label));

    const QString stdName = CafUtils::labelAttrStdName(label);
    if (!stdName.isEmpty())
        treeItem->setText(0, treeItem->text(0) + " " + stdName);
}

static void deepLoadChildrenLabels(const TDF_Label& label, QTreeWidgetItem* treeItem)
{
    for (TDF_ChildIterator it(label, Standard_False); it.More(); it.Next()) {
        const TDF_Label childLabel = it.Value();
        auto childTreeItem = new QTreeWidgetItem(treeItem);
        loadLabel(childLabel, childTreeItem);
        deepLoadChildrenLabels(childLabel, childTreeItem);
    }
}

} // namespace Internal

DialogInspectXde::DialogInspectXde(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogInspectXde)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 1);
    m_ui->splitter->setStretchFactor(1, 4);
    QObject::connect(
                m_ui->treeWidget_Document, &QTreeWidget::itemClicked,
                this, &DialogInspectXde::onLabelTreeWidgetItemClicked);
}

DialogInspectXde::~DialogInspectXde()
{
    delete m_ui;
}

void DialogInspectXde::load(const Handle_TDocStd_Document& doc)
{
    m_doc = doc;
    if (!XCAFDoc_DocumentTool::IsXCAFDocument(doc)) {
        qtgui::QWidgetUtils::asyncMsgBoxCritical(
                    this, tr("Error"), tr("This document is not suitable for XDE"));
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

void DialogInspectXde::onLabelTreeWidgetItemClicked(QTreeWidgetItem *item, int /*column*/)
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

        const bool isShapeLabel = XCAFDoc_ShapeTool::IsShape(label);
        if (isShapeLabel) {
            auto shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_doc->Main());
            auto itemShapeProps = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            itemShapeProps->setText(0, tr("Shape"));
            Internal::loadLabelShapeProperties(label, shapeTool, itemShapeProps);
        }

        auto colorTool = XCAFDoc_DocumentTool::ColorTool(m_doc->Main());
        if (colorTool->IsColor(label) || isShapeLabel) {
            auto itemColorProps = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            itemColorProps->setText(0, tr("Color"));
            Internal::loadLabelColorProperties(label, colorTool, itemColorProps);
        }

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
        auto visMaterialTool = XCAFDoc_DocumentTool::VisMaterialTool(m_doc->Main());
        if (visMaterialTool->IsMaterial(label) || isShapeLabel) {
            auto itemVisMaterialProps = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            itemVisMaterialProps->setText(0, tr("VisMaterial"));
            Internal::loadLabelVisMaterialProperties(label, visMaterialTool, itemVisMaterialProps);
        }
#endif
    }

    m_ui->treeWidget_LabelProps->expandAll();
    for (int i = 0; i < m_ui->treeWidget_LabelProps->columnCount(); ++i)
        m_ui->treeWidget_LabelProps->resizeColumnToContents(i);
}

} // namespace Mayo

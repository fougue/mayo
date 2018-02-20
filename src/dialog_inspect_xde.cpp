#include "dialog_inspect_xde.h"

#include "brep_utils.h"
#include "caf_utils.h"
#include "ui_dialog_inspect_xde.h"

#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/occtools/qt_utils.h"

#include <QtCore/QMetaType>

#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_UAttribute.hxx>
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

#include <sstream>
#include <tuple>

Q_DECLARE_METATYPE(TDF_Label)

namespace Mayo {

namespace Internal {

enum TreeWidgetItemRole {
    TreeWidgetItem_TdfLabelRole = Qt::UserRole + 1
};

static QString occColorToQString(
        const Quantity_Color& color, const QString& format = "R:%1 G:%2 B:%3")
{
    return format.arg(color.Red()).arg(color.Green()).arg(color.Blue());
}

static void loadLabelAttributes(const TDF_Label &label, QTreeWidgetItem *treeItem)
{
    // if (label.HasAttribute())
    for (TDF_AttributeIterator it(label); it.More(); it.Next()) {
        const TDF_Attribute* ptrAttr = it.Value();
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
            value = QString::number(area.Get());
        }
        else if (attrId == XCAFDoc_Centroid::GetID()) {
            const auto& centroid = static_cast<const XCAFDoc_Centroid&>(*ptrAttr);
            text = "XCAFDoc_Centroid";
            value = occ::QtUtils::toQString(centroid.Get());
        }
        else if (attrId == XCAFDoc_Volume::GetID()) {
            const auto& volume = static_cast<const XCAFDoc_Volume&>(*ptrAttr);
            text = "XCAFDoc_Volume";
            value = QString::number(volume.Get());
        }
        else if (attrId == XCAFDoc_Color::GetID()) {
            const auto& color = static_cast<const XCAFDoc_Color&>(*ptrAttr);
            text = "XCAFDoc_Color";
            value = occColorToQString(color.GetColor());
        }
        else if (attrId == XCAFDoc_Location::GetID()) {
            const auto& location = static_cast<const XCAFDoc_Location&>(*ptrAttr);
            text = "XCAFDoc_Location";
            value = !location.Get().IsIdentity() ? "identity" : "matrix_4x4";
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

static QTreeWidgetItem* createOccColorTreeItem(
        const QString& text, const Quantity_Color& color)
{
    auto itemColor = new QTreeWidgetItem;
    itemColor->setText(0, text);
    itemColor->setText(1, occColorToQString(color));
    QPixmap pixColor(24, 16);
    pixColor.fill(occ::QtUtils::toQColor(color));
    itemColor->setIcon(1, pixColor);
    return itemColor;
}

static void loadLabelColorProperties(
        const TDF_Label& label,
        const Handle_XCAFDoc_ColorTool& colorTool,
        QTreeWidgetItem* treeItem)
{
    typedef std::tuple<QString, bool> ColorProperty;
    auto listColorProperty = {
        ColorProperty("IsColor", colorTool->IsColor(label)),
        ColorProperty("IsSet_ColorGen", colorTool->IsSet(label, XCAFDoc_ColorGen)),
        ColorProperty("IsSet_ColorSurf", colorTool->IsSet(label, XCAFDoc_ColorSurf)),
        ColorProperty("IsSet_ColorCurv", colorTool->IsSet(label, XCAFDoc_ColorCurv)),
        ColorProperty("IsVisible", colorTool->IsVisible(label))
    };
    QList<QTreeWidgetItem*> listItemProp;
    const QString strYes = DialogInspectXde::tr("Yes");
    const QString strNo = DialogInspectXde::tr("No");
    for (const ColorProperty& prop : listColorProperty) {
        auto itemProp = new QTreeWidgetItem;
        const bool isPropOn = std::get<1>(prop);
        itemProp->setText(0, std::get<0>(prop));
        itemProp->setText(1, isPropOn ? strYes : strNo);
        itemProp->setForeground(1, isPropOn ? Qt::green : Qt::red);
        listItemProp.push_back(itemProp);
    }

    Quantity_Color color;
    if (colorTool->GetColor(label, color))
        listItemProp.push_back(createOccColorTreeItem("Color", color));
    if (colorTool->GetColor(label, XCAFDoc_ColorGen, color))
        listItemProp.push_back(createOccColorTreeItem("Color_ColorGen", color));
    if (colorTool->GetColor(label, XCAFDoc_ColorSurf, color))
        listItemProp.push_back(createOccColorTreeItem("Color_ColorSurf", color));
    if (colorTool->GetColor(label, XCAFDoc_ColorCurv, color))
        listItemProp.push_back(createOccColorTreeItem("Color_ColorCurv", color));

    treeItem->addChildren(listItemProp);
}

static void loadLabelShapeProperties(
        const TDF_Label& label,
        const Handle_XCAFDoc_ShapeTool shapeTool,
        QTreeWidgetItem* treeItem)
{
    QList<QTreeWidgetItem*> listItemProp;

    TopoDS_Shape shape;
    if (shapeTool->GetShape(label, shape)) {
        auto itemShapeType = new QTreeWidgetItem;
        itemShapeType->setText(0, DialogInspectXde::tr("ShapeType"));
        const char* cstrShapeType = occ::BRepUtils::shapeTypeToString(shape.ShapeType());
        itemShapeType->setText(1, QString::fromLatin1(cstrShapeType));
        listItemProp.push_back(itemShapeType);
    }

    typedef std::tuple<QString, bool> ShapeProperty;
    auto listShapeProperty = {
        ShapeProperty("IsShape", shapeTool->IsShape(label)),
        ShapeProperty("IsTopLevel", shapeTool->IsTopLevel(label)),
        ShapeProperty("IsFree", shapeTool->IsFree(label)),
        ShapeProperty("IsAssembly", shapeTool->IsAssembly(label)),
        ShapeProperty("IsComponent", shapeTool->IsComponent(label)),
        ShapeProperty("IsSimpleShape", shapeTool->IsSimpleShape(label)),
        ShapeProperty("IsCompound", shapeTool->IsCompound(label)),
        ShapeProperty("IsSubShape", shapeTool->IsSubShape(label)),
        ShapeProperty("IsExternRef", shapeTool->IsExternRef(label)),
        ShapeProperty("IsReference", shapeTool->IsReference(label))
    };
    const QString strYes = DialogInspectXde::tr("Yes");
    const QString strNo = DialogInspectXde::tr("No");
    for (const ShapeProperty& prop : listShapeProperty) {
        auto itemProp = new QTreeWidgetItem;
        const bool isPropOn = std::get<1>(prop);
        itemProp->setText(0, std::get<0>(prop));
        itemProp->setText(1, isPropOn ? strYes : strNo);
        itemProp->setForeground(1, isPropOn ? Qt::green : Qt::red);
        listItemProp.push_back(itemProp);
    }

    if (shapeTool->IsReference(label)) {
        TDF_Label labelRef;
        if (shapeTool->GetReferredShape(label, labelRef)) {
            auto itemRefShape = new QTreeWidgetItem;
            itemRefShape->setText(0, DialogInspectXde::tr("ReferredShape"));
            const QString textItemRefShape =
                    occ::CafUtils::labelTag(labelRef)
                    + " "
                    + occ::CafUtils::labelAttrStdName(labelRef);
            itemRefShape->setText(1, textItemRefShape);
            listItemProp.push_back(itemRefShape);
        }
    }

    treeItem->addChildren(listItemProp);
}

static void loadLabel(const TDF_Label &label, QTreeWidgetItem *treeItem)
{
    treeItem->setData(0, TreeWidgetItem_TdfLabelRole, QVariant::fromValue(label));
    treeItem->setText(0, occ::CafUtils::labelTag(label));

    const QString stdName = occ::CafUtils::labelAttrStdName(label);
    if (!stdName.isEmpty())
        treeItem->setText(0, treeItem->text(0) + " " + stdName);
}

static void deepLoadChildrenLabels(
        const TDF_Label &label, QTreeWidgetItem *treeItem)
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
    QObject::connect(
                m_ui->treeWidget_Document, &QTreeWidget::itemClicked,
                this, &DialogInspectXde::onLabelTreeWidgetItemClicked);
}

DialogInspectXde::~DialogInspectXde()
{
    delete m_ui;
}

void DialogInspectXde::load(const Handle_TDocStd_Document &doc)
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
//        TDF_LabelSequence seqFreeShapeLabel;
//        const Handle_XCAFDoc_ShapeTool shapeTool =
//                XCAFDoc_DocumentTool::ShapeTool(doc->Main());
//        shapeTool->GetFreeShapes(seqFreeShapeLabel);
    }
}

void DialogInspectXde::onLabelTreeWidgetItemClicked(
        QTreeWidgetItem *item, int /*column*/)
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

        const Handle_XCAFDoc_ShapeTool shapeTool =
                XCAFDoc_DocumentTool::ShapeTool(m_doc->Main());
        if (shapeTool->IsShape(label)) {
            auto itemShapeProps = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            itemShapeProps->setText(0, tr("Shape specific"));
            Internal::loadLabelShapeProperties(label, shapeTool, itemShapeProps);
        }

        const Handle_XCAFDoc_ColorTool colorTool =
                XCAFDoc_DocumentTool::ColorTool(m_doc->Main());
        if (colorTool->IsColor(label) || shapeTool->IsShape(label)) {
            auto itemColorProps = new QTreeWidgetItem(m_ui->treeWidget_LabelProps);
            itemColorProps->setText(0, tr("Color specific"));
            Internal::loadLabelColorProperties(label, colorTool, itemColorProps);
        }
    }
    m_ui->treeWidget_LabelProps->expandAll();
    for (int i = 0; i < m_ui->treeWidget_LabelProps->columnCount(); ++i)
        m_ui->treeWidget_LabelProps->resizeColumnToContents(i);
}

} // namespace Mayo

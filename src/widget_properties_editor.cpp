/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_properties_editor.h"

#include "document.h"
#include "document_item.h"
#include "gui_application.h"
#include "gui_document.h"
#include "gpx_document_item.h"
#include "options.h"
#include "string_utils.h"
#include "unit_system.h"
#include "ui_widget_properties_editor.h"
#include <fougtools/occtools/qt_utils.h>
#include <fougtools/qttools/gui/qwidget_utils.h>

#include <QtGui/QPainter>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QToolButton>

#include <functional>
#include <unordered_map>

namespace Mayo {

namespace Internal {

class PanelEditor : public QWidget {
public:
    PanelEditor(QWidget* parent = nullptr)
        : QWidget(parent)
    {}

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);

        // This is needed by the "classic" theme, force a painted background
        const QRect frame = this->frameGeometry();
        const QRect surface(0, 0, frame.width(), frame.height());
        const QColor panelColor = palette().color(QPalette::Base);
        painter.fillRect(surface, panelColor);

        QStyleOption option;
        option.initFrom(this);
        option.state |= QStyle::State_HasFocus;
        this->style()->drawPrimitive(QStyle::PE_FrameLineEdit, &option, &painter, this);
    }
};

static QPixmap colorSquarePixmap(const QColor& c, int sideLen = 16)
{
    QPixmap pixColor(sideLen, sideLen);
    pixColor.fill(c);
    return pixColor;
}

static QWidget* hSpacerWidget(QWidget* parent, int stretch = 1)
{
    auto widget = new QWidget(parent);
    QSizePolicy sp = widget->sizePolicy();
    sp.setHorizontalStretch(stretch);
    widget->setSizePolicy(sp);
    return widget;
}

static QString yesNoString(bool on)
{
    return on ? WidgetPropertiesEditor::tr("Yes") : WidgetPropertiesEditor::tr("No");
}

static QString propertyValueText(const Property* prop)
{
    auto options = Options::instance();
    const QLocale locale = options->locale();
    const char* propTypeName = prop != nullptr ? prop->dynTypeName() : "";
    if (propTypeName == PropertyBool::TypeName) {
        return yesNoString(static_cast<const PropertyBool*>(prop)->value());
    }
    else if (propTypeName == PropertyInt::TypeName) {
        return locale.toString(static_cast<const PropertyInt*>(prop)->value());
    }
    else if (propTypeName == PropertyDouble::TypeName) {
        return StringUtils::text(
                    static_cast<const PropertyDouble*>(prop)->value(),
                    options->defaultTextOptions());
    }
    else if (propTypeName == PropertyQByteArray::TypeName) {
        return QString::fromUtf8(static_cast<const PropertyQByteArray*>(prop)->value());
    }
    else if (propTypeName == PropertyQString::TypeName) {
        return static_cast<const PropertyQString*>(prop)->value();
    }
    else if (propTypeName == PropertyQDateTime::TypeName) {
        return locale.toString(static_cast<const PropertyQDateTime*>(prop)->value());
    }
    else if (propTypeName == PropertyOccColor::TypeName) {
        return StringUtils::text(static_cast<const PropertyOccColor*>(prop)->value());
    }
    else if (propTypeName == PropertyOccPnt::TypeName) {
        return StringUtils::text(
                    static_cast<const PropertyOccPnt*>(prop)->value(),
                    options->defaultTextOptions());
    }
    else if (propTypeName == PropertyOccTrsf::TypeName) {
        return StringUtils::text(
                    static_cast<const PropertyOccTrsf*>(prop)->value(),
                    options->defaultTextOptions());
    }
    else if (propTypeName == PropertyEnumeration::TypeName) {
        auto propEnum = static_cast<const PropertyEnumeration*>(prop);
        const auto& enumMappings = propEnum->enumeration().mappings();
        for (const Enumeration::Mapping& mapping : enumMappings) {
            if (mapping.value == propEnum->value())
                return mapping.string;
        }
        return QString();
    }
    else if (propTypeName == BasePropertyQuantity::TypeName) {
        auto qtyProp = static_cast<const BasePropertyQuantity*>(prop);
        if (qtyProp->quantityUnit() == Unit::Time) {
            const QTime duration = QTime(0, 0).addSecs(qtyProp->quantityValue());
            return options->locale().toString(duration);
        }
        const UnitSystem::TranslateResult trRes =
                options->unitSystemTranslate(
                    qtyProp->quantityValue(), qtyProp->quantityUnit());
        return WidgetPropertiesEditor::tr("%1%2")
                .arg(StringUtils::text(trRes.value, options->defaultTextOptions()))
                .arg(trRes.strUnit);
    }
    return WidgetPropertiesEditor::tr("ERROR: no stringifier for property type '%1'")
            .arg(propTypeName);
}

static QWidget* createPropertyEditor(BasePropertyQuantity* prop, QWidget* parent)
{
    auto editor = new QDoubleSpinBox(parent);
    const UnitSystem::TranslateResult trRes =
            Options::instance()->unitSystemTranslate(
                prop->quantityValue(), prop->quantityUnit());
    editor->setSuffix(QString::fromUtf8(trRes.strUnit));
    editor->setDecimals(Options::instance()->unitSystemDecimals());
    const double rangeMin =
            prop->constraintsEnabled() ?
                prop->minimum() : std::numeric_limits<double>::min();
    const double rangeMax =
            prop->constraintsEnabled() ?
                prop->maximum() : std::numeric_limits<double>::max();
    editor->setRange(rangeMin, rangeMax);
    editor->setValue(trRes.value);
    auto signalValueChanged =
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged);
    QObject::connect(editor, signalValueChanged, [=](double value) {
        prop->setQuantityValue(value);
    });
    return editor;
}

static QWidget* createPanelEditor(QWidget* parent)
{
    auto frame = new PanelEditor(parent);
    auto layout = new QHBoxLayout(frame);
    layout->setContentsMargins(2, 0, 0, 0);
    return frame;
}

static QWidget* createPropertyEditor(PropertyBool* prop, QWidget* parent)
{
    auto frame = createPanelEditor(parent);
    auto editor = new QCheckBox(frame);
    frame->layout()->addWidget(editor);
    auto propBool = static_cast<PropertyBool*>(prop);
    editor->setText(yesNoString(propBool->value()));
    editor->setChecked(propBool->value());
    QObject::connect(editor, &QCheckBox::toggled, [=](bool on) {
        editor->setText(yesNoString(on));
        propBool->setValue(on);
    });
    return frame;
}

static QWidget* createPropertyEditor(PropertyInt* prop, QWidget* parent)
{
    auto editor = new QSpinBox(parent);
    if (prop->constraintsEnabled()) {
        editor->setRange(prop->minimum(), prop->maximum());
        editor->setSingleStep(prop->singleStep());
    }
    editor->setValue(prop->value());
    auto signalValueChanged =
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged);
    QObject::connect(editor, signalValueChanged, [=](int val) {
        prop->setValue(val);
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyQString* prop, QWidget* parent)
{
    auto editor = new QLineEdit(parent);
    editor->setText(prop->value());
    QObject::connect(editor, &QLineEdit::textChanged, [=](const QString& text) {
        prop->setValue(text);
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyEnumeration* prop, QWidget* parent)
{
    auto editor = new QComboBox(parent);
    const Enumeration& enumDef = prop->enumeration();
    for (const Enumeration::Mapping& mapping : enumDef.mappings())
        editor->addItem(mapping.string, mapping.value);
    editor->setCurrentIndex(editor->findData(prop->value()));
    auto signalActivated =
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated);
    QObject::connect(editor, signalActivated, [=](int index) {
        prop->setValue(editor->itemData(index).toInt());
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyOccColor* prop, QWidget* parent)
{
    auto frame = createPanelEditor(parent);
    auto editor = new QWidget(frame);

    auto labelColor = new QLabel(frame);
    const QColor inputColor = occ::QtUtils::toQColor(prop->value());
    labelColor->setPixmap(colorSquarePixmap(inputColor));

    auto labelRgb = new QLabel(frame);
    labelRgb->setText(propertyValueText(prop));

    auto btnColor = new QToolButton(frame);
    btnColor->setText("...");
    btnColor->setToolTip(WidgetPropertiesEditor::tr("Choose color ..."));
    QObject::connect(btnColor, &QAbstractButton::clicked, [=]{
        auto dlg = new QColorDialog(editor);
        dlg->setCurrentColor(inputColor);
        QObject::connect(dlg, &QColorDialog::colorSelected, [=](const QColor& c) {
            prop->setValue(occ::QtUtils::toOccColor(c));
            labelColor->setPixmap(colorSquarePixmap(c));
        });
        qtgui::QWidgetUtils::asyncDialogExec(dlg);
    });

    frame->layout()->addWidget(labelColor);
    frame->layout()->addWidget(labelRgb);
    frame->layout()->addWidget(btnColor);
    frame->layout()->addWidget(hSpacerWidget(editor));

    return frame;
}

class PropertyItemDelegate : public QStyledItemDelegate {
public:
    PropertyItemDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    double rowHeightFactor() const { return m_rowHeightFactor; }
    void setRowHeightFactor(double v) { m_rowHeightFactor = v; }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        if (index.column() == 1) {
            auto prop = qvariant_cast<Property*>(index.data());
            if (prop != nullptr
                    && prop->dynTypeName() == PropertyOccColor::TypeName)
            {
                auto propColor = static_cast<PropertyOccColor*>(prop);
                painter->save();

                QApplication::style()->drawPrimitive(
                            QStyle::PE_PanelItemViewItem,
                            &option,
                            painter,
                            option.widget);

                const QColor color = occ::QtUtils::toQColor(propColor->value());
                const QPixmap pixColor = colorSquarePixmap(color, option.rect.height());
                painter->drawPixmap(option.rect.x(), option.rect.y(), pixColor);
                const QString strColor = propertyValueText(propColor);

                QRect labelRect = option.rect;
                labelRect.setX(option.rect.x() + pixColor.width() + 6);
                QApplication::style()->drawItemText(
                            painter,
                            labelRect,
                            Qt::AlignLeft | Qt::AlignVCenter,
                            option.palette,
                            option.state.testFlag(QStyle::State_Enabled),
                            strColor);

                painter->restore();
                return;
            }
        }
        QStyledItemDelegate::paint(painter, option, index);
    }

    QString displayText(const QVariant& value, const QLocale&) const override
    {
        if (value.type() == QVariant::String)
            return value.toString();
        else if (value.canConvert<Property*>()) {
            const auto prop = qvariant_cast<Property*>(value);
            return propertyValueText(prop);
        }
        return QString();
    }

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem&,
            const QModelIndex& index) const override
    {
        if (index.column() == 0)
            return nullptr;
        auto prop = qvariant_cast<Property*>(index.data());
        if (prop == nullptr || prop->isUserReadOnly())
            return nullptr;
        const char* propTypeName = prop->dynTypeName();
        if (propTypeName == BasePropertyQuantity::TypeName)
            return createPropertyEditor(static_cast<BasePropertyQuantity*>(prop), parent);
        if (propTypeName == PropertyBool::TypeName)
            return createPropertyEditor(static_cast<PropertyBool*>(prop), parent);
        if (propTypeName == PropertyInt::TypeName)
            return createPropertyEditor(static_cast<PropertyInt*>(prop), parent);
        if (propTypeName == PropertyQString::TypeName)
            return createPropertyEditor(static_cast<PropertyQString*>(prop), parent);
        if (propTypeName == PropertyOccColor::TypeName)
            return createPropertyEditor(static_cast<PropertyOccColor*>(prop), parent);
        if (propTypeName == PropertyEnumeration::TypeName)
            return createPropertyEditor(static_cast<PropertyEnumeration*>(prop), parent);
        return nullptr;
    }

    void setModelData(
            QWidget*, QAbstractItemModel*, const QModelIndex&) const override
    {
        // Disable default behavior that sets item data(property is changed directly)
    }

    QSize sizeHint(
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override
    {
        const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
        return QSize(baseSize.width(), m_rowHeightFactor * baseSize.height());
    }

private:
    double m_rowHeightFactor = 1.;
};

} // namespace Internal

WidgetPropertiesEditor::WidgetPropertiesEditor(QWidget *parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetPropertiesEditor)
{
    m_ui->setupUi(this);
    m_ui->treeWidget_Browser->setUniformRowHeights(true);
    m_ui->treeWidget_Browser->setIndentation(15);
    m_itemDelegate = new Internal::PropertyItemDelegate(m_ui->treeWidget_Browser);
    m_ui->treeWidget_Browser->setItemDelegate(m_itemDelegate);

    QObject::connect(
                Options::instance(), &Options::unitSystemSchemaChanged,
                this, &WidgetPropertiesEditor::refreshAllQtProperties);
    QObject::connect(
                Options::instance(), &Options::unitSystemDecimalsChanged,
                this, &WidgetPropertiesEditor::refreshAllQtProperties);
}

WidgetPropertiesEditor::~WidgetPropertiesEditor()
{
    delete m_ui;
}

void WidgetPropertiesEditor::editProperties(Document *doc)
{
    this->releaseObjects();
    if (doc != nullptr) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        m_currentDoc = doc;
        this->refreshAllQtProperties();
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
    }
}

void WidgetPropertiesEditor::editProperties(DocumentItem *docItem)
{
    this->releaseObjects();
    if (docItem != nullptr) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        m_currentDocItem = docItem;
        const GuiDocument* guiDoc =
                GuiApplication::instance()->findGuiDocument(m_currentDocItem->document());
        m_currentGpxDocItem = guiDoc->findItemGpx(m_currentDocItem);
        this->refreshAllQtProperties();
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
    }
}

void WidgetPropertiesEditor::editProperties(Span<HandleProperty> spanHndProp)
{
    this->releaseObjects();
    for (HandleProperty& hndProp : spanHndProp)
        m_currentVecHndProperty.push_back(std::move(hndProp));
    if (!m_currentVecHndProperty.empty()) {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserDetails);
        this->refreshAllQtProperties();
    }
    else {
        m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
    }
}

void WidgetPropertiesEditor::clear()
{
    this->releaseObjects();
    m_ui->stack_Browser->setCurrentWidget(m_ui->page_BrowserEmpty);
}

void WidgetPropertiesEditor::addLineWidget(QWidget* widget)
{
    widget->setAutoFillBackground(true);
    auto treeItem = new QTreeWidgetItem(m_ui->treeWidget_Browser);
    m_ui->treeWidget_Browser->setFirstItemColumnSpanned(treeItem, true);
    m_ui->treeWidget_Browser->setItemWidget(treeItem, 0, widget);
}

double WidgetPropertiesEditor::rowHeightFactor() const
{
    return m_itemDelegate->rowHeightFactor();
}

void WidgetPropertiesEditor::setRowHeightFactor(double v)
{
    m_itemDelegate->setRowHeightFactor(v);
}

void WidgetPropertiesEditor::createQtProperties(
        const std::vector<Property*>& properties, QTreeWidgetItem* parentItem)
{
    for (Property* prop : properties)
        this->createQtProperty(prop, parentItem);
}

void WidgetPropertiesEditor::createQtProperty(
        Property* property, QTreeWidgetItem* parentItem)
{
    auto itemProp = new QTreeWidgetItem;
    itemProp->setText(0, property->label());
    itemProp->setData(1, Qt::DisplayRole, QVariant::fromValue<Property*>(property));
    itemProp->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    if (parentItem != nullptr)
        parentItem->addChild(itemProp);
    else
        m_ui->treeWidget_Browser->addTopLevelItem(itemProp);
}

void WidgetPropertiesEditor::refreshAllQtProperties()
{
    m_ui->treeWidget_Browser->clear();

    // Document
    if (m_currentDoc != nullptr) {
        this->createQtProperties(m_currentDoc->properties(), nullptr);
    }

    // Data for DocumentItem
    if (m_currentDocItem != nullptr) {
        auto itemGroupData = new QTreeWidgetItem;
        itemGroupData->setText(0, tr("Data"));
        this->createQtProperties(m_currentDocItem->properties(), itemGroupData);
        m_ui->treeWidget_Browser->addTopLevelItem(itemGroupData);
        itemGroupData->setExpanded(true);
    }

    // Graphics for DocumentItem
    if (m_currentGpxDocItem != nullptr) {
        auto itemGroupGpx = new QTreeWidgetItem;
        itemGroupGpx->setText(0, tr("Graphics"));
        const GpxDocumentItem* gpxItem = m_currentGpxDocItem;
        this->createQtProperties(gpxItem->properties(), itemGroupGpx);
        m_ui->treeWidget_Browser->addTopLevelItem(itemGroupGpx);
        itemGroupGpx->setExpanded(true);
    }

    // "On-the-fly" properties
    if (!m_currentVecHndProperty.empty()) {
        for (const HandleProperty& propHnd : m_currentVecHndProperty)
            this->createQtProperty(propHnd.get(), nullptr);
    }

    m_ui->treeWidget_Browser->resizeColumnToContents(0);
    m_ui->treeWidget_Browser->resizeColumnToContents(1);
}

void WidgetPropertiesEditor::releaseObjects()
{
    m_currentDoc = nullptr;
    m_currentDocItem = nullptr;
    m_currentGpxDocItem = nullptr;
    m_currentVecHndProperty.clear();
}

} // namespace Mayo

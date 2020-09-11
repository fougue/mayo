/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_properties_editor.h"

#include "../base/application.h"
#include "../base/document.h"
#include "../base/property_enumeration.h"
#include "../base/settings.h"
#include "../base/settings_keys.h"
#include "../base/string_utils.h"
#include "../base/unit_system.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "theme.h"
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
#include <vector>

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

static QString toStringDHMS(QuantityTime time)
{
    const double duration_s = UnitSystem::seconds(time);
    const double days = duration_s / 86400.;
    const int dayCount = std::floor(days);
    const double hours = (days - dayCount) * 24;
    const int hourCount = std::floor(hours);
    const double mins = (hours - hourCount) * 60;
    const int minCount = std::floor(mins);
    const double secs = (mins - minCount) * 60;
    const int secCount = std::floor(secs);
    QString text;
    if (dayCount > 0)
        text += WidgetPropertiesEditor::tr("%1d ").arg(dayCount);
    if (hourCount > 0)
        text += WidgetPropertiesEditor::tr("%1h ").arg(hourCount);
    if (minCount > 0)
        text += WidgetPropertiesEditor::tr("%1min ").arg(minCount);
    if (secCount > 0)
        text += WidgetPropertiesEditor::tr("%1s").arg(secCount);
    return text.trimmed();
}

static UnitSystem::TranslateResult unitTranslate(const BasePropertyQuantity* prop)
{
    if (prop->quantityUnit() == Unit::Angle) {
        auto propAngle = static_cast<const PropertyAngle*>(prop);
        return UnitSystem::degrees(propAngle->quantity());
    }

    return UnitSystem::translate(
                Application::instance()->settings()->unitSystemSchema(),
                prop->quantityValue(),
                prop->quantityUnit());
}

static QString propertyValueText(const PropertyBool* prop) {
    return yesNoString(prop->value());
}

static QString propertyValueText(const PropertyInt* prop) {
    return Application::instance()->settings()->locale().toString(prop->value());
}

static QString propertyValueText(const PropertyDouble* prop) {
    return StringUtils::text(prop->value(), Application::instance()->settings()->defaultTextOptions());
}

static QString propertyValueText(const PropertyQByteArray* prop) {
    return QString::fromUtf8(prop->value());
}

static QString propertyValueText(const PropertyQString* prop) {
    return prop->value();
}

static QString propertyValueText(const PropertyQDateTime* prop) {
    return Application::instance()->settings()->locale().toString(prop->value());
}

static QString propertyValueText(const PropertyOccColor* prop) {
    return StringUtils::text(prop->value());
}

static QString propertyValueText(const PropertyOccPnt* prop) {
    return StringUtils::text(prop->value(), Application::instance()->settings()->defaultTextOptions());
}

static QString propertyValueText(const PropertyOccTrsf* prop) {
    return StringUtils::text(prop->value(), Application::instance()->settings()->defaultTextOptions());
}

static QString propertyValueText(const PropertyEnumeration* prop)
{
    if (!prop->enumeration())
        return QString();

    for (const Enumeration::Item& enumItem : prop->enumeration()->items()) {
        if (enumItem.value == prop->value())
            return enumItem.text;
    }

    return QString();
}

static QString propertyValueText(const BasePropertyQuantity* prop)
{
    if (prop->quantityUnit() == Unit::Time) {
        auto propTime = static_cast<const PropertyTime*>(prop);
        return toStringDHMS(propTime->quantity());
    }

    const UnitSystem::TranslateResult trRes = unitTranslate(prop);
    return WidgetPropertiesEditor::tr("%1%2")
            .arg(StringUtils::text(trRes.value, Application::instance()->settings()->defaultTextOptions()))
            .arg(trRes.strUnit);
}

static QString propertyValueText(
        const BasePropertyQuantity* prop,
        const WidgetPropertiesEditor::UnitTranslation& unitTr)
{
    const double trValue = prop->quantityValue() * unitTr.factor;
    return WidgetPropertiesEditor::tr("%1%2")
            .arg(StringUtils::text(trValue, Application::instance()->settings()->defaultTextOptions()))
            .arg(unitTr.strUnit);
}

static bool handlePropertyValidationError(
        const Result<void>& resultValidation,
        QWidget* propertyEditor,
        const std::function<void()>& fnCallbackError = nullptr)
{
    if (!resultValidation) {
        qtgui::QWidgetUtils::asyncMsgBoxCritical(
                    propertyEditor->parentWidget(),
                    WidgetPropertiesEditor::tr("Error"),
                    resultValidation.errorText());
        QSignalBlocker sigBlock(propertyEditor); Q_UNUSED(sigBlock);
        if (fnCallbackError)
            fnCallbackError();
    }

    return resultValidation.valid();
}

static QWidget* createPropertyEditor(BasePropertyQuantity* prop, QWidget* parent)
{
    auto editor = new QDoubleSpinBox(parent);
    const UnitSystem::TranslateResult trRes = unitTranslate(prop);
    editor->setSuffix(QString::fromUtf8(trRes.strUnit));
    editor->setDecimals(Application::instance()->settings()->unitSystemDecimals());
    const double rangeMin =
            prop->constraintsEnabled() ?
                prop->minimum() : std::numeric_limits<double>::min();
    const double rangeMax =
            prop->constraintsEnabled() ?
                prop->maximum() : std::numeric_limits<double>::max();
    editor->setRange(rangeMin, rangeMax);
    editor->setValue(trRes.value);
    QObject::connect(editor, &QDoubleSpinBox::editingFinished, [=]{
        double value = editor->value();
        const double f = trRes.factor;
        value = qFuzzyCompare(f, 1.) ? value : value * f;
        if (!qFuzzyCompare(prop->quantityValue(), value)) {
            const Result result = prop->setQuantityValue(value);
            handlePropertyValidationError(result, editor, [=]{
                editor->setValue(trRes.value);
            });
        }
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
    editor->setText(yesNoString(prop->value()));
    editor->setChecked(prop->value());
    QObject::connect(editor, &QCheckBox::toggled, [=](bool on) {
        const Result<void> result = prop->setValue(on);
        handlePropertyValidationError(result, editor, [=]{
            editor->setChecked(prop->value());
        });
        editor->setText(yesNoString(prop->value()));
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
    QObject::connect(editor, qOverload<int>(&QSpinBox::valueChanged), [=](int val) {
        const Result<void> result = prop->setValue(val);
        handlePropertyValidationError(result, editor, [=]{
            editor->setValue(prop->value());
        });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyDouble* prop, QWidget* parent)
{
    auto editor = new QDoubleSpinBox(parent);
    if (prop->constraintsEnabled()) {
        editor->setRange(prop->minimum(), prop->maximum());
        editor->setSingleStep(prop->singleStep());
    }
    editor->setValue(prop->value());
    editor->setDecimals(Application::instance()->settings()->unitSystemDecimals());
    QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double val) {
        const Result<void> result = prop->setValue(val);
        handlePropertyValidationError(result, editor, [=]{
            editor->setValue(prop->value());
        });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyQString* prop, QWidget* parent)
{
    auto editor = new QLineEdit(parent);
    editor->setText(prop->value());
    QObject::connect(editor, &QLineEdit::textChanged, [=](const QString& text) {
        const Result<void> result = prop->setValue(text);
        handlePropertyValidationError(result, editor, [=]{
            editor->setText(prop->value());
        });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyEnumeration* prop, QWidget* parent)
{
    auto editor = new QComboBox(parent);
    const Enumeration* enumDef = prop->enumeration();
    if (!enumDef)
        return editor;

    for (const Enumeration::Item& enumItem : enumDef->items())
        editor->addItem(enumItem.text, enumItem.value);

    editor->setCurrentIndex(editor->findData(prop->value()));
    QObject::connect(editor, qOverload<int>(&QComboBox::activated), [=](int index) {
        const Result<void> result = prop->setValue(editor->itemData(index).toInt());
        handlePropertyValidationError(result, editor, [=]{
            editor->setCurrentIndex(editor->findData(prop->value()));
        });
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyOccColor* prop, QWidget* parent)
{
    auto frame = createPanelEditor(parent);

    auto labelColor = new QLabel(frame);
    const QColor inputColor = occ::QtUtils::toQColor(prop->value());
    labelColor->setPixmap(colorSquarePixmap(inputColor));

    auto labelRgb = new QLabel(frame);
    labelRgb->setText(propertyValueText(prop));

    auto btnColor = new QToolButton(frame);
    btnColor->setText("...");
    btnColor->setToolTip(WidgetPropertiesEditor::tr("Choose color ..."));
    QObject::connect(btnColor, &QAbstractButton::clicked, [=]{
        auto dlg = new QColorDialog(frame);
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
    frame->layout()->addWidget(hSpacerWidget(frame));

    return frame;
}

static QDoubleSpinBox* createOccPntCoordEditor(
        QWidget* parent,
        PropertyOccPnt* prop,
        double (gp_Pnt::*funcGetCoord)() const,
        void (gp_Pnt::*funcSetCoord)(double))
{
    auto editor = new QDoubleSpinBox(parent);
    const QuantityLength lenCoord = ((prop->value()).*funcGetCoord)() * Quantity_Millimeter;
    const UnitSystem::TranslateResult trRes =
            UnitSystem::translate(Application::instance()->settings()->unitSystemSchema(), lenCoord);
    //editor->setSuffix(QString::fromUtf8(trRes.strUnit));
    editor->setDecimals(Application::instance()->settings()->unitSystemDecimals());
    editor->setButtonSymbols(QDoubleSpinBox::NoButtons);
    editor->setRange(std::numeric_limits<double>::min(),
                     std::numeric_limits<double>::max());
    editor->setValue(trRes.value);
    QSizePolicy sp = editor->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Expanding);
    editor->setSizePolicy(sp);
    editor->setMinimumWidth(25);
    QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value) {
        const double f = trRes.factor;
        value = qFuzzyCompare(f, 1.) ? value : value * f;
        gp_Pnt pnt = prop->value();
        (pnt.*funcSetCoord)(value);
        const Result<void> result = prop->setValue(pnt);
        handlePropertyValidationError(result, editor);
    });
    return editor;
}

static QWidget* createPropertyEditor(PropertyOccPnt* prop, QWidget* parent)
{
    auto frame = createPanelEditor(parent);
    QLayout* frameLayout = frame->layout();
    frameLayout->addWidget(new QLabel("X", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::X, &gp_Pnt::SetX));
    frameLayout->addWidget(new QLabel("Y", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::Y, &gp_Pnt::SetY));
    frameLayout->addWidget(new QLabel("Z", frame));
    frameLayout->addWidget(createOccPntCoordEditor(frame, prop, &gp_Pnt::Z, &gp_Pnt::SetZ));
    return frame;
}

class PropertyItemDelegate : public QStyledItemDelegate {
public:
    PropertyItemDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    double rowHeightFactor() const { return m_rowHeightFactor; }
    void setRowHeightFactor(double v) { m_rowHeightFactor = v; }

    using UnitTranslation = WidgetPropertiesEditor::UnitTranslation;
    bool overridePropertyUnitTranslation(
            const BasePropertyQuantity* prop, UnitTranslation unitTr)
    {
        if (!prop || prop->quantityUnit() != unitTr.unit)
            return false;
        m_mapPropUnitTr.emplace(prop, unitTr);
        return true;
    }

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        if (index.column() == 1) {
            const auto prop = qvariant_cast<Property*>(index.data());
            if (prop && prop->dynTypeName() == PropertyOccColor::TypeName) {
                auto propColor = static_cast<const PropertyOccColor*>(prop);
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
        if (index.column() == 1 && !index.data().isNull()) {
            const auto prop = qvariant_cast<Property*>(index.data());
            if (!prop->isUserReadOnly()
                    && option.state.testFlag(QStyle::State_Enabled)
                    && option.state.testFlag(QStyle::State_MouseOver))
            {
                const QSize itemSize = this->sizeHint(option, index);
                const QSize pixItemSize = option.decorationSize * 0.75;
                const QPixmap pixEdit =
                        mayoTheme()->icon(Theme::Icon::Edit).pixmap(pixItemSize);
                painter->drawPixmap(
                            option.rect.x() + itemSize.width() + 4,
                            option.rect.y() + (itemSize.height() - pixItemSize.height()) / 2.,
                            pixEdit);
            }
        }
    }

    QString displayText(const QVariant& value, const QLocale&) const override
    {
        if (value.type() == QVariant::String)
            return value.toString();
        else if (value.canConvert<Property*>()) {
            const auto prop = qvariant_cast<Property*>(value);
            //return propertyValueText(prop);
            const char* propTypeName = prop ? prop->dynTypeName() : "";
            if (propTypeName == PropertyBool::TypeName)
                return propertyValueText(static_cast<const PropertyBool*>(prop));
            if (propTypeName == PropertyInt::TypeName)
                return propertyValueText(static_cast<const PropertyInt*>(prop));
            if (propTypeName == PropertyDouble::TypeName)
                return propertyValueText(static_cast<const PropertyDouble*>(prop));
            if (propTypeName == PropertyQByteArray::TypeName)
                return propertyValueText(static_cast<const PropertyQByteArray*>(prop));
            if (propTypeName == PropertyQString::TypeName)
                return propertyValueText(static_cast<const PropertyQString*>(prop));
            if (propTypeName == PropertyQDateTime::TypeName)
                return propertyValueText(static_cast<const PropertyQDateTime*>(prop));
            if (propTypeName == PropertyOccColor::TypeName)
                return propertyValueText(static_cast<const PropertyOccColor*>(prop));
            if (propTypeName == PropertyOccPnt::TypeName)
                return propertyValueText(static_cast<const PropertyOccPnt*>(prop));
            if (propTypeName == PropertyOccTrsf::TypeName)
                return propertyValueText(static_cast<const PropertyOccTrsf*>(prop));
            if (propTypeName == PropertyEnumeration::TypeName)
                return propertyValueText(static_cast<const PropertyEnumeration*>(prop));
            if (propTypeName == BasePropertyQuantity::TypeName) {
                auto qtyProp = static_cast<const BasePropertyQuantity*>(prop);
                auto itFound = m_mapPropUnitTr.find(qtyProp);
                if (itFound != m_mapPropUnitTr.cend())
                    return propertyValueText(qtyProp, itFound->second);
                else
                    return propertyValueText(qtyProp);
            }
            return WidgetPropertiesEditor::tr("ERROR no stringifier for property type '%1'")
                    .arg(propTypeName);
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
        if (!prop || prop->isUserReadOnly())
            return nullptr;
        const char* propTypeName = prop->dynTypeName();
        if (propTypeName == PropertyBool::TypeName)
            return createPropertyEditor(static_cast<PropertyBool*>(prop), parent);
        if (propTypeName == PropertyInt::TypeName)
            return createPropertyEditor(static_cast<PropertyInt*>(prop), parent);
        if (propTypeName == PropertyDouble::TypeName)
            return createPropertyEditor(static_cast<PropertyDouble*>(prop), parent);
        if (propTypeName == PropertyQString::TypeName)
            return createPropertyEditor(static_cast<PropertyQString*>(prop), parent);
        if (propTypeName == PropertyOccColor::TypeName)
            return createPropertyEditor(static_cast<PropertyOccColor*>(prop), parent);
        if (propTypeName == PropertyOccPnt::TypeName)
            return createPropertyEditor(static_cast<PropertyOccPnt*>(prop), parent);
        if (propTypeName == PropertyEnumeration::TypeName)
            return createPropertyEditor(static_cast<PropertyEnumeration*>(prop), parent);
        if (propTypeName == BasePropertyQuantity::TypeName)
            return createPropertyEditor(static_cast<BasePropertyQuantity*>(prop), parent);
        return nullptr;
    }

    void setModelData(
            QWidget*, QAbstractItemModel*, const QModelIndex&) const override
    {
        // Disable default behavior that sets item data(property is changed directly)
    }

    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override
    {
        const QSize baseSize = QStyledItemDelegate::sizeHint(option, index);
        if (index.data(Qt::SizeHintRole).isNull())
            return QSize(baseSize.width(), m_rowHeightFactor * baseSize.height());
        return baseSize;
    }

private:
    double m_rowHeightFactor = 1.;
    std::unordered_map<const BasePropertyQuantity*, UnitTranslation> m_mapPropUnitTr;
};

} // namespace Internal

struct WidgetPropertiesEditor::Group {
    QTreeWidgetItem* treeItem;
};

class WidgetPropertiesEditor::Private {
public:
    void createQtProperty(Property* property, QTreeWidgetItem* parentItem);
    QTreeWidgetItem* addLineWidgetItem(QWidget* widget, int height);
    QTreeWidgetItem* findTreeItem(const Property* property) const;
    bool hasGroup(const WidgetPropertiesEditor::Group* group) const;

    Ui_WidgetPropertiesEditor* ui = nullptr;
    Internal::PropertyItemDelegate* itemDelegate = nullptr;
    std::vector<Property*> vecProperty;
    std::vector<QWidget*> vecLineWidget;
    std::vector<WidgetPropertiesEditor::Group> vecGroup;
};

WidgetPropertiesEditor::WidgetPropertiesEditor(QWidget *parent)
    : QWidget(parent),
      d(new Private)
{
    d->ui = new Ui_WidgetPropertiesEditor;
    d->ui->setupUi(this);
    d->ui->treeWidget_Browser->setIndentation(0);
    d->ui->treeWidget_Browser->setItemsExpandable(false);
    d->ui->treeWidget_Browser->setRootIsDecorated(false);
    d->itemDelegate = new Internal::PropertyItemDelegate(d->ui->treeWidget_Browser);
    d->ui->treeWidget_Browser->setItemDelegate(d->itemDelegate);
}

WidgetPropertiesEditor::~WidgetPropertiesEditor()
{
    delete d->ui;
    delete d;
}

WidgetPropertiesEditor::Group* WidgetPropertiesEditor::addGroup(const QString& name)
{
    Group grp = {};
    grp.treeItem = new QTreeWidgetItem;
    grp.treeItem->setText(0, name);
    grp.treeItem->setFirstColumnSpanned(true);
    d->ui->treeWidget_Browser->addTopLevelItem(grp.treeItem);
    grp.treeItem->setExpanded(true);
    d->vecGroup.push_back(grp);
    return &d->vecGroup.back();
}

void WidgetPropertiesEditor::setGroupName(Group* group, const QString& name)
{
    if (d->hasGroup(group))
        group->treeItem->setText(0, name);
}

void WidgetPropertiesEditor::editProperties(PropertyOwner* propOwner, Group* grp)
{
    if (propOwner) {
        d->ui->stack_Browser->setCurrentWidget(d->ui->page_BrowserDetails);
        QTreeWidgetItem* parentTreeItem = d->hasGroup(grp) ? grp->treeItem : nullptr;
        for (Property* prop : propOwner->properties())
            d->createQtProperty(prop, parentTreeItem);
        d->ui->treeWidget_Browser->resizeColumnToContents(0);
        d->ui->treeWidget_Browser->resizeColumnToContents(1);
    }
}

void WidgetPropertiesEditor::editProperty(Property* prop, Group* grp)
{
    if (prop) {
        d->ui->stack_Browser->setCurrentWidget(d->ui->page_BrowserDetails);
        QTreeWidgetItem* parentTreeItem = d->hasGroup(grp) ? grp->treeItem : nullptr;
        d->createQtProperty(prop, parentTreeItem);
        d->ui->treeWidget_Browser->resizeColumnToContents(0);
        d->ui->treeWidget_Browser->resizeColumnToContents(1);
    }
}

void WidgetPropertiesEditor::clear()
{
    d->vecProperty.clear();
    d->vecLineWidget.clear();
    d->vecGroup.clear();
    d->ui->treeWidget_Browser->clear();
}

void WidgetPropertiesEditor::setPropertyEnabled(const Property* prop, bool on)
{
    QTreeWidgetItem* treeItem = d->findTreeItem(prop);
    if (treeItem) {
        if (on)
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsEnabled);
        else
            treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsEnabled);
    }
}

void WidgetPropertiesEditor::setPropertySelectable(const Property* prop, bool on)
{
    QTreeWidgetItem* treeItem = d->findTreeItem(prop);
    if (treeItem) {
        if (on)
            treeItem->setFlags(treeItem->flags() | Qt::ItemIsSelectable);
        else
            treeItem->setFlags(treeItem->flags() & ~Qt::ItemIsSelectable);
    }
}

void WidgetPropertiesEditor::addLineSpacer(int height)
{
    auto widget = new QWidget;
    d->addLineWidgetItem(widget, height);
}

void WidgetPropertiesEditor::addLineWidget(QWidget* widget, int height)
{
    d->addLineWidgetItem(widget, height);
}

Span<QWidget* const> WidgetPropertiesEditor::lineWidgets() const
{
    return d->vecLineWidget;
}

double WidgetPropertiesEditor::rowHeightFactor() const
{
    return d->itemDelegate->rowHeightFactor();
}

void WidgetPropertiesEditor::setRowHeightFactor(double v)
{
    d->itemDelegate->setRowHeightFactor(v);
}

bool WidgetPropertiesEditor::overridePropertyUnitTranslation(
        const BasePropertyQuantity* prop, UnitTranslation unitTr)
{
    return d->itemDelegate->overridePropertyUnitTranslation(prop, unitTr);
}

void WidgetPropertiesEditor::Private::createQtProperty(
        Property* property, QTreeWidgetItem* parentItem)
{
    this->vecProperty.push_back(property);
    auto itemProp = new QTreeWidgetItem;
    const QString labelSpacer = parentItem ? "       " : "";
    itemProp->setText(0, labelSpacer + property->label());
    itemProp->setData(1, Qt::DisplayRole, QVariant::fromValue<Property*>(property));
    itemProp->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    if (parentItem)
        parentItem->addChild(itemProp);
    else
        this->ui->treeWidget_Browser->addTopLevelItem(itemProp);
}

QTreeWidgetItem* WidgetPropertiesEditor::Private::addLineWidgetItem(QWidget* widget, int height)
{
    widget->setAutoFillBackground(true);
    auto treeItem = new QTreeWidgetItem(this->ui->treeWidget_Browser);
    treeItem->setFlags(Qt::ItemIsEnabled);
    if (height > 0) {
        treeItem->setSizeHint(0, QSize(100, height));
        treeItem->setSizeHint(1, QSize(100, height));
    }
    this->vecLineWidget.push_back(widget);
    this->ui->treeWidget_Browser->setFirstItemColumnSpanned(treeItem, true);
    this->ui->treeWidget_Browser->setItemWidget(treeItem, 0, widget);
    return treeItem;
}

QTreeWidgetItem* WidgetPropertiesEditor::Private::findTreeItem(const Property* property) const
{
    for (QTreeWidgetItemIterator it(this->ui->treeWidget_Browser); *it; ++it) {
        QTreeWidgetItem* treeItem = *it;
        const QVariant value = treeItem->data(1, Qt::DisplayRole);
        if (value.canConvert<Property*>()
                && qvariant_cast<Property*>(value) == property)
        {
            return treeItem;
        }
    }
    return nullptr;
}

bool WidgetPropertiesEditor::Private::hasGroup(const Group *group) const
{
    return group
            && group->treeItem
            && group->treeItem->treeWidget() == ui->treeWidget_Browser;
}

} // namespace Mayo

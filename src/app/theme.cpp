/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "theme.h"
#include "../base/meta_enum.h"

#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QSplitterHandle>
#include <QtWidgets/QStyleFactory>

#include <QtCore/QtDebug>

#include <unordered_map>

namespace Mayo {

namespace {

const QIcon& nullQIcon()
{
    static const QIcon null;
    return null;
}

// Provides a specific style dedicated to Mayo look and feel
// * One of the special things are the "header" combo boxes. This is the kind of QComboBox object
//   used in toolbar just below Mayo's main menubar.
//   These QComboBoxes look as "auto raised" QToolButtons and also the arrow indicator is more
//   visible
// * Height of the items in QComboBox popups are a bit enlarged
class MayoStyle : public QProxyStyle {
public:
    MayoStyle(QStyle* style)
        : QProxyStyle(style)
    {
    }

    void setQComboBoxArrowPixmap(const QPixmap& pixmap)
    {
        m_qComboBoxArrowPixmap = pixmap;
    }

    void setQSplitterHighlightColor(const QColor& c)
    {
        m_qSplitterHighlightColor = c;
    }

    void drawControl(
            ControlElement elm, const QStyleOption* opt, QPainter* painter, const QWidget* widget
        ) const override
    {
        if (elm == QStyle::CE_Splitter)
            this->drawQSplitter(elm, opt, painter, widget);
        else
            QProxyStyle::drawControl(elm, opt, painter, widget);
    }

    void drawComplexControl(
            ComplexControl ctrl, const QStyleOptionComplex* opt, QPainter* painter, const QWidget* widget
        ) const override
    {
        if (ctrl == QStyle::CC_ComboBox && hasHeaderQComboBoxMark(widget))
            this->drawHeaderQComboBox(opt, painter, widget);
        else
            QProxyStyle::drawComplexControl(ctrl, opt, painter, widget);
    }

    QSize sizeFromContents(
            ContentsType type, const QStyleOption* opt, const QSize& size, const QWidget* widget
        ) const override
    {
        QSize sizeResult = QProxyStyle::sizeFromContents(type, opt, size, widget);
        if (type == CT_ItemViewItem) {
            auto comboBox = findQComboBoxParent(widget);
            if (comboBox) {
                const double f = hasHeaderQComboBoxMark(comboBox) ? 1.5 : 1.25;
                sizeResult.setHeight(sizeResult.height() * f);
            }
        }

        return sizeResult;
    }

    void polish(QWidget* widget) override
    {
        QProxyStyle::polish(widget);
        if (qobject_cast<QSplitter*>(widget) || qobject_cast<QSplitterHandle*>(widget)) {
            widget->setMouseTracking(true);
            widget->setAttribute(Qt::WA_Hover, true);
        }
    }

    static void markAsHeaderQComboBox(QComboBox* cb)
    {
        cb->setProperty("mayo_isHeaderComboBox", true);
    }

    static bool hasHeaderQComboBoxMark(const QWidget* widget)
    {
        if (widget) {
            const QVariant v = widget->property("mayo_isHeaderComboBox");
            return !v.isNull() ? v.toBool() : false;
        }

        return false;
    }

private:
    static QComboBox* findQComboBoxParent(const QWidget* widget)
    {
        QWidget* it = widget ? widget->parentWidget() : nullptr;
        while (it) {
            auto comboBox = qobject_cast<QComboBox*>(it);
            if (comboBox)
                return comboBox;
            else
                it = it->parentWidget();
        }

        return nullptr;
    }

    void drawQSplitter(
            ControlElement elm, const QStyleOption* opt, QPainter* painter, const QWidget* widget
        ) const
    {
        QProxyStyle::drawControl(elm, opt, painter, widget);
        if (opt && opt->state.testFlag(QStyle::State_MouseOver) && m_qSplitterHighlightColor.isValid())
            painter->fillRect(opt->rect, m_qSplitterHighlightColor);
    }

    void drawHeaderQComboBox(
            const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget
        ) const
    {
        if (
            option->state.testFlag(QStyle::State_Enabled)
            && option->state.testFlag(QStyle::State_Active)
            && option->state.testFlag(QStyle::State_MouseOver)
            )
        {
            QStyleOptionToolButton optsBtn;
            optsBtn.initFrom(widget);
            optsBtn.features = QStyleOptionToolButton::None;
            optsBtn.toolButtonStyle = Qt::ToolButtonIconOnly;
            if (option->state.testFlag(QStyle::State_On))
                optsBtn.state |= QStyle::State_On;

            this->drawPrimitive(QStyle::PE_PanelButtonTool, &optsBtn, painter, widget);
        }

        const QRect arrowRect = this->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxArrow, widget);
        const QRect arrowPixmapRect(
            arrowRect.left() + arrowRect.width() / 2 - (m_qComboBoxArrowPixmap.width() / 2),
            arrowRect.top() + arrowRect.height() / 2 - (m_qComboBoxArrowPixmap.height() / 2),
            m_qComboBoxArrowPixmap.width(),
            m_qComboBoxArrowPixmap.height()
        );
        painter->drawPixmap(arrowPixmapRect, m_qComboBoxArrowPixmap);
    }

    QPixmap m_qComboBoxArrowPixmap;
    QColor m_qSplitterHighlightColor;
};

QColor alphaChanged(const QColor& c, int alpha)
{
    QColor n = c;
    n.setAlpha(alpha);
    return n;
}

QPixmap invertedPixmap(const QPixmap& pix)
{
    QImage img = pix.toImage();
    img.invertPixels();
    return QPixmap::fromImage(img);
}

QString iconFileName(Theme::Icon icn)
{
    switch (icn) {
    case Theme::Icon::AddFile: return "add-file.svg";
    case Theme::Icon::File: return "file.svg";
    case Theme::Icon::OpenFiles: return "open-files.svg";
    case Theme::Icon::Import: return "import.svg";
    case Theme::Icon::Edit: return "edit.svg";
    case Theme::Icon::Export: return "export.svg";
    case Theme::Icon::Expand: return "expand.svg";
    case Theme::Icon::Cross: return "cross.svg";
    case Theme::Icon::Grid: return "grid.svg";
    case Theme::Icon::Link: return "link.svg";
    case Theme::Icon::Back: return "back.svg";
    case Theme::Icon::Next: return "next.svg";
    case Theme::Icon::Multiple: return "multiple.svg";
    case Theme::Icon::Camera: return "camera.svg";
    case Theme::Icon::LeftSidebar: return "left-sidebar.svg";
    case Theme::Icon::BackSquare: return "back-square.svg";
    case Theme::Icon::IndicatorDown: return "indicator-down_8.png";
    case Theme::Icon::Reload: return "reload.svg";
    case Theme::Icon::Stop: return "stop.svg";
    case Theme::Icon::Gear: return "gear.svg";
    case Theme::Icon::ZoomIn: return "zoom-in.svg";
    case Theme::Icon::ZoomOut: return "zoom-out.svg";
    case Theme::Icon::ClipPlane: return "clip-plane.svg";
    case Theme::Icon::Measure: return "measure.svg";
    case Theme::Icon::View3dIso: return "view-iso.svg";
    case Theme::Icon::View3dLeft: return "view-left.svg";
    case Theme::Icon::View3dRight: return "view-right.svg";
    case Theme::Icon::View3dTop: return "view-top.svg";
    case Theme::Icon::View3dBottom: return "view-bottom.svg";
    case Theme::Icon::View3dFront: return "view-front.svg";
    case Theme::Icon::View3dBack: return "view-back.svg";
    case Theme::Icon::VisibilityMenu: return "visibility-menu.svg";
    case Theme::Icon::VisibilityShowAll: return "visibility-show-all.svg";
    case Theme::Icon::VisibilityShowSelection: return "visibility-show-selection.svg";
    case Theme::Icon::VisibilityHideSelection: return "visibility-hide-selection.svg";
    case Theme::Icon::VisibilityShowSelectionOnly: return "visibility-show-selection-only.svg";
    case Theme::Icon::TurnClockwise: return "turn-cw.svg";
    case Theme::Icon::TurnCounterClockwise: return "turn-ccw.svg";
    case Theme::Icon::ItemMesh: return "item-mesh.svg";
    case Theme::Icon::ItemXde: return "item-xde.svg";
    case Theme::Icon::XdeAssembly: return "xde-assembly.svg";
    case Theme::Icon::XdeSimpleShape: return "xde-simple-shape.svg";
    }
    return {};
}

class ThemeClassic : public Theme {
public:
    QColor color(Color role) const override
    {
        const QPalette appPalette = qApp->palette();
        switch (role) {
        case Theme::Color::Palette_Base:
            return appPalette.color(QPalette::Base);
        case Theme::Color::Palette_Window:
            return appPalette.color(QPalette::Window);
        case Theme::Color::Palette_Button:
        case Theme::Color::ButtonFlat_Background:
            return appPalette.color(QPalette::Button);
        case Theme::Color::ButtonView3d_Background: {
            return appPalette.color(QPalette::Button);
        }
        case Theme::Color::ButtonFlat_Hover:
            return appPalette.color(QPalette::Button).darker(110);
        case Theme::Color::ButtonFlat_Checked:
            return appPalette.color(QPalette::Button).darker(125);
        case Theme::Color::ButtonView3d_Hover:
        case Theme::Color::ButtonView3d_Checked:
            return QColor(65, 200, 250);
        case Theme::Color::Graphic3d_AspectFillArea:
            return QColor(128, 200, 255);
        case Theme::Color::View3d_BackgroundGradientStart:
            return QColor(128, 148, 255);
        case Theme::Color::View3d_BackgroundGradientEnd:
            return Qt::white;
        case Theme::Color::RubberBandView3d_Line:
            return QColor(65, 200, 250);
        case Theme::Color::RubberBandView3d_Fill:
            return QColor(65, 200, 250).lighter();
        case Theme::Color::MessageIndicator_InfoBackground:
            return QColor(128, 200, 255);
        case Theme::Color::MessageIndicator_InfoText:
        case Theme::Color::MessageIndicator_ErrorText:
            return appPalette.color(QPalette::WindowText);
        case Theme::Color::MessageIndicator_ErrorBackground:
            return QColor(225, 127, 127, 140);
        }
        return {};
    }

    const QIcon& icon(Icon icn) const override
    {
        auto it = m_mapIcon.find(icn);
        return it != m_mapIcon.cend() ? it->second : nullQIcon();
    }

    void setup() override
    {
        const QString icnBasePath = ":/images/themes/classic/";
        for (const Icon icn : MetaEnum::values<Theme::Icon>()) {
            const QString icnFileName = iconFileName(icn);
            m_mapIcon.emplace(icn, QIcon(QPixmap(icnBasePath + icnFileName)));
        }

        auto mayoStyle = new MayoStyle(qApp->style());
        mayoStyle->setQComboBoxArrowPixmap(QPixmap(":/images/themes/classic/indicator-down_8.png"));
        const QColor windowColor = qApp->palette().color(QPalette::Window);
        mayoStyle->setQSplitterHighlightColor(alphaChanged(windowColor.darker(120), 150));
        qApp->setStyle(mayoStyle);
    }

    void setupHeaderComboBox(QComboBox* cb) override
    {
        MayoStyle::markAsHeaderQComboBox(cb);
    }

private:
    std::unordered_map<Theme::Icon, QIcon> m_mapIcon;
};

class ThemeDark : public Theme {
public:
    QColor color(Color role) const override
    {
        const QPalette appPalette = qApp->palette();
        switch (role) {
        case Theme::Color::Palette_Base:
            return appPalette.color(QPalette::Base);
        case Theme::Color::Palette_Window:
            return appPalette.color(QPalette::Window);
        case Theme::Color::Palette_Button:
        case Theme::Color::ButtonFlat_Background:
            return appPalette.color(QPalette::Button);
        case Theme::Color::ButtonFlat_Hover:
            return appPalette.color(QPalette::Button).darker(110);
        case Theme::Color::ButtonFlat_Checked:
            return appPalette.color(QPalette::Highlight);
        case Theme::Color::ButtonView3d_Background:
            return appPalette.color(QPalette::Button).lighter(125);
        case Theme::Color::ButtonView3d_Hover:
            return appPalette.color(QPalette::Button).lighter(160);
        case Theme::Color::ButtonView3d_Checked:
            return appPalette.color(QPalette::Highlight);
        case Theme::Color::Graphic3d_AspectFillArea:
            return appPalette.color(QPalette::Highlight);
        case Theme::Color::View3d_BackgroundGradientStart:
            return QColor(100, 100, 100);
        case Theme::Color::View3d_BackgroundGradientEnd:
            return QColor(200, 200, 200);
        case Theme::Color::RubberBandView3d_Line:
            return appPalette.color(QPalette::Highlight);
        case Theme::Color::RubberBandView3d_Fill:
            return appPalette.color(QPalette::Highlight).lighter();
        case Theme::Color::MessageIndicator_InfoBackground:
            return appPalette.color(QPalette::Highlight);
        case Theme::Color::MessageIndicator_InfoText:
        case Theme::Color::MessageIndicator_ErrorText:
            return appPalette.color(QPalette::WindowText);
        case Theme::Color::MessageIndicator_ErrorBackground:
            return QColor(225, 127, 127, 140);
        }
        return {};
    }

    const QIcon& icon(Icon icn) const override
    {
        auto it = m_mapIcon.find(icn);
        return it != m_mapIcon.cend() ? it->second : nullQIcon();
    }

    void setup() override
    {
        auto fnIsNeutralIcon = [](Icon icn) {
            return icn == Icon::AddFile || icn == Icon::OpenFiles || icn == Icon::Stop;
        };
        for (Icon icn : MetaEnum::values<Theme::Icon>()) {
            const QString icnFileName = iconFileName(icn);
            const QString icnBasePath =
                    !fnIsNeutralIcon(icn) ?
                        ":/images/themes/dark/" :
                        ":/images/themes/classic/";
            QPixmap pix(icnBasePath + icnFileName);
            if (!fnIsNeutralIcon(icn)) {
                const bool invertColors = icn != Icon::XdeAssembly && icn != Icon::XdeSimpleShape;
                if (invertColors)
                    pix = invertedPixmap(pix);
            }

            m_mapIcon.emplace(icn, pix);
        }

        QPalette p = qApp->palette();
        p.setColor(QPalette::Base, QColor(50, 50, 50));   // #323232
        p.setColor(QPalette::Window, QColor(37, 37, 37)); // #252525
        p.setColor(QPalette::Button, QColor(73, 73, 73)); // #494949
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::PlaceholderText, p.color(QPalette::Text).darker());
        p.setColor(QPalette::HighlightedText, Qt::white);
        const QColor linkColor(115, 131, 191);
        p.setColor(QPalette::Link, linkColor);
        p.setColor(QPalette::LinkVisited, linkColor);

        const QColor disabledGray(40, 40, 40); // #282828
        const QColor disabledTextGray(128, 128, 128); // #808080
        p.setColor(QPalette::Disabled, QPalette::Window, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Base, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::AlternateBase, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Button, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Text, disabledTextGray);
        p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledTextGray);
        p.setColor(QPalette::Disabled, QPalette::WindowText, disabledTextGray);
        qApp->setPalette(p);

        QString css =
                R"(
                QFrame[frameShape="5"] {
                    color: gray;
                    margin-top: 2px;
                    margin-bottom: 2px;
                }
                QAbstractItemView {
                    show-decoration-selected: 1;
                    background: mayo_PaletteWindowColor;
                    selection-background-color: mayo_MenuItemSelectedColor;
                }
                QAbstractItemView::item:hover { background: mayo_MenuItemSelectedColor; }
                QMenuBar::item:selected { background: mayo_PaletteButtonColor; }
                QMenuBar::item:pressed { background: mayo_PaletteHighlightColor; }
                QMenu {
                    background: mayo_PaletteBaseColor;
                    border: 1px solid mayo_MenuBorderColor;
                }
                QMenu::item:selected { background: rgb(110,110,110); }
                QMenu::separator {
                    background: mayo_MenuBorderColor;
                    height: 1px;
                }
                QComboBox QAbstractItemView {
                    background: mayo_PaletteBaseColor;
                    border: 1px solid mayo_MenuBorderColor;
                }
                QLineEdit { background: mayo_PaletteBaseColor; }
                QTextEdit { background: mayo_PaletteBaseColor; }
                QSpinBox  { background: mayo_PaletteBaseColor; }
                QDoubleSpinBox { background: mayo_PaletteBaseColor; }
                QToolButton:checked { background: #383838; }
                QToolButton:pressed { background: #383838; }
                QGroupBox {
                    border: 1px solid #808080;
                    margin-top: 4ex;
                }
                QFileDialog { background: mayo_PaletteBaseColor; }
                QComboBox {
                    background: mayo_PaletteButtonColor;
                    combobox-popup: 0;  /* This avoids the popup to hide the combo box with Fusion style */
                }
                QComboBox:editable { background: mayo_PaletteButtonColor; }
                QComboBox:disabled { background: mayo_PaletteBaseColor_Disabled; }
                QProgressBar { background: mayo_PaletteBaseColor; }
                )";
        css.replace("mayo_MenuBorderColor", QColor{100, 100, 100}.name());
        css.replace("mayo_MenuItemSelectedColor", QColor{80, 80, 80}.name());
        css.replace("mayo_PaletteBaseColor_Disabled", p.color(QPalette::Disabled, QPalette::Base).name());
        css.replace("mayo_PaletteBaseColor", p.color(QPalette::Base).name());
        css.replace("mayo_PaletteWindowColor", p.color(QPalette::Window).name());
        css.replace("mayo_PaletteButtonColor", p.color(QPalette::Button).name());
        css.replace("mayo_PaletteHighlightColor", p.color(QPalette::Highlight).name());
        qApp->setStyleSheet(css);

        // Set style
        auto mayoStyle = new MayoStyle(QStyleFactory::create("Fusion"));
        mayoStyle->setQComboBoxArrowPixmap(QPixmap(":/images/themes/dark/indicator-down_8.png"));
        mayoStyle->setQSplitterHighlightColor(alphaChanged(p.color(QPalette::Button), 150));
        qApp->setStyle(mayoStyle);
        qApp->setEffectEnabled(Qt::UI_AnimateCombo, false);
    }

    void setupHeaderComboBox(QComboBox* cb) override
    {
        MayoStyle::markAsHeaderQComboBox(cb);
    }

private:
    std::unordered_map<Theme::Icon, QIcon> m_mapIcon;
};

} // namespace

Theme* createTheme(const QString& key)
{
    if (key == "classic")
        return new ThemeClassic;

    if (key == "dark")
        return new ThemeDark;

    return nullptr;
}

} // namespace Mayo

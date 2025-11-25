/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "theme.h"
#include "../base/meta_enum.h"

#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStyleFactory>
#include <unordered_map>

namespace Mayo {

namespace Internal {

static const QIcon& nullQIcon()
{
    static const QIcon null;
    return null;
}

static QString cssFlatComboBox(
        const QString& urlPixDownArrow,
        const QString& urlPixDownArrowDisabled)
{
    const QPalette appPalette = qApp->palette();
    const QString css = QString(
                R"(
                QComboBox {
                    border-style: solid;
                    background: %1;
                    padding: 2px 15px 2px 10px;
                }
                QComboBox:hover {
                    border-style: solid;
                    background: %2;
                    padding: 2px 15px 2px 10px;
                }
                QComboBox::drop-down {
                    subcontrol-origin: padding;
                    subcontrol-position: top right;
                    width: 15px;
                    border-left-width: 0px;
                    border-top-right-radius: 3px;
                    border-bottom-right-radius: 3px;
                }
                QComboBox::down-arrow { image: url(%3); }
                QComboBox::down-arrow:disabled { image: url(%4); }
                )"
                ).arg(appPalette.color(QPalette::Window).name(),
                      appPalette.color(QPalette::Window).darker(110).name(),
                      urlPixDownArrow,
                      urlPixDownArrowDisabled);
    return css;
}

static QPixmap invertedPixmap(const QPixmap& pix)
{
    QImage img = pix.toImage();
    img.invertPixels();
    return QPixmap::fromImage(img);
}

static QString iconFileName(Theme::Icon icn)
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
    return QString();
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
        return QColor();
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
    }

    void setupHeaderComboBox(QComboBox* cb) override
    {
        const QString urlDown(":/images/themes/classic/indicator-down_8.png");
        const QString urlDownDisabled(":/images/themes/classic/indicator-down-disabled_8.png");
        cb->setStyleSheet(cssFlatComboBox(urlDown, urlDownDisabled));
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
        return QColor();
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

        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette p = qApp->palette();
        p.setColor(QPalette::Base, QColor(80, 80, 80));
        p.setColor(QPalette::Window, QColor(37, 37, 37));
        p.setColor(QPalette::Button, QColor(73, 73, 73));
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::HighlightedText, Qt::white);
        const QColor linkColor(115, 131, 191);
        p.setColor(QPalette::Link, linkColor);
        p.setColor(QPalette::LinkVisited, linkColor);

        const QColor disabledGray(40, 40, 40);
        const QColor disabledTextGray(128, 128, 128);
        p.setColor(QPalette::Disabled, QPalette::Window, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Base, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::AlternateBase, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Button, disabledGray);
        p.setColor(QPalette::Disabled, QPalette::Text, disabledTextGray);
        p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledTextGray);
        p.setColor(QPalette::Disabled, QPalette::WindowText, disabledTextGray);
        qApp->setPalette(p);

        const QString css =
                R"(
                QFrame[frameShape="5"] {
                    color: gray;
                    margin-top: 2px;
                    margin-bottom: 2px;
                }
                QAbstractItemView {
                    show-decoration-selected: 1;
                    background: #252525;
                    selection-background-color: #505050;
                }
                QAbstractItemView::item:hover { background: #383838; }
                QMenu {
                    background: #252525;
                    border: 1px solid rgb(100,100,100);
                }
                QMenu::item:selected { background: rgb(110,110,110); }
                QMenu::separator {
                    background: rgb(110,110,110);
                    height: 1px;
                }
                QLineEdit { background: #505050; }
                QTextEdit { background: #505050; }
                QSpinBox  { background: #505050; }
                QDoubleSpinBox { background: #505050; }
                QToolButton:checked { background: #383838; }
                QToolButton:pressed { background: #383838; }
                QComboBox { background: #505050; }
                QGroupBox {
                    border: 1px solid #808080;
                    margin-top: 4ex;
                }
                QFileDialog { background: #505050; }
                QComboBox:editable { background: #505050; }
                QComboBox:disabled { background: rgb(40,40,40); }
                QProgressBar { background: #505050; }
                )";
        qApp->setStyleSheet(css);
    }

    void setupHeaderComboBox(QComboBox* cb) override
    {
        const QString urlDown(":/images/themes/dark/indicator-down_8.png");
        const QString urlDownDisabled(":/images/themes/classic/indicator-down-disabled_8.png");
        cb->setStyleSheet(cssFlatComboBox(urlDown, urlDownDisabled));
    }

private:
    std::unordered_map<Theme::Icon, QIcon> m_mapIcon;
};

} // namespace Internal

Theme* createTheme(const QString& key)
{
    if (key == "classic")
        return new Internal::ThemeClassic;

    if (key == "dark")
        return new Internal::ThemeDark;

    return nullptr;
}

} // namespace Mayo

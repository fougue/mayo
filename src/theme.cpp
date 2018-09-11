/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "theme.h"

#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStyleFactory>

namespace Mayo {

namespace Internal {

static QString cssFlatComboBox(
        const QString& urlPixDownArrow,
        const QString& urlPixDownArrowDisabled)
{
    const QPalette appPalette = qApp->palette();
    const QString css = QString(
                "QComboBox {"
                "    border-style: solid;"
                "    background: %1;"
                "    padding: 2px 15px 2px 10px;"
                "}\n"
                "QComboBox:hover {"
                "    border-style: solid;"
                "    background: %2;"
                "    padding: 2px 15px 2px 10px;"
                "}\n"
                "QComboBox::drop-down {"
                "    subcontrol-origin: padding;"
                "    subcontrol-position: top right;"
                "    width: 15px;"
                "    border-left-width: 0px;"
                "    border-top-right-radius: 3px;"
                "    border-bottom-right-radius: 3px;"
                "}\n"
                "QComboBox::down-arrow { image: url(%3); }\n"
                "QComboBox::down-arrow:disabled { image: url(%4); }\n"
                ).arg(appPalette.color(QPalette::Window).name(),
                      appPalette.color(QPalette::Window).darker(110).name(),
                      urlPixDownArrow,
                      urlPixDownArrowDisabled);
    return css;
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
        case Theme::Color::ButtonView3d_Background:
            return appPalette.color(QPalette::Button);
        case Theme::Color::ButtonFlat_Hover:
            return appPalette.color(QPalette::Button).darker(110);
        case Theme::Color::ButtonFlat_Checked:
            return appPalette.color(QPalette::Button).darker(125);
        case Theme::Color::ButtonView3d_Hover:
        case Theme::Color::ButtonView3d_Checked:
            return QColor(65, 200, 250);
        case Theme::Color::View3d_BackgroundGradientStart:
            return QColor(128, 148, 255);
        case Theme::Color::View3d_BackgroundGradientEnd:
            return Qt::white;
        case Theme::Color::MessageIndicator_Background:
            return QColor(128, 200, 255);
        case Theme::Color::MessageIndicator_Text:
            return appPalette.color(QPalette::WindowText);
        }
        return QColor();
    }

    static QPixmap pixmap(Icon icn)
    {
        switch (icn) {
        case Icon::Expand: return QPixmap(":/images/themes/classic/expand_16.png");
        case Icon::Cross: return QPixmap(":/images/themes/classic/cross_32.png");
        case Icon::Link: return QPixmap(":/images/themes/classic/link-button_16.png");
        case Icon::Pin: return QPixmap(":/images/themes/classic/pin_16.png");
        case Icon::Back: return QPixmap(":/images/themes/classic/back_32.png");
        case Icon::Next: return QPixmap(":/images/themes/classic/next_32.png");
        case Icon::Camera: return QPixmap(":/images/themes/classic/camera_32.png");
        case Icon::LeftSidebar: return QPixmap(":/images/themes/classic/left-sidebar_32.png");
        case Icon::LeftArrowCross: return QPixmap(":/images/themes/classic/left-arrow-cross_16.png");
        case Icon::IndicatorDown: return QPixmap(":/images/themes/classic/indicator-down_8.png");
        case Icon::Stop: return QPixmap(":/images/no.png");
        case Icon::ItemMesh: return QPixmap(":/images/themes/classic/mesh_16.png");
        }
        return QPixmap();
    }

    template<Theme::Icon THEME_ICON>
    static const QIcon& lazyIcon()
    {
        static QIcon icn;
        if (icn.isNull())
            icn = QIcon(ThemeClassic::pixmap(THEME_ICON));
        return icn;
    }

    const QIcon& icon(Icon icn) const override
    {
        static const QIcon nullIcn;
        switch (icn) {
        case Icon::Expand: return lazyIcon<Icon::Expand>();
        case Icon::Cross: return lazyIcon<Icon::Cross>();
        case Icon::Link: return lazyIcon<Icon::Link>();
        case Icon::Pin: return lazyIcon<Icon::Pin>();
        case Icon::Back: return lazyIcon<Icon::Back>();
        case Icon::Next: return lazyIcon<Icon::Next>();
        case Icon::Camera: return lazyIcon<Icon::Camera>();
        case Icon::LeftSidebar: return lazyIcon<Icon::LeftSidebar>();
        case Icon::LeftArrowCross: return lazyIcon<Icon::LeftArrowCross>();
        case Icon::IndicatorDown: return lazyIcon<Icon::IndicatorDown>();
        case Icon::Stop: return lazyIcon<Icon::Stop>();
        case Icon::ItemMesh: return lazyIcon<Icon::ItemMesh>();
        }
        return nullIcn;
    }

    void setup() override
    {
        //    m_ui->centralWidget->setStyleSheet(
        //                "QSplitter::handle:vertical { width: 2px; }\n"
        //                "QSplitter::handle:horizontal { width: 2px; }\n");
    }

    void setupHeaderComboBox(QComboBox* cb)
    {
        const QString urlDown(":/images/themes/classic/indicator-down_8.png");
        const QString urlDownDisabled(":/images/themes/classic/indicator-down-disabled_8.png");
        cb->setStyleSheet(cssFlatComboBox(urlDown, urlDownDisabled));
    }
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
            return appPalette.color(QPalette::Button).darker(125);
        case Theme::Color::ButtonView3d_Background:
            return appPalette.color(QPalette::Button).lighter(125);
        case Theme::Color::ButtonView3d_Hover:
        case Theme::Color::ButtonView3d_Checked:
            return appPalette.color(QPalette::Button).lighter(160);
        case Theme::Color::View3d_BackgroundGradientStart:
            return Qt::lightGray;
        case Theme::Color::View3d_BackgroundGradientEnd:
            return Qt::white;
        case Theme::Color::MessageIndicator_Background:
            return appPalette.color(QPalette::Highlight).lighter(125);
        case Theme::Color::MessageIndicator_Text:
            return appPalette.color(QPalette::WindowText);
        }
        return QColor();
    }

    template<Theme::Icon THEME_ICON>
    static const QIcon& invertedClassicIcon()
    {
        static QIcon icn;
        if (icn.isNull()) {
            QImage img = ThemeClassic::pixmap(THEME_ICON).toImage();
            img.invertPixels();
            icn = QIcon(QPixmap::fromImage(img));
        }
        return icn;
    }

    const QIcon& icon(Icon icn) const override
    {
        switch (icn) {
        case Icon::Expand: return invertedClassicIcon<Icon::Expand>();
        case Icon::Cross: return invertedClassicIcon<Icon::Cross>();
        case Icon::Link: return invertedClassicIcon<Icon::Link>();
        case Icon::Pin: return invertedClassicIcon<Icon::Pin>();
        case Icon::Back: return invertedClassicIcon<Icon::Back>();
        case Icon::Next: return invertedClassicIcon<Icon::Next>();
        case Icon::Camera: return invertedClassicIcon<Icon::Camera>();
        case Icon::LeftSidebar: return invertedClassicIcon<Icon::LeftSidebar>();
        case Icon::LeftArrowCross: return invertedClassicIcon<Icon::LeftArrowCross>();
        case Icon::IndicatorDown: return invertedClassicIcon<Icon::IndicatorDown>();
        case Icon::Stop: {
            static const QIcon icn(":/images/themes/dark/stop_32.png");
            return icn;
        }
        case Icon::ItemMesh: return invertedClassicIcon<Icon::ItemMesh>();
        } // endswitch()
        static const QIcon nullIcn;
        return nullIcn;
    }

    void setup() override
    {
        qApp->setStyle(QStyleFactory::create("Fusion"));
        QPalette p = qApp->palette();
        p.setColor(QPalette::Base, QColor(80, 80, 80));
        p.setColor(QPalette::Window, QColor(53, 53, 53));
        p.setColor(QPalette::Button, QColor(73, 73, 73));
        p.setColor(QPalette::Highlight, QColor(110, 110, 110));
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::WindowText, Qt::white);
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
                "QFrame[frameShape=\"5\"] { color: gray; margin-top: 2px; margin-bottom: 2px; } "
                "QAbstractItemView { background: #505050; } "
                "QAbstractItemView::item:hover { background: #606060; }"
                "QLineEdit { background: #505050; }"
                "QMenu { background: #505050; border: 1px solid rgb(100,100,100); }"
                "QMenu::item:selected { background: rgb(110,110,110); }"
                "QTextEdit { background: #505050; }"
                "QSpinBox  { background: #505050; }"
                "QDoubleSpinBox { background: #505050; }"
                "QToolButton:checked { background: #383838; }"
                "QToolButton:pressed { background: #383838; }"
                "QComboBox { background: #505050; } "
                "QGroupBox { border: 1px solid #808080; margin-top: 4ex; } "
                "QFileDialog { background: #505050; } "
                "QComboBox:editable { background: #505050; } "
                "QComboBox::disabled { background: rgb(40,40,40); } "
                "QProgressBar { background: #505050; }";
        qApp->setStyleSheet(css);
    }

    void setupHeaderComboBox(QComboBox* cb)
    {
        const QString urlDown(":/images/themes/dark/indicator-down_8.png");
        const QString urlDownDisabled(":/images/themes/classic/indicator-down-disabled_8.png");
        cb->setStyleSheet(cssFlatComboBox(urlDown, urlDownDisabled));
    }
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

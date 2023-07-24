/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_help.h"

#include "app_module.h"
#include "qstring_conv.h"
#include "qtwidgets_utils.h"
#include "../base/meta_enum.h"
#include "../base/filepath.h"
#include "../base/io_system.h"

#include <QtCore/QDir>
#include <QtCore/QFileSelector>
#include <QtCore/QLibraryInfo>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>
#include <QtGui/QScreen>
#include <QtGui/QStyleHints>
#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyleFactory>

// NOTICE for Linux/X11
//     Because of #define conflicts, OpenGL_Context.hxx must be included *before* QtGui/QOpenGLContext
//     It also has to be included *after* QtCore/QTextStream
//     Beware of these limitations when adding/removing inclusion of headers here
#include <OpenGl_Context.hxx>
#include <Standard_Version.hxx>

#include <QtGui/QOpenGLContext>
#include <QtGui/QWindow>

#ifdef HAVE_GMIO
#  include <gmio_core/version.h>
#endif

#include <thread>

namespace Mayo {

CommandSystemInformation::CommandSystemInformation(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("System Information..."));
    this->setAction(action);
}

void CommandSystemInformation::execute()
{
    const QString text = CommandSystemInformation::data();

    auto dlg = new QDialog(this->widgetMain());
    dlg->setWindowTitle(this->action()->text());
    dlg->resize(800 * dlg->devicePixelRatioF(), 600 * dlg->devicePixelRatioF());

    auto textEdit = new QPlainTextEdit(dlg);
    textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textEdit->setReadOnly(true);
    textEdit->setPlainText(text);

    auto btnBox = new QDialogButtonBox(dlg);
    btnBox->addButton(QDialogButtonBox::Close);
    auto btnCopy = btnBox->addButton(Command::tr("Copy to Clipboard"), QDialogButtonBox::ActionRole);
    QObject::connect(btnCopy, &QAbstractButton::clicked, this, [=]{
        QGuiApplication::clipboard()->setText(text);
    });
    QObject::connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    auto layout = new QVBoxLayout(dlg);
    layout->addWidget(textEdit);
    layout->addWidget(btnBox);

    QtWidgetsUtils::asyncDialogExec(dlg);
}

namespace {

QTextStream& operator<<(QTextStream& str, bool v)
{
    str << (v ? "true" : "false");
    return str;
}

QTextStream& operator<<(QTextStream& str, const QFont& f)
{
    str << '"' << f.family() << "\" "  << f.pointSize();
    return str;
}

QTextStream& operator<<(QTextStream& str, const QSize& sz)
{
    str << sz.width() << 'x' << sz.height();
    return str;
}

QTextStream& operator<<(QTextStream& str, const QSizeF& sz)
{
    str << sz.width() << 'x' << sz.height();
    return str;
}

QTextStream& operator<<(QTextStream& str, const QRect& r)
{
    str << '(' << r.x() << ", " << r.y() << ", " << r.size() << ")";
    return str;
}

QTextStream& operator<<(QTextStream& str, std::string_view sv)
{
    str << to_QString(sv);
    return str;
}

const char indent[] = "    ";
const char indentx2[] = "        ";

} // namespace

static void dumpOpenGlInfo(QTextStream& str)
{
    QOpenGLContext qtContext;
    if (!qtContext.create()) {
        str << "Unable to create QOpenGLContext object" << '\n';
        return;
    }

    QWindow window;
    window.setSurfaceType(QSurface::OpenGLSurface);
    window.create();
    qtContext.makeCurrent(&window);

    OpenGl_Context occContext;
    if (!occContext.Init()) {
        str << "Unable to initialize OpenGl_Context object" << '\n';
        return;
    }

    TColStd_IndexedDataMapOfStringString dict;
    occContext.DiagnosticInformation(dict, Graphic3d_DiagnosticInfo_Basic);
    for (TColStd_IndexedDataMapOfStringString::Iterator it(dict); it.More(); it.Next())
        str << indent << to_QString(it.Key()) << ": " << to_QString(it.Value()) << '\n';

    str << indent << "MaxDegreeOfAnisotropy: " << occContext.MaxDegreeOfAnisotropy() << '\n'
        << indent << "MaxDrawBuffers: " << occContext.MaxDrawBuffers() << '\n'
        << indent << "MaxClipPlanes: " << occContext.MaxClipPlanes() << '\n'
        << indent << "HasRayTracing: " << occContext.HasRayTracing() << '\n'
        << indent << "HasRayTracingTextures: " << occContext.HasRayTracingTextures() << '\n'
        << indent << "HasRayTracingAdaptiveSampling: " << occContext.HasRayTracingAdaptiveSampling() << '\n'
        << indent << "UseVBO: " << occContext.ToUseVbo() << '\n';

#if OCC_VERSION_HEX >= 0x070400
    str << indent << "MaxDumpSizeX: " << occContext.MaxDumpSizeX() << '\n'
        << indent << "MaxDumpSizeY: " << occContext.MaxDumpSizeY() << '\n'
        << indent << "HasRayTracingAdaptiveSamplingAtomic: " << occContext.HasRayTracingAdaptiveSamplingAtomic() << '\n';
#endif

#if OCC_VERSION_HEX >= 0x070500
    str << indent << "HasTextureBaseLevel: " << occContext.HasTextureBaseLevel() << '\n'
        << indent << "HasSRGB: " << occContext.HasSRGB() << '\n'
        << indent << "RenderSRGB: " << occContext.ToRenderSRGB() << '\n'
        << indent << "IsWindowSRGB: " << occContext.IsWindowSRGB() << '\n'
        << indent << "HasPBR: " << occContext.HasPBR() << '\n';
#endif

#if OCC_VERSION_HEX >= 0x070700
    str << indent << "GraphicsLibrary: " << MetaEnum::name(occContext.GraphicsLibrary()) << '\n'
        << indent << "HasTextureMultisampling: " << occContext.HasTextureMultisampling() << '\n';
#endif
}

QString CommandSystemInformation::data()
{
    // Helper function returning unicode representation of a QChar object in the form "U+NNNN"
    auto fnStrUnicodeChar = [](const QChar& ch) {
        QString str;
        QTextStream ostr(&str);
        ostr << "U+" << qSetFieldWidth(4) << qSetPadChar('0') << Qt::uppercasedigits << Qt::hex << ch.unicode();
        return str;
    };

    QString strSysInfo;
    QTextStream ostr(&strSysInfo);

    // OS version
    ostr << '\n' << "OS: " << QSysInfo::prettyProductName()
         << " [" << QSysInfo::kernelType() << " version " << QSysInfo::kernelVersion() << "]" << '\n'
         << "Current CPU Architecture: " << QSysInfo::currentCpuArchitecture() << '\n';

    // Qt version
    ostr << '\n' << QLibraryInfo::build() << " on \"" << QGuiApplication::platformName() << "\" " << '\n'
         << indent << "QStyle keys: " << QStyleFactory::keys().join("; ") << '\n'
         << indent << "Image formats(read): " << QImageReader::supportedImageFormats().join(' ') << '\n'
         << indent << "Image formats(write): " << QImageWriter::supportedImageFormats().join(' ') << '\n';

    // OpenCascade version
    ostr << '\n' << "OpenCascade(build): " << OCC_VERSION_STRING_EXT << '\n';

// gmio version
#ifdef HAVE_GMIO
    ostr << '\n' << "gmio(build): " << GMIO_VERSION_STR << '\n';
#endif

    // I/O supported formats
    {
        ostr << '\n' << "Import(read) formats:" << '\n' << indent;
        const IO::System* ioSystem = AppModule::get()->ioSystem();
        for (IO::Format format : ioSystem->readerFormats())
            ostr << IO::formatIdentifier(format) << " ";

        ostr << '\n' << "Export(write) formats:" << '\n' << indent;
        for (IO::Format format : ioSystem->writerFormats())
            ostr << IO::formatIdentifier(format) << " ";

        ostr << '\n';
    }

    // Environment
    ostr << '\n' << "Environment:\n";
    {
        const QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
        for (const QString& key : sysEnv.keys())
            ostr << indent << key << "=\"" << sysEnv.value(key) << "\"\n";
    }

    // Locales
    ostr << '\n' << "Locale:\n";
    {
        const std::locale& stdLoc = AppModule::get()->stdLocale();
        const auto& numFacet = std::use_facet<std::numpunct<wchar_t>>(stdLoc);
        const QChar charDecPnt(numFacet.decimal_point());
        const QChar char1000Sep(numFacet.thousands_sep());
        ostr << indent << "std::locale:" << '\n'
             << indentx2 << "name: " << to_QString(stdLoc.name()) << '\n'
             << indentx2 << "numpunct.decimal_point: " << charDecPnt << "  " << fnStrUnicodeChar(charDecPnt) << '\n'
             << indentx2 << "numpunct.thousands_sep: " << char1000Sep << "  " << fnStrUnicodeChar(char1000Sep) << '\n'
             << indentx2 << "numpunct.grouping: " << to_QString(numFacet.grouping()) << '\n'
             << indentx2 << "numpunct.truename: " << to_QString(numFacet.truename()) << '\n'
             << indentx2 << "numpunct.falsename: " << to_QString(numFacet.falsename()) << '\n';
        const QLocale& qtLoc = AppModule::get()->qtLocale();
        ostr << indent << "QLocale:" << '\n'
             << indentx2 << "name: " << qtLoc.name() << '\n'
             << indentx2 << "language: " << MetaEnum::name(qtLoc.language()) << '\n'
             << indentx2 << "measurementSytem: " << MetaEnum::name(qtLoc.measurementSystem()) << '\n'
             << indentx2 << "textDirection: " << MetaEnum::name(qtLoc.textDirection()) << '\n'
             << indentx2 << "decimalPoint: " << qtLoc.decimalPoint() << "  " << fnStrUnicodeChar(qtLoc.decimalPoint()) << '\n'
             << indentx2 << "groupSeparator: " << qtLoc.groupSeparator() << "  " << fnStrUnicodeChar(qtLoc.groupSeparator()) << '\n';
    }

    // C++ StdLib
    ostr << '\n' << "C++ StdLib:\n";
    {
        const QChar dirSeparator(FilePath::preferred_separator);
        ostr << indent << "std::thread::hardware_concurrency: " << std::thread::hardware_concurrency() << '\n'
             << indent << "std::filepath::path::preferred_separator: " << dirSeparator << "  " << fnStrUnicodeChar(dirSeparator) << '\n';
    }

    // Library info
    ostr << '\n' << "Library info:\n";
    {
        auto fnLibInfo = [&](QLibraryInfo::LibraryLocation loc) {
            const QString locDir = QDir::toNativeSeparators(QLibraryInfo::QLibraryInfo::location(loc));
            ostr << indent << MetaEnum::name(loc) << ": " << locDir << '\n';
        };
        fnLibInfo(QLibraryInfo::PrefixPath);
        fnLibInfo(QLibraryInfo::HeadersPath);
        fnLibInfo(QLibraryInfo::LibrariesPath);
        fnLibInfo(QLibraryInfo::LibraryExecutablesPath);
        fnLibInfo(QLibraryInfo::BinariesPath);
        fnLibInfo(QLibraryInfo::PluginsPath);
        fnLibInfo(QLibraryInfo::ArchDataPath);
        fnLibInfo(QLibraryInfo::DataPath);
        fnLibInfo(QLibraryInfo::TranslationsPath);
        fnLibInfo(QLibraryInfo::SettingsPath);
    }

    // Standard paths
    ostr << '\n' << "Standard Paths:\n";
    {
        auto fnStdPath = [&](QStandardPaths::StandardLocation loc) {
            QStringList locValues = QStandardPaths::standardLocations(loc);
            for (QString& dir : locValues)
                dir = QDir::toNativeSeparators(dir);

            ostr << indent << MetaEnum::name(loc) << ": " << locValues.join("; ") << '\n';
        };
        fnStdPath(QStandardPaths::DocumentsLocation);
        fnStdPath(QStandardPaths::FontsLocation);
        fnStdPath(QStandardPaths::ApplicationsLocation);
        fnStdPath(QStandardPaths::TempLocation);
        fnStdPath(QStandardPaths::HomeLocation);
        fnStdPath(QStandardPaths::CacheLocation);
        fnStdPath(QStandardPaths::GenericCacheLocation);
        fnStdPath(QStandardPaths::GenericDataLocation);
        fnStdPath(QStandardPaths::RuntimeLocation);
        fnStdPath(QStandardPaths::ConfigLocation);
        fnStdPath(QStandardPaths::GenericConfigLocation);
        fnStdPath(QStandardPaths::AppDataLocation);
        fnStdPath(QStandardPaths::AppLocalDataLocation);
        fnStdPath(QStandardPaths::AppConfigLocation);
    }

    // File selectors
    ostr << '\n' << "File selectors(increasing order of precedence):\n" << indent;
    for (const QString& selector : QFileSelector().allSelectors())
        ostr << selector << " ";

    ostr << '\n';

    // QGuiApplication
    ostr << '\n' << "QGuiApplication:\n";
    ostr << indent << "platformName: " << QGuiApplication::platformName() << '\n'
         << indent << "desktopFileName: " << QGuiApplication::desktopFileName() << '\n'
         << indent << "desktopSettingsAware: " << QGuiApplication::desktopSettingsAware() << '\n'
         << indent << "layoutDirection: " << MetaEnum::name(QGuiApplication::layoutDirection()) << '\n';
    const QStyleHints* sh = QGuiApplication::styleHints();
    if (sh) {
        const QChar pwdChar = sh->passwordMaskCharacter();
        ostr << indent << "styleHints:\n"
             << indentx2 << "keyboardAutoRepeatRate: " << sh->keyboardAutoRepeatRate() << '\n'
             << indentx2 << "keyboardInputInterval: " << sh->keyboardInputInterval() << '\n'
             << indentx2 << "mouseDoubleClickInterval: " << sh->mouseDoubleClickInterval() << '\n'
             << indentx2 << "mousePressAndHoldInterval: " << sh->mousePressAndHoldInterval() << '\n'
             << indentx2 << "passwordMaskCharacter: " << pwdChar << "  " << fnStrUnicodeChar(pwdChar) << '\n'
             << indentx2 << "passwordMaskDelay: " << sh->passwordMaskDelay() << '\n'
             << indentx2 << "setFocusOnTouchRelease: " << sh->setFocusOnTouchRelease() << '\n'
             << indentx2 << "showIsFullScreen: " << sh->showIsFullScreen() << '\n'
             << indentx2 << "showIsMaximized: " << sh->showIsMaximized() << '\n'
             << indentx2 << "showShortcutsInContextMenus: " << sh->showShortcutsInContextMenus() << '\n'
             << indentx2 << "startDragDistance: " << sh->startDragDistance() << '\n'
             << indentx2 << "startDragTime: " << sh->startDragTime() << '\n'
             << indentx2 << "startDragVelocity: " << sh->startDragVelocity() << '\n'
             << indentx2 << "useRtlExtensions: " << sh->useRtlExtensions() << '\n'
             << indentx2 << "tabFocusBehavior: " << MetaEnum::name(sh->tabFocusBehavior()) << '\n'
             << indentx2 << "singleClickActivation: " << sh->singleClickActivation() << '\n'
             << indentx2 << "useHoverEffects: " << sh->useHoverEffects() << '\n'
             << indentx2 << "wheelScrollLines: " << sh->wheelScrollLines() << '\n'
             << indentx2 << "mouseQuickSelectionThreshold: " << sh->mouseQuickSelectionThreshold() << '\n'
             << indentx2 << "mouseDoubleClickDistance: " << sh->mouseDoubleClickDistance() << '\n'
             << indentx2 << "mouseQuickSelectionThreshold: " << sh->mouseQuickSelectionThreshold() << '\n'
             << indentx2 << "touchDoubleTapDistance: " << sh->touchDoubleTapDistance() << '\n';
    }

    // Fonts
    ostr << '\n' << "Fonts:\n";
    ostr << indent << "General font: " << QFontDatabase::systemFont(QFontDatabase::GeneralFont) << '\n'
         << indent << "Fixed font: " << QFontDatabase::systemFont(QFontDatabase::FixedFont) << '\n'
         << indent << "Title font: " << QFontDatabase::systemFont(QFontDatabase::TitleFont) << '\n'
         << indent << "Smallest font: " << QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont) << '\n';

    // Screens
    ostr << '\n' << "Screens:\n";
    {
        const QList<QScreen*> listScreen = QGuiApplication::screens();
        ostr << indent << "Count: " << listScreen.size() << '\n';
        for (int i = 0; i < listScreen.size(); ++i) {
            const QScreen* screen = listScreen.at(i);
            ostr << indent << "Screen #" << i << '\n';
            ostr << indentx2 << "name: " << screen->name() << '\n'
                 << indentx2 << "model: " << screen->model() << '\n'
                 << indentx2 << "manufacturer: " << screen->manufacturer() << '\n'
                 << indentx2 << "serialNumber: " << screen->serialNumber() << '\n'
                 << indentx2 << "depth: " << screen->depth() << '\n'
                 << indentx2 << "size: " << screen->size() << '\n'
                 << indentx2 << "availableSize: " << screen->availableSize() << '\n'
                 << indentx2 << "virtualSize: " << screen->virtualSize() << '\n'
                 << indentx2 << "availableVirtualSize: " << screen->availableVirtualSize() << '\n'
                 << indentx2 << "geometry: " << screen->geometry() << '\n'
                 << indentx2 << "availableGeometry: " << screen->availableGeometry() << '\n'
                 << indentx2 << "virtualGeometry: " << screen->virtualGeometry() << '\n'
                 << indentx2 << "availableVirtualGeometry: " << screen->availableVirtualGeometry() << '\n'
                 << indentx2 << "physicalSize: " << screen->physicalSize() << '\n'
                 << indentx2 << "physicalDotsPerInchX: " << screen->physicalDotsPerInchX() << '\n'
                 << indentx2 << "physicalDotsPerInchY: " << screen->physicalDotsPerInchY() << '\n'
                 << indentx2 << "physicalDotsPerInch: " << screen->physicalDotsPerInch() << '\n'
                 << indentx2 << "logicalDotsPerInchX: " << screen->logicalDotsPerInchX() << '\n'
                 << indentx2 << "logicalDotsPerInchY: " << screen->logicalDotsPerInchY() << '\n'
                 << indentx2 << "logicalDotsPerInch: " << screen->logicalDotsPerInch() << '\n'
                 << indentx2 << "devicePixelRatio: " << screen->devicePixelRatio() << '\n'
                 << indentx2 << "logicalDotsPerInch: " << screen->logicalDotsPerInch() << '\n'
                 << indentx2 << "primaryOrientation: " << MetaEnum::name(screen->primaryOrientation()) << '\n'
                 << indentx2 << "orientation: " << MetaEnum::name(screen->orientation()) << '\n'
                 << indentx2 << "nativeOrientation: " << MetaEnum::name(screen->nativeOrientation()) << '\n'
                 << indentx2 << "refreshRate: " << screen->refreshRate() << "Hz" << '\n';
        }
    }

    // OpenGL
    ostr << '\n' << "OpenGL:" << '\n';
    dumpOpenGlInfo(ostr);

    ostr.flush();
    return strSysInfo;
}

} // namespace Mayo

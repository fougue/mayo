/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_help.h"

#include "app_module.h"
#include "command_system_information_occopengl.h"
#include "library_info.h"
#include "qtwidgets_utils.h"
#include "../base/meta_enum.h"
#include "../base/filepath.h"
#include "../base/io_system.h"
#include "../qtcommon/qstring_conv.h"
#include <common/mayo_version.h>

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

#include <Standard_Version.hxx>

#include <QtGui/QOpenGLContext>
#include <QtGui/QWindow>

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

// Helper function returning unicode representation of a QChar object in the form "U+NNNN"
QString toUnicodeHexa(const QChar& ch)
{
    QString strHexa;
    QTextStream ostr(&strHexa);
    ostr.setNumberFlags(ostr.numberFlags() | QTextStream::UppercaseDigits);
    ostr.setIntegerBase(16); // Same as Qt::hex
    ostr << "U+" << qSetFieldWidth(4) << qSetPadChar('0') << static_cast<int>(ch.unicode());
    return strHexa;
}

// Helper function returning unicode representation of a QString object in the form "U+NNNNU+PPPP..."
[[maybe_unused]] QString toUnicodeHexa(const QString& str)
{
    QString strHexa;
    QTextStream ostr(&strHexa);
    ostr.setNumberFlags(ostr.numberFlags() | QTextStream::UppercaseDigits);
    ostr.setIntegerBase(16); // Same as Qt::hex
    for (const QChar& ch : str)
        ostr << "U+" << qSetFieldWidth(4) << qSetPadChar('0') << static_cast<int>(ch.unicode());

    return strHexa;
}

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

    try {
        auto infos = Internal::getOccOpenGlInfos();
        for (const auto& [key, value] : infos) {
            str << indent << key << ": ";
            std::visit([&](const auto& arg) { str << arg; }, value);
            str << '\n';
        }
    }
    catch (const std::exception& error) {
        str << error.what() << '\n';
        return;
    }
}

QString CommandSystemInformation::data()
{
    QString strSysInfo;
    QTextStream ostr(&strSysInfo);

    // Mayo version
    ostr << '\n'
         << "Mayo: v" << strVersion
         << "  commit:" << strVersionCommitId
         << "  revnum:" << versionRevisionNumber
         << "  " << QT_POINTER_SIZE * 8 << "bit"
         << '\n'
    ;

    // OS version
    ostr << '\n' << "OS: " << QSysInfo::prettyProductName()
         << " [" << QSysInfo::kernelType() << " version " << QSysInfo::kernelVersion() << "]" << '\n'
         << "Current CPU Architecture: " << QSysInfo::currentCpuArchitecture() << '\n'
    ;

    // Qt version
    ostr << '\n' << QLibraryInfo::build() << " on \"" << QGuiApplication::platformName() << "\" " << '\n'
         << indent << "QStyle keys: " << QStyleFactory::keys().join("; ") << '\n'
         << indent << "Image formats(read): " << QImageReader::supportedImageFormats().join(' ') << '\n'
         << indent << "Image formats(write): " << QImageWriter::supportedImageFormats().join(' ') << '\n'
    ;

    // OpenCascade version
    ostr << '\n' << "OpenCascade: " << OCC_VERSION_STRING_EXT << " (build)" << '\n';

    // Other registered libraries
    for (const LibraryInfo& libInfo : AppModule::get()->libraryInfoArray()) {
        ostr << '\n' << libInfo.name << ": " << libInfo.version << " " << libInfo.versionDetails << '\n';
    }

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

    // Locales
    ostr << '\n' << "Locale:\n";
    {
        const std::locale& stdLoc = AppModule::get()->stdLocale();
        const auto& numFacet = std::use_facet<std::numpunct<wchar_t>>(stdLoc);
        const QChar charDecPnt(numFacet.decimal_point());
        const QChar char1000Sep(numFacet.thousands_sep());
        std::string strGrouping;
        for (char c : numFacet.grouping())
            strGrouping += std::to_string(static_cast<int>(c)) + " ";

        ostr << indent << "std::locale:" << '\n'
             << indentx2 << "name: " << to_QString(stdLoc.name()) << '\n'
             << indentx2 << "numpunct.decimal_point: " << charDecPnt << " " << toUnicodeHexa(charDecPnt) << '\n'
             << indentx2 << "numpunct.thousands_sep: " << char1000Sep << " " << toUnicodeHexa(char1000Sep) << '\n'
             << indentx2 << "numpunct.grouping: " << to_QString(strGrouping) << '\n'
             << indentx2 << "numpunct.truename: " << to_QString(numFacet.truename()) << '\n'
             << indentx2 << "numpunct.falsename: " << to_QString(numFacet.falsename()) << '\n'
        ;
        const QLocale& qtLoc = AppModule::get()->qtLocale();
        ostr << indent << "QLocale:" << '\n'
             << indentx2 << "name: " << qtLoc.name() << '\n'
             << indentx2 << "language: " << MetaEnum::name(qtLoc.language()) << '\n'
             << indentx2 << "measurementSytem: " << MetaEnum::name(qtLoc.measurementSystem()) << '\n'
             << indentx2 << "textDirection: " << MetaEnum::name(qtLoc.textDirection()) << '\n'
             << indentx2 << "decimalPoint: " << qtLoc.decimalPoint() << " " << toUnicodeHexa(qtLoc.decimalPoint()) << '\n'
             << indentx2 << "groupSeparator: " << qtLoc.groupSeparator() << " " << toUnicodeHexa(qtLoc.groupSeparator()) << '\n'
        ;
    }

    // C++ StdLib
    ostr << '\n' << "C++ StdLib:\n";
    {
        const QChar dirSeparator(FilePath::preferred_separator);
        ostr << indent << "std::thread::hardware_concurrency: " << std::thread::hardware_concurrency() << '\n'
             << indent << "std::filepath::path::preferred_separator: " << dirSeparator << " " << toUnicodeHexa(dirSeparator) << '\n'
        ;
    }

    // OpenGL
    ostr << '\n' << "OpenGL:" << '\n';
    dumpOpenGlInfo(ostr);

    // File selectors
    ostr << '\n' << "File selectors(increasing order of precedence):\n" << indent;
    for (const QString& selector : QFileSelector().allSelectors())
        ostr << selector << " ";

    ostr << '\n';

    // Fonts
    ostr << '\n' << "Fonts:\n";
    ostr << indent << "General font: " << QFontDatabase::systemFont(QFontDatabase::GeneralFont) << '\n'
         << indent << "Fixed font: " << QFontDatabase::systemFont(QFontDatabase::FixedFont) << '\n'
         << indent << "Title font: " << QFontDatabase::systemFont(QFontDatabase::TitleFont) << '\n'
         << indent << "Smallest font: " << QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont) << '\n'
    ;

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
                 << indentx2 << "refreshRate: " << screen->refreshRate() << "Hz" << '\n'
            ;
        }
    }

    // QGuiApplication
    ostr << '\n' << "QGuiApplication:\n";
    ostr << indent << "platformName: " << QGuiApplication::platformName() << '\n'
         << indent << "desktopFileName: " << QGuiApplication::desktopFileName() << '\n'
         << indent << "desktopSettingsAware: " << QGuiApplication::desktopSettingsAware() << '\n'
         << indent << "layoutDirection: " << MetaEnum::name(QGuiApplication::layoutDirection()) << '\n'
    ;
    const QStyleHints* sh = QGuiApplication::styleHints();
    if (sh) {
        const auto pwdChar = sh->passwordMaskCharacter();
        ostr << indent << "styleHints:\n"
             << indentx2 << "keyboardAutoRepeatRate: " << sh->keyboardAutoRepeatRate() << '\n'
             << indentx2 << "keyboardInputInterval: " << sh->keyboardInputInterval() << '\n'
             << indentx2 << "mouseDoubleClickInterval: " << sh->mouseDoubleClickInterval() << '\n'
             << indentx2 << "mousePressAndHoldInterval: " << sh->mousePressAndHoldInterval() << '\n'
             << indentx2 << "passwordMaskCharacter: " << pwdChar << " " << toUnicodeHexa(pwdChar) << '\n'
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
             << indentx2 << "mouseDoubleClickDistance: " << sh->mouseDoubleClickDistance() << '\n'
             << indentx2 << "touchDoubleTapDistance: " << sh->touchDoubleTapDistance() << '\n'
#endif
        ;
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

    // Environment
    ostr << '\n' << "Environment:\n";
    {
        const QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
        for (const QString& key : sysEnv.keys())
            ostr << indent << key << "=\"" << sysEnv.value(key) << "\"\n";
    }

    ostr.flush();
    return strSysInfo;
}

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module.h"

#include "../base/bnd_utils.h"
#include "../base/brep_utils.h"
#include "../base/cpp_utils.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../base/tkernel_utils.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qtcore_utils.h"

#include <BRepBndLib.hxx>

#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QtDebug>

#include <fmt/format.h>
#include <iterator>

namespace Mayo {

namespace {

void readRecentFile(QDataStream& stream, RecentFile* recentFile)
{
    QString strFilepath;
    stream >> strFilepath;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->filepath = filepathFrom(strFilepath);
    stream >> recentFile->thumbnail.imageData;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->thumbnail.imageCacheKey = -1;
    // Read thumbnail timestamp
    // Warning: qint64 and int64_t may not be the exact same type(eg __int64 and longlong with Windows/MSVC)
    qint64 timestamp;
    stream >> timestamp;
    if (stream.status() != QDataStream::Ok)
        return;

    recentFile->thumbnailTimestamp = timestamp;
}

QuantityLength shapeChordalDeflection(const TopoDS_Shape& shape)
{
    // Excerpted from Prs3d::GetDeflection(...)
    constexpr QuantityLength baseDeviation = 1 * Quantity_Millimeter;

    Bnd_Box bndBox;
    constexpr bool useTriangulation = true;
    BRepBndLib::Add(shape, bndBox, !useTriangulation);
    if (bndBox.IsVoid())
        return baseDeviation;

    if (BndUtils::isOpen(bndBox)) {
        if (!BndUtils::hasFinitePart(bndBox))
            return baseDeviation;

        bndBox = BndUtils::finitePart(bndBox);
    }

    const auto coords = BndBoxCoords::get(bndBox);
    const gp_XYZ diag = coords.maxVertex().XYZ() - coords.minVertex().XYZ();
    const double diagMaxComp = std::max({ diag.X(), diag.Y(), diag.Z() });
    return 4 * diagMaxComp * baseDeviation;
}

} // namespace

AppModule::AppModule()
    : m_application(new Application),
      m_settings(new Settings),
      m_props(m_settings),
      m_stdLocale(std::locale("")),
      m_qtLocale(QLocale::system())
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qRegisterMetaType<MessageType>("MessageType");
        metaTypesRegistered = true;
    }

    m_settings->setPropertyValueConversion(this);
    Application::defineMayoFormat(m_application);
}

QStringUtils::TextOptions AppModule::defaultTextOptions() const
{
    QStringUtils::TextOptions opts;
    opts.locale = this->qtLocale();
    opts.unitDecimals = m_props.unitSystemDecimals;
    opts.unitSchema = m_props.unitSystemSchema;
    return opts;
}

const std::locale& AppModule::stdLocale() const
{
    return m_stdLocale;
}

const QLocale& AppModule::qtLocale() const
{
    return m_qtLocale;
}

const Enumeration& AppModule::languages()
{
    static const Enumeration langs = {
        { 0, AppModule::textId("en") },
        { 1, AppModule::textId("fr") },
        { 2, AppModule::textId("zh") },
    };
    return langs;
}

QString AppModule::languageCode() const
{
    const char keyLang[] = "application/language";
    const Settings::Variant code = m_settings->findValueFromKey(keyLang);
    const Enumeration& langs = AppModule::languages();
    if (code.isConvertibleToConstRefString()) {
        const std::string& strCode = code.toConstRefString();
        if (langs.contains(strCode))
            return QString::fromStdString(strCode);
    }

    std::string_view langDefault = langs.findNameByValue(0);
    return QString::fromUtf8(langDefault.data(), CppUtils::safeStaticCast<int>(langDefault.size()));
}

void AppModule::addLibraryInfo(const LibraryInfo& lib)
{
    if (!lib.name.empty() && !lib.version.empty())
        m_vecLibraryInfo.push_back(lib);
}

void AppModule::addLibraryInfo(
        std::string_view libName, std::string_view version, std::string_view versionDetails
    )
{
    const LibraryInfo libInfo{
        std::string{libName}, std::string{version}, std::string{versionDetails}
    };
    this->addLibraryInfo(libInfo);
}

Span<const LibraryInfo> AppModule::libraryInfoArray() const
{
    return m_vecLibraryInfo;
}

bool AppModule::excludeSettingPredicate(const Property& prop)
{
    return !prop.isUserVisible();
}

const PropertyGroup* AppModule::findReaderParameters(IO::Format format) const
{
    auto it = m_props.m_mapFormatReaderParameters.find(format);
    return it != m_props.m_mapFormatReaderParameters.cend() ? it->second : nullptr;
}

const PropertyGroup* AppModule::findWriterParameters(IO::Format format) const
{
    auto it = m_props.m_mapFormatWriterParameters.find(format);
    return it != m_props.m_mapFormatWriterParameters.cend() ? it->second : nullptr;
}

Settings::Variant AppModule::toVariant(const Property& prop) const
{
    if (isType<PropertyRecentFiles>(prop)) {
        const auto& filesProp = constRef<PropertyRecentFiles>(prop);
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        AppModule::writeRecentFiles(stream, filesProp.value());
        return Variant(QtCoreUtils::toStdByteArray(blob));
    }
    else if (isType<PropertyAppUiState>(prop)) {
        return Variant(AppUiState::toBlob(constRef<PropertyAppUiState>(prop)));
    }
    else {
        return PropertyValueConversion::toVariant(prop);
    }
}

bool AppModule::fromVariant(Property* prop, const Settings::Variant& variant) const
{
    if (isType<PropertyRecentFiles>(prop)) {
        const QByteArray blob = QtCoreUtils::QByteArray_fromRawData(variant.toConstRefByteArray());
        QDataStream stream(blob);
        RecentFiles recentFiles;
        AppModule::readRecentFiles(stream, &recentFiles);
        ptr<PropertyRecentFiles>(prop)->setValue(recentFiles);
        return stream.status() == QDataStream::Ok;
    }
    else if (isType<PropertyAppUiState>(prop)) {
        bool ok = false;
        auto uiState = AppUiState::fromBlob(variant.toConstRefByteArray(), &ok);
        ptr<PropertyAppUiState>(prop)->setValue(uiState);
        return ok;
    }
    else {
        return PropertyValueConversion::fromVariant(prop, variant);
    }
}

void AppModule::emitMessage(MessageType msgType, std::string_view text)
{
    const std::string stext{text};
    const Messenger::Message* msg = nullptr;
    {
        [[maybe_unused]] std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.push_back({ msgType, stext });
        msg = &m_messageLog.back();
    }

    this->signalMessage.send(*msg);
}

void AppModule::clearMessageLog()
{
    {
        [[maybe_unused]] std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.clear();
    }

    this->signalMessageLogCleared.send();
}

void AppModule::prependRecentFile(const FilePath& fp)
{
    const RecentFile* ptrRecentFile = this->findRecentFile(fp);
    RecentFiles newRecentFiles = m_props.recentFiles.value();
    if (ptrRecentFile) {
        RecentFile& firstRecentFile = newRecentFiles.front();
        RecentFile& recentFile = newRecentFiles.at(ptrRecentFile - &m_props.recentFiles.value().front());
        std::swap(firstRecentFile, recentFile);
    }
    else {
        RecentFile recentFile;
        recentFile.filepath = fp;
        newRecentFiles.insert(newRecentFiles.begin(), std::move(recentFile));
        constexpr int sizeLimit = 15;
        while (newRecentFiles.size() > sizeLimit)
            newRecentFiles.pop_back();
    }

    m_props.recentFiles.setValue(newRecentFiles);
}

const RecentFile* AppModule::findRecentFile(const FilePath& fp) const
{
    const RecentFiles& listRecentFile = m_props.recentFiles.value();
    auto itFound =
            std::find_if(
                listRecentFile.cbegin(),
                listRecentFile.cend(),
                [=](const RecentFile& recentFile) {
        return filepathEquivalent(fp, recentFile.filepath);
    });
    return itFound != listRecentFile.cend() ? &(*itFound) : nullptr;
}

void AppModule::recordRecentFile(GuiDocument* guiDoc)
{
    if (!guiDoc)
        return;

    const RecentFile* recentFile = this->findRecentFile(guiDoc->document()->filePath());
    if (!recentFile) {
        qDebug() << fmt::format(
                        "RecentFile object is null\n"
                        "    Function: {}\n    Document: {}\n    RecentFilesCount: {}",
                        Q_FUNC_INFO,
                        guiDoc->document()->filePath().u8string(),
                        m_props.recentFiles.value().size()
                    ).c_str();
        return;
    }

    if (!recentFile->isThumbnailOutOfSync())
        return;

    RecentFile newRecentFile = *recentFile;
    const bool okRecord = this->impl_recordRecentFile(&newRecentFile, guiDoc);
    if (!okRecord)
        return;

    const RecentFiles& listRecentFile = m_props.recentFiles.value();
    RecentFiles newListRecentFile = listRecentFile;
    const auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
    newListRecentFile.at(indexRecentFile) = newRecentFile;
    m_props.recentFiles.setValue(newListRecentFile);
}

void AppModule::recordRecentFiles(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

    const RecentFiles& listRecentFile = m_props.recentFiles.value();
    RecentFiles newListRecentFile = listRecentFile;
    for (GuiDocument* guiDoc : guiApp->guiDocuments()) {
        const RecentFile* recentFile = this->findRecentFile(guiDoc->document()->filePath());
        if (!recentFile || !recentFile->isThumbnailOutOfSync())
            continue; // Skip

        RecentFile newRecentFile = *recentFile;
        if (this->impl_recordRecentFile(&newRecentFile, guiDoc)) {
            auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
            newListRecentFile.at(indexRecentFile) = newRecentFile;
        }
    }

    m_props.recentFiles.setValue(newListRecentFile);
}

void AppModule::setRecentFileThumbnailRecorder(std::function<Thumbnail(GuiDocument*, QSize)> fn)
{
    m_fnRecentFileThumbnailRecorder = std::move(fn);
}


void AppModule::readRecentFiles(QDataStream& stream, RecentFiles* recentFiles)
{
    auto fnCheckStreamStatus = [](QDataStream::Status status) {
        if (status != QDataStream::Ok) {
            qDebug() << fmt::format(
                            "QDataStream error\n    Function: {}\n    Status: {}",
                            Q_FUNC_INFO, MetaEnum::name(status)
                        ).c_str();
            return false;
        }

        return true;
    };

    uint32_t count = 0;
    stream >> count;
    if (!fnCheckStreamStatus(stream.status()))
        return; // Stream extraction error, abort

    recentFiles->clear();
    for (uint32_t i = 0; i < count; ++i) {
        RecentFile recent;
        readRecentFile(stream, &recent);
        if (!fnCheckStreamStatus(stream.status()))
            return; // Stream extraction error, abort

        if (!recent.filepath.empty() && recent.thumbnailTimestamp != 0)
            recentFiles->push_back(std::move(recent));
    }
}

void AppModule::writeRecentFiles(QDataStream& stream, const RecentFiles& recentFiles)
{
    stream << uint32_t(recentFiles.size());
    for (const RecentFile& rf : recentFiles) {
        stream << filepathTo<QString>(rf.filepath);
        stream << rf.thumbnail.imageData;
        stream << qint64(rf.thumbnailTimestamp);
    }
}

OccBRepMeshParameters AppModule::brepMeshParameters(const TopoDS_Shape& shape) const
{
    using BRepMeshQuality = AppModuleProperties::BRepMeshQuality;

    OccBRepMeshParameters params;
    params.InParallel = true;
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    params.AllowQualityDecrease = true;
#endif
    if (m_props.meshingQuality == BRepMeshQuality::UserDefined) {
        params.Deflection = UnitSystem::meters(m_props.meshingChordalDeflection.quantity());
        params.Angle = UnitSystem::radians(m_props.meshingAngularDeflection.quantity());
        params.Relative = m_props.meshingRelative;
    }
    else {
        struct Coefficients {
            double chordalDeflection;
            double angularDeflection;
        };
        auto fnCoefficients = [](BRepMeshQuality meshQuality) -> Coefficients {
            switch (meshQuality) {
            case BRepMeshQuality::VeryCoarse: return { 8, 4 };
            case BRepMeshQuality::Coarse: return { 4, 2 };
            case BRepMeshQuality::Normal: return { 1, 1 };
            case BRepMeshQuality::Precise: return { 1/4., 1/2. };
            case BRepMeshQuality::VeryPrecise: return { 1/8., 1/4. };
            case BRepMeshQuality::UserDefined: return { -1, -1 };
            }
            return { 1, 1 };
        };
        const Coefficients coeffs = fnCoefficients(m_props.meshingQuality);
        params.Deflection = UnitSystem::meters(coeffs.chordalDeflection * shapeChordalDeflection(shape));
        params.Angle = UnitSystem::radians(coeffs.angularDeflection * (20 * Quantity_Degree));
    }

    return params;
}

void AppModule::computeBRepMesh(const TopoDS_Shape& shape, TaskProgress* progress)
{
    BRepUtils::computeMesh(shape, this->brepMeshParameters(shape), progress);
}

void AppModule::computeBRepMesh(const TDF_Label& labelEntity, TaskProgress* progress)
{
    if (XCaf::isShape(labelEntity))
        this->computeBRepMesh(XCaf::shape(labelEntity), progress);
}

void AppModule::addPropertiesProvider(std::unique_ptr<DocumentTreeNodePropertiesProvider> ptr)
{
    m_vecDocTreeNodePropsProvider.push_back(std::move(ptr));
}

const DocumentTreeNodePropertiesProvider* AppModule::findPropertiesProvider(const DocumentTreeNode& treeNode) const
{
    for (const auto& provider : m_vecDocTreeNodePropsProvider) {
        if (provider->supports(treeNode))
            return provider.get();
    }

    return nullptr;
}

std::unique_ptr<PropertyGroup> AppModule::properties(const DocumentTreeNode& treeNode) const
{
    const auto provider = this->findPropertiesProvider(treeNode);
    return provider ? provider->properties(treeNode) : std::unique_ptr<PropertyGroup>{};
}

AppModule* AppModule::get()
{
    static AppModule appModule;
    return &appModule;
}

AppModule::~AppModule()
{
    delete m_settings;
    m_settings = nullptr;
}

bool AppModule::impl_recordRecentFile(RecentFile* recentFile, GuiDocument* guiDoc)
{
    if (!recentFile)
        return false;

    if (!guiDoc)
        return false;

    if (!m_fnRecentFileThumbnailRecorder)
        return false;

    if (!filepathEquivalent(recentFile->filepath, guiDoc->document()->filePath())) {
        qDebug() << fmt::format(
                        "Filepath mismatch with GUI document\n"
                        "    Function: {}\n    Filepath: {}\n    Document: {}",
                        Q_FUNC_INFO,
                        recentFile->filepath.u8string(),
                        guiDoc->document()->filePath().u8string()
                    ).c_str();
        return false;
    }

    if (recentFile->thumbnailTimestamp == RecentFile::timestampLastModified(recentFile->filepath))
        return true;

    recentFile->thumbnail = m_fnRecentFileThumbnailRecorder(guiDoc, this->recentFileThumbnailSize());
    recentFile->thumbnailTimestamp = RecentFile::timestampLastModified(recentFile->filepath);
    return true;
}

} // namespace Mayo

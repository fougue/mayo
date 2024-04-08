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
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"

#include <BRepBndLib.hxx>

#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QtDebug>

#include <fmt/format.h>
#include <ios>
#include <iterator>
#include <type_traits>

namespace Mayo {

namespace {

template<typename T>
struct Serializer {
    static auto fnType()
    {
        if constexpr(std::is_same_v<T, int>)
            return qint32{};
        else if constexpr(std::is_same_v<T, unsigned>)
            return quint32{};
        else if constexpr(std::is_same_v<T, std::int64_t>)
            return qint64{};
        else if constexpr(std::is_same_v<T, std::uint64_t>)
            return quint64{};
        else if constexpr(std::is_same_v<T, std::time_t>)
            return qint64{};
        else if constexpr(std::is_same_v<T, FilePath>)
            return QString{};
        else
            return T{};
    }

    using StreamType = decltype(Serializer<T>::fnType());

    static void read(QDataStream& stream, T& v)
    {
        if constexpr(std::is_same_v<T, FilePath>) {
            QString strFilePath;
            stream >> strFilePath;
            v = filepathFrom(strFilePath);
        }
        else {
            StreamType qv;
            stream >> qv;
            v = static_cast<T>(qv);
        }
    }

    static void write(QDataStream& stream, const T& v)
    {
        if constexpr(std::is_same_v<T, FilePath>) {
            stream << filepathTo<QString>(v);
        }
        else {
            stream << static_cast<StreamType>(v);
        }
    }
};

bool checkDataStreamStatus(QDataStream::Status status)
{
    if (status != QDataStream::Ok) {
        qDebug() << fmt::format(
                        "QDataStream error\n    Function: {}\n    Status: {}",
                        Q_FUNC_INFO, MetaEnum::name(status)
                    ).c_str();
        return false;
    }

    return true;
}

template<typename T>
void dataStreamRead(QDataStream& stream, T& v)
{
    Serializer<T>::read(stream, v);
    if (!checkDataStreamStatus(stream.status()))
        throw std::ios_base::failure("serialize() error with QDataStream");
}

template<typename T>
void dataStreamWrite(QDataStream& stream, T v)
{
    Serializer<T>::write(stream, v);
}

template<typename T> void check_has_filepath_attribute() {
    static_assert(
        std::is_same_v<decltype(T::filepath), FilePath>,
        "Type 'T' must have 'filepath' member attribute of type 'std::filesystem::path'"
    );
}

template<typename RecentItem>
const RecentItem* findRecentItem(const FilePath& fp, const std::vector<RecentItem>& listRecentItem)
{
    check_has_filepath_attribute<RecentItem>();
    auto itFound =
        std::find_if(
            listRecentItem.cbegin(),
            listRecentItem.cend(),
            [=](const RecentItem& recentItem) { return filepathEquivalent(fp, recentItem.filepath); }
        );
    return itFound != listRecentItem.cend() ? &(*itFound) : nullptr;
}

template<typename RecentItem>
void prependRecentItem(
        const FilePath& fp,
        GenericProperty<std::vector<RecentItem>>& propRecentItems,
        std::function<void(RecentItem&)> fnUpdateRecentItem = nullptr
    )
{
    using RecentItems = std::vector<RecentItem>;
    const RecentItem* ptrRecentItem = findRecentItem(fp, propRecentItems.value());
    RecentItems newRecentItems = propRecentItems.value();
    if (ptrRecentItem) {
        RecentItem& firstRecentItem = newRecentItems.front();
        RecentItem& recentItem = newRecentItems.at(ptrRecentItem - &propRecentItems.value().front());
        if (fnUpdateRecentItem)
            fnUpdateRecentItem(recentItem);

        std::swap(firstRecentItem, recentItem);
    }
    else {
        RecentItem recentItem;
        recentItem.filepath = fp;
        if (fnUpdateRecentItem)
            fnUpdateRecentItem(recentItem);

        newRecentItems.insert(newRecentItems.begin(), std::move(recentItem));
        constexpr unsigned sizeLimit = 15;
        while (newRecentItems.size() > sizeLimit)
            newRecentItems.pop_back();
    }

    propRecentItems.setValue(newRecentItems);
}

template<typename RecentItem>
Settings::Variant recentItemsToVariant(const std::vector<RecentItem>& recentItems)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    AppModule::write(stream, recentItems);
    Settings::Variant varBlob(blob.toStdString());
    varBlob.setByteArray(true);
    return varBlob;
}

template<typename RecentItem>
std::vector<RecentItem> recentItemsFromVariant(const Settings::Variant& variant, bool* ok = nullptr)
{
    const QByteArray blob = QtCoreUtils::QByteArray_frowRawData(variant.toConstRefString());
    QDataStream stream(blob);
    std::vector<RecentItem> recentItems;
    AppModule::read(stream, &recentItems);
    if (ok)
        *ok = stream.status() == QDataStream::Ok;

    return recentItems;
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
        { 1, AppModule::textId("fr") }
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
        return recentItemsToVariant(filesProp.value());
    }
    else if (isType<PropertyRecentScripts>(prop)) {
        const auto& scriptsProp = constRef<PropertyRecentScripts>(prop);
        return recentItemsToVariant(scriptsProp.value());
    }
    else {
        return PropertyValueConversion::toVariant(prop);
    }
}

bool AppModule::fromVariant(Property* prop, const Settings::Variant& variant) const
{
    bool ok = false;
    if (isType<PropertyRecentFiles>(prop)) {
        auto recentFiles = recentItemsFromVariant<RecentFile>(variant, &ok);
        ptr<PropertyRecentFiles>(prop)->setValue(recentFiles);
    }
    else if (isType<PropertyRecentScripts>(prop)) {
        auto recentScripts = recentItemsFromVariant<RecentScript>(variant, &ok);
        ptr<PropertyRecentScripts>(prop)->setValue(recentScripts);
    }
    else {
        ok = PropertyValueConversion::fromVariant(prop, variant);
    }

    return ok;
}

void AppModule::emitMessage(MessageType msgType, std::string_view text)
{
    const QString qtext = to_QString(text);
    {
        [[maybe_unused]] std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.push_back({ msgType, qtext });
    }

    this->signalMessage.send(msgType, qtext);
}

void AppModule::clearMessageLog()
{
    {
        [[maybe_unused]] std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.clear();
    }

    this->signalMessageLogCleared.send();
}

void AppModule::prependRecentFile(const FilePath& fp, GuiDocument* guiDoc)
{
    auto fnUpdate = [=](RecentFile& recent) {
        this->impl_recordRecentFile(&recent, guiDoc);
    };
    prependRecentItem<RecentFile>(fp, m_props.recentFiles, fnUpdate);
}

const RecentFile* AppModule::findRecentFile(const FilePath& fp) const
{
    return findRecentItem(fp, m_props.recentFiles.value());
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

void AppModule::read(QDataStream& stream, RecentFiles* recentFiles)
{
    try {
        unsigned count = 0;
        dataStreamRead(stream, count);
        recentFiles->clear();
        for (unsigned i = 0; i < count; ++i) {
            RecentFile recent;
            dataStreamRead(stream, recent.filepath);
            dataStreamRead(stream, recent.thumbnail.imageData);
            recent.thumbnail.imageCacheKey = -1;
            // Read thumbnail timestamp
            // Warning: qint64 and int64_t may not be the exact same type(eg __int64 and longlong with Windows/MSVC)
            dataStreamRead(stream, recent.thumbnailTimestamp);
            if (!recent.filepath.empty() && recent.thumbnailTimestamp != 0)
                recentFiles->push_back(std::move(recent));
        }
    }
    catch (const std::exception&) {
    }
}

void AppModule::write(QDataStream& stream, const RecentFiles& recentFiles)
{
    dataStreamWrite(stream, unsigned(recentFiles.size()));
    for (const RecentFile& rf : recentFiles) {
        dataStreamWrite(stream, rf.filepath);
        dataStreamWrite(stream, rf.thumbnail.imageData);
        dataStreamWrite(stream, rf.thumbnailTimestamp);
    }
}

void AppModule::prependRecentScript(const FilePath& fp)
{
    auto fnUpdate = [](RecentScript& recent) {
        ++recent.executionCount;
        std::timespec ts;
        std::timespec_get(&ts, TIME_UTC);
        recent.lastExecutionDateTime = ts.tv_sec;
    };
    prependRecentItem<RecentScript>(fp, m_props.recentScripts, fnUpdate);
}

const RecentScript* AppModule::findRecentScript(const FilePath& fp) const
{
    return findRecentItem(fp, m_props.recentScripts.value());
}

void AppModule::read(QDataStream& stream, RecentScripts* recentScripts)
{
    try {
        unsigned count = 0;
        dataStreamRead(stream, count);
        recentScripts->clear();
        for (unsigned i = 0; i < count; ++i) {
            RecentScript recent;
            dataStreamRead(stream, recent.filepath);
            dataStreamRead(stream, recent.executionCount);
            dataStreamRead(stream, recent.lastExecutionDateTime);
            if (!recent.filepath.empty())
                recentScripts->push_back(std::move(recent));
        }
    }
    catch (const std::exception&) {
    }
}

void AppModule::write(QDataStream& stream, const RecentScripts& recentScripts)
{
    dataStreamWrite(stream, unsigned(recentScripts.size()));
    for (const RecentScript& rs : recentScripts) {
        dataStreamWrite(stream, rs.filepath);
        dataStreamWrite(stream, rs.executionCount);
        dataStreamWrite(stream, rs.lastExecutionDateTime);
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

std::unique_ptr<PropertyGroupSignals> AppModule::properties(const DocumentTreeNode& treeNode) const
{
    for (const auto& provider : m_vecDocTreeNodePropsProvider) {
        if (provider->supports(treeNode))
            return provider->properties(treeNode);
    }

    return std::unique_ptr<PropertyGroupSignals>();
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

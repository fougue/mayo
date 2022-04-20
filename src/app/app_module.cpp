/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module.h"

#include "../base/application.h"
#include "../base/bnd_utils.h"
#include "../base/brep_utils.h"
#include "../base/cpp_utils.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "qtcore_utils.h"
#include "filepath_conv.h"
#include "qstring_conv.h"

#include <BRepBndLib.hxx>

#include <QtCore/QDir>
#include <QtCore/QtDebug>
#include <QtGui/QGuiApplication>

#include <fmt/format.h>
#include <iterator>

namespace Mayo {

AppModule::AppModule()
    : m_settings(new Settings(this)),
      m_props(m_settings),
      m_locale(QLocale::system())
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qRegisterMetaType<MessageType>("Messenger::MessageType");
        metaTypesRegistered = true;
    }

    m_settings->setPropertyValueConversion(this);
}

QStringUtils::TextOptions AppModule::defaultTextOptions() const
{
    QStringUtils::TextOptions opts;
    opts.locale = this->locale();
    opts.unitDecimals = m_props.unitSystemDecimals;
    opts.unitSchema = m_props.unitSystemSchema;
    return opts;
}

const QLocale& AppModule::locale() const
{
    return m_locale;
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
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << filesProp.value();
        Variant varBlob(blob.toStdString());
        varBlob.setByteArray(true);
        return varBlob;
    }
    else {
        return PropertyValueConversion::toVariant(prop);
    }
}

bool AppModule::fromVariant(Property* prop, const Settings::Variant& variant) const
{
    if (isType<PropertyRecentFiles>(prop)) {
        if (qobject_cast<QGuiApplication*>(QCoreApplication::instance()) == nullptr)
            return true;

        const QByteArray blob = QtCoreUtils::QByteArray_frowRawData(variant.toConstRefString());
        QDataStream stream(blob);
        RecentFiles recentFiles;
        stream >> recentFiles;
        ptr<PropertyRecentFiles>(prop)->setValue(recentFiles);
        return stream.status() == QDataStream::Ok;
    }
    else {
        return PropertyValueConversion::fromVariant(prop, variant);
    }
}

void AppModule::emitMessage(Messenger::MessageType msgType, std::string_view text)
{
    {
        std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.push_back({ msgType, to_QString(text) });
    }

    emit this->message(msgType, to_QString(text));
}

void AppModule::clearMessageLog()
{
    {
        std::lock_guard<std::mutex> lock(m_mutexMessageLog);
        m_messageLog.clear();
    }

    emit this->messageLogCleared();
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

void AppModule::recordRecentFileThumbnail(GuiDocument* guiDoc)
{
    if (!guiDoc)
        return;

    const RecentFile* recentFile = this->findRecentFile(guiDoc->document()->filePath());
    if (!recentFile) {
        qDebug() << fmt::format("RecentFile object is null\n"
                                "    Function: {}\n    Document: {}\n    RecentFilesCount: {}",
                                Q_FUNC_INFO,
                                guiDoc->document()->filePath().u8string(),
                                m_props.recentFiles.value().size())
                    .c_str();
        return;
    }

    if (!recentFile->isThumbnailOutOfSync())
        return;

    RecentFile newRecentFile = *recentFile;
    const bool okRecord = newRecentFile.recordThumbnail(guiDoc, this->recentFileThumbnailSize());
    if (!okRecord)
        return;

    const RecentFiles& listRecentFile = m_props.recentFiles.value();
    RecentFiles newListRecentFile = listRecentFile;
    const auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
    newListRecentFile.at(indexRecentFile) = newRecentFile;
    m_props.recentFiles.setValue(newListRecentFile);
}

void AppModule::recordRecentFileThumbnails(GuiApplication* guiApp)
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
        if (newRecentFile.recordThumbnail(guiDoc, this->recentFileThumbnailSize())) {
            auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
            newListRecentFile.at(indexRecentFile) = newRecentFile;
        }
    }

    m_props.recentFiles.setValue(newListRecentFile);
}

static QuantityLength shapeChordalDeflection(const TopoDS_Shape& shape)
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

const DocumentTreeNodePropertiesProviderTable* AppModule::documentTreeNodePropertiesProviderTable() const
{
    return &m_docTreeNodePropsProviderTable;
}

DocumentTreeNodePropertiesProviderTable* AppModule::documentTreeNodePropertiesProviderTable()
{
    return &m_docTreeNodePropsProviderTable;
}

AppModule* AppModule::get()
{
    static AppModule appModule;
    return &appModule;
}

} // namespace Mayo

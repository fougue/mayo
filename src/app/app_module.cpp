/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "app_module.h"

#include "app_module_properties.h"
#include "../base/io_parameters_provider.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../qtcommon/qtcore_utils.h"

#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QtDebug>

#include <fmt/format.h>
#include <iterator>
#include <mutex>

namespace Mayo {

namespace {

class AppModulePropertyValueConversion : public PropertyValueConversion {
public:
    Variant toVariant(const Property& prop) const override
    {
        if (isType<PropertyRecentFiles>(prop)) {
            const auto& filesProp = constRef<PropertyRecentFiles>(prop);
            QByteArray blob;
            QDataStream stream(&blob, QIODevice::WriteOnly);
            RecentFileIO::write(stream, filesProp.value());
            return Variant{QtCoreUtils::toStdByteArray(blob)};
        }
        else if (isType<PropertyAppUiState>(prop)) {
            return Variant{AppUiState::toBlob(constRef<PropertyAppUiState>(prop))};
        }
        else {
            return PropertyValueConversion::toVariant(prop);
        }
    }

    bool fromVariant(Property* prop, const Variant& variant) const override
    {
        if (isType<PropertyRecentFiles>(prop)) {
            const QByteArray blob = QtCoreUtils::QByteArray_fromRawData(variant.toConstRefByteArray());
            QDataStream stream(blob);
            RecentFiles recentFiles;
            RecentFileIO::read(stream, &recentFiles);
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
};

class AppModuleIOParametersProvider : public IO::ParametersProvider {
public:
    AppModuleIOParametersProvider(
            const std::unordered_map<IO::Format, PropertyGroup*>& mapFormatReaderParameters,
            const std::unordered_map<IO::Format, PropertyGroup*>& mapFormatWriterParameters
        ) :
        m_mapFormatReaderParameters(mapFormatReaderParameters),
        m_mapFormatWriterParameters(mapFormatWriterParameters)
    {
    }

    const PropertyGroup* findReaderParameters(IO::Format format) const override
    {
        auto it = m_mapFormatReaderParameters.find(format);
        return it != m_mapFormatReaderParameters.cend() ? it->second : nullptr;
    }

    const PropertyGroup* findWriterParameters(IO::Format format) const override
    {
        auto it = m_mapFormatWriterParameters.find(format);
        return it != m_mapFormatWriterParameters.cend() ? it->second : nullptr;
    }

private:
    const std::unordered_map<IO::Format, PropertyGroup*>& m_mapFormatReaderParameters;
    const std::unordered_map<IO::Format, PropertyGroup*>& m_mapFormatWriterParameters;
};

} // namespace

struct AppModule::Private {
    Private() :
        m_application(makeOccHandle<Application>()),
        m_props(&m_settings),
        m_appIOParamsProvider(m_props.m_mapFormatReaderParameters, m_props.m_mapFormatWriterParameters),
        m_stdLocale(std::locale("")),
        m_qtLocale(QLocale::system())
    {
    }

    ApplicationPtr m_application;
    Settings m_settings;
    IO::System m_ioSystem;
    AppModuleProperties m_props;
    AppModulePropertyValueConversion m_appPropValueConversion;
    AppModuleIOParametersProvider m_appIOParamsProvider;
    std::vector<Messenger::Message> m_messageLog;
    std::mutex m_mutexMessageLog;
    std::locale m_stdLocale;
    QLocale m_qtLocale;
    std::vector<std::unique_ptr<DocumentTreeNodePropertiesProvider>> m_vecDocTreeNodePropsProvider;
    std::vector<LibraryInfo> m_vecLibraryInfo;
};

AppModule::AppModule()
    : d(new Private)
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qRegisterMetaType<MessageType>("MessageType");
        metaTypesRegistered = true;
    }

    Application::defineMayoFormat(d->m_application);
    d->m_settings.setPropertyValueConversion(&d->m_appPropValueConversion);
    d->m_settings.signalPropertyChanged.connectSlot([this](const Property* prop){
        if (prop == &d->m_props.autoExpandCompoundToAssembly)
            d->m_application->setAutoExpandCompoundToAssembly(d->m_props.autoExpandCompoundToAssembly);
    });
}

AppModule::~AppModule()
{
    delete d;
}

const ApplicationPtr& AppModule::application() const
{
    return d->m_application;
}

const AppModuleProperties* AppModule::properties() const
{
    return &d->m_props;
}

AppModuleProperties* AppModule::properties()
{
    return &d->m_props;
}

Settings* AppModule::settings()
{
    return &d->m_settings;
}

const Settings* AppModule::settings() const
{
    return &d->m_settings;
}

QStringUtils::TextOptions AppModule::defaultTextOptions() const
{
    QStringUtils::TextOptions opts;
    opts.locale = this->qtLocale();
    opts.unitDecimals = this->properties()->unitSystemDecimals;
    opts.unitSchema = this->properties()->unitSystemSchema;
    return opts;
}

const std::locale& AppModule::stdLocale() const
{
    return d->m_stdLocale;
}

const QLocale& AppModule::qtLocale() const
{
    return d->m_qtLocale;
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
    const Settings::Variant code = this->settings()->findValueFromKey(keyLang);
    const Enumeration& langs = AppModule::languages();
    if (code.isConvertibleToConstRefString()) {
        const std::string& strCode = code.toConstRefString();
        if (langs.contains(strCode))
            return QString::fromStdString(strCode);
    }

    std::string_view langDefault = langs.findNameByValue(0);
    return QString::fromUtf8(langDefault.data(), static_cast<int>(langDefault.size()));
}

void AppModule::addLibraryInfo(const LibraryInfo& lib)
{
    if (!lib.name.empty() && !lib.version.empty())
        d->m_vecLibraryInfo.push_back(lib);
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

gsl::span<const LibraryInfo> AppModule::libraryInfoArray() const
{
    return d->m_vecLibraryInfo;
}

bool AppModule::excludeSettingPredicate(const Property& prop)
{
    return !prop.isUserVisible();
}

const IO::System* AppModule::ioSystem() const
{
    return &d->m_ioSystem;
}

IO::System* AppModule::ioSystem()
{
    return &d->m_ioSystem;
}

const IO::ParametersProvider* AppModule::ioParametersProvider() const
{
    return &d->m_appIOParamsProvider;
}

void AppModule::emitMessage(MessageType msgType, std::string_view text)
{
    const std::string stext{text};
    const Messenger::Message* msg = nullptr;
    {
        [[maybe_unused]] std::scoped_lock lock(d->m_mutexMessageLog);
        d->m_messageLog.push_back({ msgType, stext });
        msg = &d->m_messageLog.back();
    }

    this->signalMessage.send(*msg);
}

void AppModule::clearMessageLog()
{
    {
        [[maybe_unused]] std::scoped_lock lock(d->m_mutexMessageLog);
        d->m_messageLog.clear();
    }

    this->signalMessageLogCleared.send();
}

gsl::span<const Messenger::Message> AppModule::messageLog() const
{
    return d->m_messageLog;
}

void AppModule::addPropertiesProvider(std::unique_ptr<DocumentTreeNodePropertiesProvider> ptr)
{
    d->m_vecDocTreeNodePropsProvider.push_back(std::move(ptr));
}

const DocumentTreeNodePropertiesProvider* AppModule::findPropertiesProvider(const DocumentTreeNode& treeNode) const
{
    for (const auto& provider : d->m_vecDocTreeNodePropsProvider) {
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

} // namespace Mayo

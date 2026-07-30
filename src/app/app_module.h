/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "document_tree_node_properties_providers.h"
#include "library_info.h"
#include "qstring_utils.h"

#include "../base/application.h"
#include "../base/messenger.h"

#include <locale>

namespace Mayo {

class AppModuleProperties;
class Enumeration;
class Settings;

namespace IO {
class System;
class ParametersProvider;
}

// Provides the root application object as a singleton
// Implements also the behavior specific to the application
class AppModule : public Messenger
{
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::AppModule)
public:
    virtual ~AppModule();

    // Query singleton instance
    static AppModule* get();

    // Application object
    const ApplicationPtr& application() const;

    // Settings
    const AppModuleProperties* properties() const;
    AppModuleProperties* properties();
    Settings* settings();
    const Settings* settings() const;

    // Predicate suitable to Settings::loadFrom() and Settings::saveAs()
    static bool excludeSettingPredicate(const Property& prop);

    // Text options corresponding to the active locale/units config
    QStringUtils::TextOptions defaultTextOptions() const;

    // Current locale used by the application
    const std::locale& stdLocale() const;
    const QLocale& qtLocale() const;

    // Available supported languages
    static const Enumeration& languages();

    // Short-name of the current language in use(eg. en=english)
    QString languageCode() const;

    // Information about 3rd-party libraries used by the application
    void addLibraryInfo(const LibraryInfo& lib);
    void addLibraryInfo(std::string_view libName, std::string_view version, std::string_view versionDetails = {});
    gsl::span<const LibraryInfo> libraryInfoArray() const;

    // Logging
    void clearMessageLog();
    gsl::span<const Messenger::Message> messageLog() const;
    Signal<const Messenger::Message&> signalMessage;
    Signal<> signalMessageLogCleared;

    // Providers to query document tree node properties
    void addPropertiesProvider(std::unique_ptr<DocumentTreeNodePropertiesProvider> ptr);
    const DocumentTreeNodePropertiesProvider* findPropertiesProvider(const DocumentTreeNode& treeNode) const;
    std::unique_ptr<PropertyGroup> properties(const DocumentTreeNode& treeNode) const;

    // IO objects
    const IO::ParametersProvider* ioParametersProvider() const;
    const IO::System* ioSystem() const;
    IO::System* ioSystem();

    // -- from Messenger
    void emitMessage(MessageType msgType, std::string_view text) override;

private:
    AppModule();
    AppModule(const AppModule&) = delete; // Not copyable
    AppModule& operator=(const AppModule&) = delete; // Not copyable

    struct Private;
    Private* const d = nullptr;
};

} // namespace Mayo

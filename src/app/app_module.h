/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "app_module_properties.h"
#include "qstring_utils.h"

#include "../base/application.h"
#include "../base/document_tree_node_properties_provider.h"
#include "../base/io_parameters_provider.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_brep_mesh_parameters.h"
#include "../base/property_value_conversion.h"
#include "../base/settings.h"
#include "../base/unit_system.h"

#include <QtCore/QSize>

#include <locale>
#include <mutex>

class QDataStream;
class TDF_Label;
class TopoDS_Shape;

namespace Mayo {

class GuiApplication;
class GuiDocument;
class TaskProgress;

// Provides the root application object as a singleton
// Implements also the behavior specific to the application
class AppModule :
        public IO::ParametersProvider,
        public PropertyValueConversion,
        public Messenger
{
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::AppModule)
public:
    // Loggable message
    struct Message {
        MessageType type;
        QString text;
    };

    // Query singleton instance
    static AppModule* get();

    ~AppModule();

    // Application object
    const ApplicationPtr& application() const { return m_application; }

    // Settings
    const AppModuleProperties* properties() const { return &m_props; }
    AppModuleProperties* properties() { return &m_props; }
    Settings* settings() const { return m_settings; }

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

    // Logging
    void clearMessageLog();
    Span<const Message> messageLog() const { return m_messageLog; }
    Signal<MessageType, QString> signalMessage;
    Signal<> signalMessageLogCleared;

    // Recent files
    void prependRecentFile(const FilePath& fp, GuiDocument* guiDoc = nullptr);
    const RecentFile* findRecentFile(const FilePath& fp) const;
    void recordRecentFile(GuiDocument* guiDoc);
    void recordRecentFiles(GuiApplication* guiApp);
    QSize recentFileThumbnailSize() const { return { 190, 150 }; }
    void setRecentFileThumbnailRecorder(std::function<Thumbnail(GuiDocument*, QSize)> fn);
    static void read(QDataStream& stream, RecentFiles* recentFiles);
    static void write(QDataStream& stream, const RecentFiles& recentFiles);

    // Recent scripts
    void prependRecentScript(const FilePath& fp);
    const RecentScript* findRecentScript(const FilePath& fp) const;
    static void read(QDataStream& stream, RecentScripts* recentScripts);
    static void write(QDataStream& stream, const RecentScripts& recentScripts);

    // Meshing of BRep shapes
    OccBRepMeshParameters brepMeshParameters(const TopoDS_Shape& shape) const;
    void computeBRepMesh(const TopoDS_Shape& shape, TaskProgress* progress = nullptr);
    void computeBRepMesh(const TDF_Label& labelEntity, TaskProgress* progress = nullptr);

    // Providers to query document tree node properties
    void addPropertiesProvider(std::unique_ptr<DocumentTreeNodePropertiesProvider> ptr);
    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const;

    // IO::System object
    const IO::System* ioSystem() const { return &m_ioSystem; }
    IO::System* ioSystem() { return &m_ioSystem; }

    // -- from IO::ParametersProvider
    const PropertyGroup* findReaderParameters(IO::Format format) const override;
    const PropertyGroup* findWriterParameters(IO::Format format) const override;

    // -- from PropertyValueConversion
    Settings::Variant toVariant(const Property& prop) const override;
    bool fromVariant(Property* prop, const Settings::Variant& variant) const override;

    // -- from Messenger
    void emitMessage(MessageType msgType, std::string_view text) override;

private:
    AppModule();
    AppModule(const AppModule&) = delete; // Not copyable
    AppModule& operator=(const AppModule&) = delete; // Not copyable

    bool impl_recordRecentFile(RecentFile* recentFile, GuiDocument* guiDoc);

    ApplicationPtr m_application;
    Settings* m_settings = nullptr;
    IO::System m_ioSystem;
    AppModuleProperties m_props;
    std::vector<Message> m_messageLog;
    std::mutex m_mutexMessageLog;
    std::locale m_stdLocale;
    QLocale m_qtLocale;
    std::vector<std::unique_ptr<DocumentTreeNodePropertiesProvider>> m_vecDocTreeNodePropsProvider;
    std::function<Thumbnail(GuiDocument*, QSize)> m_fnRecentFileThumbnailRecorder;
};

} // namespace Mayo

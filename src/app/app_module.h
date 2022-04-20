/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "app_module_properties.h"
#include "qstring_utils.h"

#include "../base/document_tree_node_properties_provider.h"
#include "../base/io_parameters_provider.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/occ_brep_mesh_parameters.h"
#include "../base/property_value_conversion.h"
#include "../base/settings.h"
#include "../base/unit_system.h"

#include <QtCore/QObject>
#include <mutex>

class TDF_Label;
class TopoDS_Shape;

namespace Mayo {

class GuiApplication;
class GuiDocument;
class TaskProgress;

class AppModule :
        public QObject,
        public IO::ParametersProvider,
        public PropertyValueConversion,
        public Messenger
{
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::AppModule)
public:
    struct Message {
        MessageType type;
        QString text;
    };

    static AppModule* get();

    // Settings
    const AppModuleProperties* properties() const { return &m_props; }
    AppModuleProperties* properties() { return &m_props; }

    Settings* settings() const { return m_settings; }
    static bool excludeSettingPredicate(const Property& prop);

    QStringUtils::TextOptions defaultTextOptions() const;
    const QLocale& locale() const;

    static const Enumeration& languages();
    QString languageCode() const;

    // Logging
    void clearMessageLog();
    Span<const Message> messageLog() const { return m_messageLog; }
    Q_SIGNAL void message(Messenger::MessageType msgType, const QString& text);
    Q_SIGNAL void messageLogCleared();

    // Recent files
    void prependRecentFile(const FilePath& fp);
    const RecentFile* findRecentFile(const FilePath& fp) const;
    void recordRecentFileThumbnail(GuiDocument* guiDoc);
    void recordRecentFileThumbnails(GuiApplication* guiApp);
    QSize recentFileThumbnailSize() const { return { 190, 150 }; }

    // Meshing of BRep shapes
    OccBRepMeshParameters brepMeshParameters(const TopoDS_Shape& shape) const;
    void computeBRepMesh(const TopoDS_Shape& shape, TaskProgress* progress = nullptr);
    void computeBRepMesh(const TDF_Label& labelEntity, TaskProgress* progress = nullptr);

    // DocumentTreeNodePropertiesProviderTable object
    const DocumentTreeNodePropertiesProviderTable* documentTreeNodePropertiesProviderTable() const;
    DocumentTreeNodePropertiesProviderTable* documentTreeNodePropertiesProviderTable();

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

    Settings* m_settings = nullptr;
    IO::System m_ioSystem;
    AppModuleProperties m_props;
    DocumentTreeNodePropertiesProviderTable m_docTreeNodePropsProviderTable;
    std::vector<Message> m_messageLog;
    std::mutex m_mutexMessageLog;
    QLocale m_locale;
};

} // namespace Mayo

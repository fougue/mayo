/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "recent_files.h"

#include "../base/application_ptr.h"
#include "../base/io_parameters_provider.h"
#include "../base/messenger.h"
#include "../base/occ_brep_mesh_parameters.h"
#include "../base/occt_enums.h"
#include "../base/property.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/property_value_conversion.h"
#include "../base/settings.h"
#include "../base/settings_index.h"
#include "../base/unit_system.h"
#include "qtcore_hfuncs.h"
#include "qstring_utils.h"

#include <QtCore/QObject>
#include <mutex>
#include <unordered_map>
#include <vector>

class TDF_Label;
class TopoDS_Shape;

namespace Mayo {

class GuiApplication;
class GuiDocument;
class TaskProgress;

class AppModule :
        public QObject,
        public PropertyGroup,
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

    AppModule(Application* app);
    static AppModule* get(const ApplicationPtr& app);

    QStringUtils::TextOptions defaultTextOptions() const;

    static QString qmFilePath(const QByteArray& languageCode);
    static QByteArray languageCode(const ApplicationPtr& app);

    static bool excludeSettingPredicate(const Property& prop);

    void prependRecentFile(const FilePath& fp);
    const RecentFile* findRecentFile(const FilePath& fp) const;
    void recordRecentFileThumbnail(GuiDocument* guiDoc);
    void recordRecentFileThumbnails(GuiApplication* guiApp);
    QSize recentFileThumbnailSize() const { return { 190, 150 }; }

    OccBRepMeshParameters brepMeshParameters(const TopoDS_Shape& shape) const;
    void computeBRepMesh(const TopoDS_Shape& shape, TaskProgress* progress = nullptr);
    void computeBRepMesh(const TDF_Label& labelEntity, TaskProgress* progress = nullptr);

    // from IO::ParametersProvider
    const PropertyGroup* findReaderParameters(IO::Format format) const override;
    const PropertyGroup* findWriterParameters(IO::Format format) const override;

    // from PropertyValueConversion
    Settings::Variant toVariant(const Property& prop) const override;
    bool fromVariant(Property* prop, const Settings::Variant& variant) const override;

    // from Messenger
    void emitMessage(MessageType msgType, std::string_view text) override;
    void clearMessageLog();
    Span<const Message> messageLog() const { return m_messageLog; }
    Q_SIGNAL void message(Messenger::MessageType msgType, const QString& text);
    Q_SIGNAL void messageLogCleared();

    // System
    const Settings_GroupIndex groupId_system;
    const Settings_SectionIndex sectionId_systemUnits;
    PropertyInt unitSystemDecimals{ this, textId("decimalCount") };
    PropertyEnum<UnitSystem::Schema> unitSystemSchema{ this, textId("schema") };
    // Application
    const Settings_GroupIndex groupId_application;
    PropertyEnumeration language;
    PropertyRecentFiles recentFiles{ this, textId("recentFiles") };
    PropertyFilePath lastOpenDir{ this, textId("lastOpenFolder") };
    PropertyString lastSelectedFormatFilter{ this, textId("lastSelectedFormatFilter") };
    PropertyBool linkWithDocumentSelector{ this, textId("linkWithDocumentSelector") };
    // Meshing
    const Settings_GroupIndex groupId_meshing;
    enum class BRepMeshQuality { VeryCoarse, Coarse, Normal, Precise, VeryPrecise, UserDefined };
    PropertyEnum<BRepMeshQuality> meshingQuality{ this, textId("meshingQuality") };
    PropertyLength meshingChordalDeflection{ this, textId("meshingChordalDeflection") };
    PropertyAngle meshingAngularDeflection{ this, textId("meshingAngularDeflection") };
    PropertyBool meshingRelative{ this, textId("meshingRelative") };
    // Graphics
    const Settings_GroupIndex groupId_graphics;
    PropertyBool defaultShowOriginTrihedron{ this, textId("defaultShowOriginTrihedron") };
    PropertyDouble instantZoomFactor{ this, textId("instantZoomFactor") };
    // -- ClipPlanes
    const Settings_SectionIndex sectionId_graphicsClipPlanes;
    PropertyBool clipPlanesCappingOn{ this, textId("cappingOn") };
    PropertyBool clipPlanesCappingHatchOn{ this, textId("cappingHatchOn") };
    // -- MeshDefaults
    const Settings_SectionIndex sectionId_graphicsMeshDefaults;
    PropertyOccColor meshDefaultsColor{ this, textId("color") };
    PropertyOccColor meshDefaultsEdgeColor{ this, textId("edgeColor") };
    PropertyEnumeration meshDefaultsMaterial{ this, textId("material"), OcctEnums::Graphic3d_NameOfMaterial() };
    PropertyBool meshDefaultsShowEdges{ this, textId("showEgesOn") };
    PropertyBool meshDefaultsShowNodes{ this, textId("showNodesOn") };

protected:
    // from PropertyGroup
    void onPropertyChanged(Property* prop) override;

private:
    Application* m_app = nullptr;
    std::vector<std::unique_ptr<PropertyGroup>> m_vecPtrPropertyGroup;
    std::unordered_map<IO::Format, PropertyGroup*> m_mapFormatReaderParameters;
    std::unordered_map<IO::Format, PropertyGroup*> m_mapFormatWriterParameters;
    std::vector<Message> m_messageLog;
    std::mutex m_mutexMessageLog;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "recent_files.h"

#include "../base/io_format.h"
#include "../base/occt_enums.h"
#include "../base/property.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/settings.h"
#include "../base/unit_system.h"

#include <unordered_map>
#include <vector>

namespace Mayo {

namespace IO { class System; }

class Settings;

class AppModuleProperties : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::AppModuleProperties)
public:
    AppModuleProperties(Settings* settings);

    void IO_bindParameters(const IO::System* ioSystem);

    void retranslate();

    // System
    const Settings::GroupIndex groupId_system;
    PropertyInt unitSystemDecimals{ this, textId("decimalCount") };
    PropertyEnum<UnitSystem::Schema> unitSystemSchema{ this, textId("schema") };
    // Application
    const Settings::GroupIndex groupId_application;
    PropertyEnumeration language;
    PropertyRecentFiles recentFiles{ this, textId("recentFiles") };
    PropertyFilePath lastOpenDir{ this, textId("lastOpenFolder") };
    PropertyString lastSelectedFormatFilter{ this, textId("lastSelectedFormatFilter") };
    PropertyBool linkWithDocumentSelector{ this, textId("linkWithDocumentSelector") };
    // Meshing
    enum class BRepMeshQuality { VeryCoarse, Coarse, Normal, Precise, VeryPrecise, UserDefined };
    PropertyEnum<BRepMeshQuality> meshingQuality{ this, textId("meshingQuality") };
    PropertyLength meshingChordalDeflection{ this, textId("meshingChordalDeflection") };
    PropertyAngle meshingAngularDeflection{ this, textId("meshingAngularDeflection") };
    PropertyBool meshingRelative{ this, textId("meshingRelative") };
    // Graphics
    PropertyBool defaultShowOriginTrihedron{ this, textId("defaultShowOriginTrihedron") };
    PropertyDouble instantZoomFactor{ this, textId("instantZoomFactor") };
    // -- Graphics/ClipPlanes
    PropertyBool clipPlanesCappingOn{ this, textId("cappingOn") };
    PropertyBool clipPlanesCappingHatchOn{ this, textId("cappingHatchOn") };
    // -- Graphics/MeshDefaults
    PropertyOccColor meshDefaultsColor{ this, textId("color") };
    PropertyOccColor meshDefaultsEdgeColor{ this, textId("edgeColor") };
    PropertyEnumeration meshDefaultsMaterial{ this, textId("material"), &OcctEnums::Graphic3d_NameOfMaterial() };
    PropertyBool meshDefaultsShowEdges{ this, textId("showEgesOn") };
    PropertyBool meshDefaultsShowNodes{ this, textId("showNodesOn") };

protected:
    // -- from PropertyGroup
    void onPropertyChanged(Property* prop) override;

private:
    friend class AppModule;
    Settings* m_settings = nullptr;
    std::vector<std::unique_ptr<PropertyGroup>> m_vecPtrPropertyGroup;
    std::unordered_map<IO::Format, PropertyGroup*> m_mapFormatReaderParameters;
    std::unordered_map<IO::Format, PropertyGroup*> m_mapFormatWriterParameters;
};

} // namespace Mayo

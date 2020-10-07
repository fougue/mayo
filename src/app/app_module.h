/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/io_parameters_provider.h"
#include "../base/property.h"
#include "../base/property_builtins.h"
#include "../base/property_enumeration.h"
#include "../base/settings_index.h"
#include "../base/string_utils.h"

#include <fougtools/qttools/core/qbytearray_hfunc.h>
#include <QtCore/QObject>
#include <unordered_map>
#include <vector>

namespace Mayo {

class AppModule :
        public QObject,
        public PropertyGroup,
        public IO::ParametersProvider
{
    Q_OBJECT
public:
    AppModule(Application* app);
    static AppModule* get(ApplicationPtr app);

    StringUtils::TextOptions defaultTextOptions() const;

    static QString qmFilePath(const QByteArray& languageCode);

    const PropertyGroup* findReaderParameters(const IO::Format& format) const override;
    const PropertyGroup* findWriterParameters(const IO::Format& format) const override;

    // System
    const Settings_GroupIndex groupId_system;
    const Settings_SectionIndex sectionId_systemUnits;
    PropertyInt unitSystemDecimals;
    PropertyEnumeration unitSystemSchema;
    // Application
    const Settings_GroupIndex groupId_application;
    PropertyEnumeration language;
    PropertyQStringList recentFiles;
    PropertyQString lastOpenDir;
    PropertyQString lastSelectedFormatFilter;
    PropertyBool linkWithDocumentSelector;
    // Graphics
    const Settings_GroupIndex groupId_graphics;
    PropertyBool defaultShowOriginTrihedron;
    // -- ClipPlanes
    const Settings_SectionIndex sectionId_graphicsClipPlanes;
    PropertyBool clipPlanesCappingOn;
    PropertyEnumeration clipPlanesCappingHatch;
    // -- MeshDefaults
    const Settings_SectionIndex sectionId_graphicsMeshDefaults;
    PropertyOccColor meshDefaultsColor;
    PropertyEnumeration meshDefaultsMaterial;
    PropertyBool meshDefaultsShowEdges;
    PropertyBool meshDefaultsShowNodes;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    Application* m_app = nullptr;
    std::vector<std::unique_ptr<PropertyGroup>> m_vecPtrPropertyGroup;
    std::unordered_map<QByteArray, PropertyGroup*> m_mapFormatReaderParameters;
    std::unordered_map<QByteArray, PropertyGroup*> m_mapFormatWriterParameters;
};

} // namespace Mayo

/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module.h"

#include "../base/application.h"
#include "../base/occt_enums.h"
#include "../base/settings.h"
#include "../graphics/graphics_entity_driver.h"

namespace Mayo {

static const Enumeration& enumUnitSchemas()
{
    static const Enumeration enumUnitSchema = {
        { UnitSystem::SI, MAYO_TEXT_ID("Mayo::AppModule", "SI") },
        { UnitSystem::ImperialUK, MAYO_TEXT_ID("Mayo::AppModule", "IMPERIAL_UK") }
    };
    return enumUnitSchema;
}

AppModule::AppModule(Application* app)
    : QObject(app),
      PropertyGroup(app->settings()),
      m_app(app),
      // System
      groupId_system(app->settings()->addGroup(MAYO_TEXT_ID("Mayo::AppModule", "system"))),
      // -- Units
      sectionId_systemUnits(
          app->settings()->addSection(this->groupId_system, MAYO_TEXT_ID("Mayo::AppModule", "units"))),
      unitSystemDecimals(app->settings(), MAYO_TEXT_ID("Mayo::AppModule", "decimalCount")),
      unitSystemSchema(app->settings(), MAYO_TEXT_ID("Mayo::AppModule", "schema"), &enumUnitSchemas()),
      // Application
      groupId_application(app->settings()->addGroup(MAYO_TEXT_ID("Mayo::AppModule", "application"))),
      recentFiles(this, MAYO_TEXT_ID("Mayo::AppModule", "recentFiles")),
      lastOpenDir(this, MAYO_TEXT_ID("Mayo::AppModule", "lastOpenFolder")),
      lastSelectedFormatFilter(this, MAYO_TEXT_ID("Mayo::AppModule", "lastSelectedFormatFilter")),
      linkWithDocumentSelector(this, MAYO_TEXT_ID("Mayo::AppModule", "linkWithDocumentSelector")),
      // Graphics
      groupId_graphics(app->settings()->addGroup(MAYO_TEXT_ID("Mayo::AppModule", "graphics"))),
      defaultShowOriginTrihedron(this, MAYO_TEXT_ID("Mayo::AppModule", "defaultShowOriginTrihedron")),
      // -- Clip planes
      sectionId_graphicsClipPlanes(
          app->settings()->addSection(this->groupId_graphics, MAYO_TEXT_ID("Mayo::AppModule", "clipPlanes"))),
      clipPlanesCappingOn(this, MAYO_TEXT_ID("Mayo::AppModule", "cappingOn")),
      clipPlanesCappingHatch(this, MAYO_TEXT_ID("Mayo::AppModule", "cappingHatch"), &OcctEnums::Aspect_HatchStyle()),
      // -- Mesh defaults
      sectionId_graphicsMeshDefaults(
          app->settings()->addSection(this->groupId_graphics, MAYO_TEXT_ID("Mayo::AppModule", "meshDefaults"))),
      meshDefaultsColor(this, MAYO_TEXT_ID("Mayo::AppModule", "color")),
      meshDefaultsMaterial(this, MAYO_TEXT_ID("Mayo::AppModule", "material"), &OcctEnums::Graphic3d_NameOfMaterial()),
      meshDefaultsShowEdges(this, MAYO_TEXT_ID("Mayo::AppModule", "showEgesOn")),
      meshDefaultsShowNodes(this, MAYO_TEXT_ID("Mayo::AppModule", "showNodesOn"))
{
    auto settings = app->settings();

    // System
    // -- Units
    settings->addSetting(&this->unitSystemSchema, this->sectionId_systemUnits);
    settings->addSetting(&this->unitSystemDecimals, this->sectionId_systemUnits);
    this->unitSystemDecimals.setRange(1, 99);
    this->unitSystemDecimals.setSingleStep(1);
    this->unitSystemDecimals.setConstraintsEnabled(true);

    // Application
    settings->addSetting(&this->recentFiles, this->groupId_application);
    settings->addSetting(&this->lastOpenDir, this->groupId_application);
    settings->addSetting(&this->lastSelectedFormatFilter, this->groupId_application);
    settings->addSetting(&this->linkWithDocumentSelector, this->groupId_application);

    // Graphics
    settings->addSetting(&this->defaultShowOriginTrihedron, this->groupId_graphics);
    // -- Clip planes
    settings->addSetting(&this->clipPlanesCappingOn, this->sectionId_graphicsClipPlanes);
    settings->addSetting(&this->clipPlanesCappingHatch, this->sectionId_graphicsClipPlanes);
    // -- Mesh defaults
    settings->addSetting(&this->meshDefaultsColor, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsMaterial, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowEdges, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowNodes, this->sectionId_graphicsMeshDefaults);
    // Import
    auto groupId_Import = settings->addGroup(MAYO_TEXT_ID("Mayo::AppModule", "Import"));
    for (const IO::Format& format : app->ioSystem()->readerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Import, format.identifier);
        const IO::FactoryReader* factory = app->ioSystem()->findFactoryReader(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createParameters(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
            m_mapFormatReaderParameters.insert({ format.identifier, ptrGroup.get() });
        }
    }

    // Export
    auto groupId_Export = settings->addGroup(MAYO_TEXT_ID("Mayo::AppModule", "Export"));
    for (const IO::Format& format : app->ioSystem()->writerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Export, format.identifier);
        const IO::FactoryWriter* factory = app->ioSystem()->findFactoryWriter(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createParameters(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
            m_mapFormatWriterParameters.insert({ format.identifier, ptrGroup.get() });
        }
    }

    // Register reset functions
    settings->addGroupResetFunction(this->groupId_system, [&]{
        this->unitSystemDecimals.setValue(2);
        this->unitSystemSchema.setValue(UnitSystem::SI);
    });
    settings->addGroupResetFunction(this->groupId_application, [&]{
        this->recentFiles.setValue(QStringList());
        this->lastOpenDir.setValue(QString());
        this->lastSelectedFormatFilter.setValue(QString());
        this->linkWithDocumentSelector.setValue(true);
    });
    settings->addGroupResetFunction(this->groupId_graphics, [&]{
        this->defaultShowOriginTrihedron.setValue(true);
        this->clipPlanesCappingOn.setValue(true);
        this->clipPlanesCappingHatch.setValue(Aspect_HS_SOLID);
        const GraphicsMeshEntityDriver::DefaultValues meshDefaults;
        this->meshDefaultsColor.setValue(meshDefaults.color);
        this->meshDefaultsMaterial.setValue(meshDefaults.material);
        this->meshDefaultsShowEdges.setValue(meshDefaults.showEdges);
        this->meshDefaultsShowNodes.setValue(meshDefaults.showNodes);
    });
}

StringUtils::TextOptions AppModule::defaultTextOptions() const
{
    StringUtils::TextOptions opts;
    opts.locale = m_app->settings()->locale();
    opts.unitDecimals = this->unitSystemDecimals.value();
    opts.unitSchema = this->unitSystemSchema.valueAs<UnitSystem::Schema>();
    return opts;
}

const PropertyGroup* AppModule::findReaderParameters(const IO::Format& format) const
{
    auto it = m_mapFormatReaderParameters.find(format.identifier);
    return it != m_mapFormatReaderParameters.cend() ? it->second : nullptr;
}

const PropertyGroup *AppModule::findWriterParameters(const IO::Format& format) const
{
    auto it = m_mapFormatWriterParameters.find(format.identifier);
    return it != m_mapFormatWriterParameters.cend() ? it->second : nullptr;
}

AppModule* AppModule::get(ApplicationPtr app)
{
    if (app)
        return app->findChild<AppModule*>(QString(), Qt::FindDirectChildrenOnly);

    return nullptr;
}

void AppModule::onPropertyChanged(Property *prop)
{
    if (prop == &this->meshDefaultsColor
            || prop == &this->meshDefaultsMaterial
            || prop == &this->meshDefaultsShowEdges
            || prop == &this->meshDefaultsShowNodes)
    {
        auto values = GraphicsMeshEntityDriver::defaultValues();
        values.color = this->meshDefaultsColor.value();
        values.material = this->meshDefaultsMaterial.valueAs<Graphic3d_NameOfMaterial>();
        values.showEdges = this->meshDefaultsShowEdges.value();
        values.showNodes = this->meshDefaultsShowNodes.value();
        GraphicsMeshEntityDriver::setDefaultValues(values);
    }

    PropertyGroup::onPropertyChanged(prop);
}

} // namespace Mayo

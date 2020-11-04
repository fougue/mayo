/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module.h"

#include "../base/application.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/occt_enums.h"
#include "../base/settings.h"
#include "../graphics/graphics_entity_driver.h"

namespace Mayo {

static const Enumeration& enumUnitSchemas()
{
    static const Enumeration enumeration = {
        { UnitSystem::SI, AppModule::textId("SI") },
        { UnitSystem::ImperialUK, AppModule::textId("IMPERIAL_UK") }
    };
    return enumeration;
}

static const Enumeration& enumLanguages()
{
    static Enumeration enumeration = {
        { 0, AppModule::textId("en") },
        { 1, AppModule::textId("fr") }
    };
    return enumeration;
}

AppModule::AppModule(Application* app)
    : QObject(app),
      PropertyGroup(app->settings()),
      m_app(app),
      // System
      groupId_system(app->settings()->addGroup(textId("system"))),
      // -- Units
      sectionId_systemUnits(
          app->settings()->addSection(this->groupId_system, textId("units"))),
      unitSystemDecimals(app->settings(), textId("decimalCount")),
      unitSystemSchema(app->settings(), textId("schema"), &enumUnitSchemas()),
      // Application
      groupId_application(app->settings()->addGroup(textId("application"))),
      language(this, textId("language"), &enumLanguages()),
      recentFiles(this, textId("recentFiles")),
      lastOpenDir(this, textId("lastOpenFolder")),
      lastSelectedFormatFilter(this, textId("lastSelectedFormatFilter")),
      linkWithDocumentSelector(this, textId("linkWithDocumentSelector")),
      // Graphics
      groupId_graphics(app->settings()->addGroup(textId("graphics"))),
      defaultShowOriginTrihedron(this, textId("defaultShowOriginTrihedron")),
      // -- Clip planes
      sectionId_graphicsClipPlanes(
          app->settings()->addSection(this->groupId_graphics, textId("clipPlanes"))),
      clipPlanesCappingOn(this, textId("cappingOn")),
      clipPlanesCappingHatchOn(this, textId("cappingHatchOn")),
      // -- Mesh defaults
      sectionId_graphicsMeshDefaults(
          app->settings()->addSection(this->groupId_graphics, textId("meshDefaults"))),
      meshDefaultsColor(this, textId("color")),
      meshDefaultsMaterial(this, textId("material"), &OcctEnums::Graphic3d_NameOfMaterial()),
      meshDefaultsShowEdges(this, textId("showEgesOn")),
      meshDefaultsShowNodes(this, textId("showNodesOn"))
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
    settings->addSetting(&this->language, this->groupId_application);
    settings->addSetting(&this->recentFiles, this->groupId_application);
    settings->addSetting(&this->lastOpenDir, this->groupId_application);
    settings->addSetting(&this->lastSelectedFormatFilter, this->groupId_application);
    settings->addSetting(&this->linkWithDocumentSelector, this->groupId_application);
    this->recentFiles.setUserVisible(false);
    this->lastOpenDir.setUserVisible(false);
    this->lastSelectedFormatFilter.setUserVisible(false);

    // Graphics
    settings->addSetting(&this->defaultShowOriginTrihedron, this->groupId_graphics);
    // -- Clip planes
    settings->addSetting(&this->clipPlanesCappingOn, this->sectionId_graphicsClipPlanes);
    settings->addSetting(&this->clipPlanesCappingHatchOn, this->sectionId_graphicsClipPlanes);
    // -- Mesh defaults
    settings->addSetting(&this->meshDefaultsColor, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsMaterial, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowEdges, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowNodes, this->sectionId_graphicsMeshDefaults);
    // Import
    auto groupId_Import = settings->addGroup(textId("import"));
    for (const IO::Format& format : app->ioSystem()->readerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Import, format.identifier);
        const IO::FactoryReader* factory = app->ioSystem()->findFactoryReader(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            settings->addGroupResetFunction(groupId_Import, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatReaderParameters.insert({ format.identifier, rawPtrGroup });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }

    // Export
    auto groupId_Export = settings->addGroup(textId("export"));
    for (const IO::Format& format : app->ioSystem()->writerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Export, format.identifier);
        const IO::FactoryWriter* factory = app->ioSystem()->findFactoryWriter(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            settings->addGroupResetFunction(groupId_Export, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatWriterParameters.insert({ format.identifier, ptrGroup.get() });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }

    // Register reset functions
    settings->addGroupResetFunction(this->groupId_system, [&]{
        this->unitSystemDecimals.setValue(2);
        this->unitSystemSchema.setValue(UnitSystem::SI);
    });
    settings->addGroupResetFunction(this->groupId_application, [&]{
        this->language.setValue(enumLanguages().findValue("en"));
        this->recentFiles.setValue(QStringList());
        this->lastOpenDir.setValue(QString());
        this->lastSelectedFormatFilter.setValue(QString());
        this->linkWithDocumentSelector.setValue(true);
    });
    settings->addGroupResetFunction(this->groupId_graphics, [&]{
        this->defaultShowOriginTrihedron.setValue(true);
        this->clipPlanesCappingOn.setValue(true);
        this->clipPlanesCappingHatchOn.setValue(true);
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

QString AppModule::qmFilePath(const QByteArray& languageCode)
{
    return QString(":/i18n/mayo_%1.qm").arg(QString::fromUtf8(languageCode));
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

AppModule* AppModule::get(const ApplicationPtr& app)
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

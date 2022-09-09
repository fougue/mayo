/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module_properties.h"
#include "app_module.h"

#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../base/unit_system.h"
#include "../graphics/graphics_mesh_object_driver.h"

namespace Mayo {

AppModuleProperties::AppModuleProperties(Settings* settings)
    : PropertyGroup(settings),
      groupId_system(settings->addGroup(textId("system"))),
      groupId_application(settings->addGroup(textId("application"))),
      language(this, textId("language"), &AppModule::languages()),
      m_settings(settings)
{
    const auto groupId_meshing = settings->addGroup(textId("meshing"));
    const auto groupId_graphics = settings->addGroup(textId("graphics"));

    const auto sectionId_systemUnits = settings->addSection(this->groupId_system, textId("units"));
    const auto sectionId_graphicsClipPlanes = settings->addSection(groupId_graphics, textId("clipPlanes"));
    const auto sectionId_graphicsMeshDefaults = settings->addSection(groupId_graphics, textId("meshDefaults"));

    this->retranslate();

    // System
    // -- Units
    settings->addSetting(&this->unitSystemSchema, sectionId_systemUnits);
    settings->addSetting(&this->unitSystemDecimals, sectionId_systemUnits);
    this->unitSystemDecimals.setRange(1, 99);
    this->unitSystemDecimals.setSingleStep(1);
    this->unitSystemDecimals.setConstraintsEnabled(true);

    // Application
    settings->addSetting(&this->language, groupId_application);
    settings->addSetting(&this->recentFiles, groupId_application);
    settings->addSetting(&this->lastOpenDir, groupId_application);
    settings->addSetting(&this->lastSelectedFormatFilter, groupId_application);
    settings->addSetting(&this->linkWithDocumentSelector, groupId_application);
    this->recentFiles.setUserVisible(false);
    this->lastOpenDir.setUserVisible(false);
    this->lastSelectedFormatFilter.setUserVisible(false);

    // Meshing
    this->meshingQuality.mutableEnumeration().changeTrContext(AppModuleProperties::textIdContext());
    settings->addSetting(&this->meshingQuality, groupId_meshing);
    settings->addSetting(&this->meshingChordalDeflection, groupId_meshing);
    settings->addSetting(&this->meshingAngularDeflection, groupId_meshing);
    settings->addSetting(&this->meshingRelative, groupId_meshing);

    // Graphics
    settings->addSetting(&this->navigationStyle, groupId_graphics);
    settings->addSetting(&this->defaultShowOriginTrihedron, groupId_graphics);
    settings->addSetting(&this->instantZoomFactor, groupId_graphics);
    // -- Clip planes
    settings->addSetting(&this->clipPlanesCappingOn, sectionId_graphicsClipPlanes);
    settings->addSetting(&this->clipPlanesCappingHatchOn, sectionId_graphicsClipPlanes);
    // -- Mesh defaults
    settings->addSetting(&this->meshDefaultsColor, sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsEdgeColor, sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsMaterial, sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowEdges, sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowNodes, sectionId_graphicsMeshDefaults);

    // Register reset functions
    settings->addResetFunction(sectionId_systemUnits, [=]{
        this->unitSystemDecimals.setValue(2);
        this->unitSystemSchema.setValue(UnitSystem::SI);
    });
    settings->addResetFunction(groupId_application, [&]{
        this->language.setValue(AppModule::languages().findValueByName("en"));
        this->recentFiles.setValue({});
        this->lastOpenDir.setValue({});
        this->lastSelectedFormatFilter.setValue({});
        this->linkWithDocumentSelector.setValue(true);
    });
    settings->addResetFunction(groupId_graphics, [=]{
        this->navigationStyle.setValue(WidgetOccViewController::NavigationStyle::Mayo);
        this->defaultShowOriginTrihedron.setValue(true);
        this->instantZoomFactor.setValue(5.);
    });
    settings->addResetFunction(groupId_meshing, [&]{
        this->meshingQuality.setValue(BRepMeshQuality::Normal);
        this->meshingChordalDeflection.setQuantity(1 * Quantity_Millimeter);
        this->meshingAngularDeflection.setQuantity(20 * Quantity_Degree);
        this->meshingRelative.setValue(false);
    });
    settings->addResetFunction(sectionId_graphicsClipPlanes, [=]{
        this->clipPlanesCappingOn.setValue(true);
        this->clipPlanesCappingHatchOn.setValue(true);
    });
    settings->addResetFunction(sectionId_graphicsMeshDefaults, [=]{
        const GraphicsMeshObjectDriver::DefaultValues meshDefaults;
        this->meshDefaultsColor.setValue(meshDefaults.color);
        this->meshDefaultsEdgeColor.setValue(meshDefaults.edgeColor);
        this->meshDefaultsMaterial.setValue(meshDefaults.material);
        this->meshDefaultsShowEdges.setValue(meshDefaults.showEdges);
        this->meshDefaultsShowNodes.setValue(meshDefaults.showNodes);
    });
}

void AppModuleProperties::IO_bindParameters(const IO::System* ioSystem)
{
    // Import
    const auto groupId_Import = m_settings->addGroup(textId("import"));
    for (IO::Format format : ioSystem->readerFormats()) {
        auto sectionId_format = m_settings->addSection(groupId_Import, IO::formatIdentifier(format));
        const IO::FactoryReader* factory = ioSystem->findFactoryReader(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, m_settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                m_settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            m_settings->addResetFunction(sectionId_format, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatReaderParameters.insert({ format, rawPtrGroup });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }

    // Export
    const auto groupId_Export = m_settings->addGroup(textId("export"));
    for (IO::Format format : ioSystem->writerFormats()) {
        auto sectionId_format = m_settings->addSection(groupId_Export, IO::formatIdentifier(format));
        const IO::FactoryWriter* factory = ioSystem->findFactoryWriter(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, m_settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                m_settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            m_settings->addResetFunction(sectionId_format, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatWriterParameters.insert({ format, ptrGroup.get() });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }
}

void AppModuleProperties::retranslate()
{
    this->language.setDescription(
                textIdTr("Language used for the application. Change will take effect after application restart"));
    this->linkWithDocumentSelector.setDescription(
                textIdTr("In case where multiple documents are opened, make sure the document displayed in "
                         "the 3D view corresponds to what is selected in the model tree"));
    this->meshingQuality.setDescription(
                textIdTr("Controls precision of the mesh to be computed from the BRep shape"));
    this->meshingChordalDeflection.setDescription(
                textIdTr("For the tesselation of faces the chordal deflection limits the distance between "
                         "a curve and its tessellation"));
    this->meshingAngularDeflection.setDescription(
                textIdTr("For the tesselation of faces the angular deflection limits the angle between "
                         "subsequent segments in a polyline"));
    this->meshingRelative.setDescription(
                textIdTr("Relative computation of edge tolerance\n\n"
                         "If activated, deflection used for the polygonalisation of each edge will be "
                         "`ChordalDeflection` &#215; `SizeOfEdge`. The deflection used for the faces will be "
                         "the maximum deflection of their edges."));
    this->navigationStyle.setDescription(
                textIdTr("3D view manipulation shortcuts configuration to mimic other common CAD applications"));
    this->defaultShowOriginTrihedron.setDescription(
                textIdTr("Show or hide by default the trihedron centered at world origin. "
                         "This doesn't affect 3D view of currently opened documents"));
    this->clipPlanesCappingOn.setDescription(
                textIdTr("Enable capping of currently clipped graphics"));
    this->clipPlanesCappingHatchOn.setDescription(
                textIdTr("Enable capping hatch texture of currently clipped graphics"));
}

void AppModuleProperties::onPropertyChanged(Property* prop)
{
    if (prop == &this->meshDefaultsColor
            || prop == &this->meshDefaultsEdgeColor
            || prop == &this->meshDefaultsMaterial
            || prop == &this->meshDefaultsShowEdges
            || prop == &this->meshDefaultsShowNodes)
    {
        auto values = GraphicsMeshObjectDriver::defaultValues();
        values.color = this->meshDefaultsColor.value();
        values.edgeColor = this->meshDefaultsEdgeColor.value();
        values.material = static_cast<Graphic3d_NameOfMaterial>(this->meshDefaultsMaterial.value());
        values.showEdges = this->meshDefaultsShowEdges.value();
        values.showNodes = this->meshDefaultsShowNodes.value();
        GraphicsMeshObjectDriver::setDefaultValues(values);
    }
    else if (prop == &this->meshingQuality) {
        const bool isUserDefined = this->meshingQuality.value() == BRepMeshQuality::UserDefined;
        this->meshingChordalDeflection.setEnabled(isUserDefined);
        this->meshingAngularDeflection.setEnabled(isUserDefined);
        this->meshingRelative.setEnabled(isUserDefined);
    }

    PropertyGroup::onPropertyChanged(prop);
}

} // namespace Mayo

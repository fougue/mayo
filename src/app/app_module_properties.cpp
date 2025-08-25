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

#include <fmt/format.h>

namespace Mayo {

namespace {

const Enumeration& cornerEnumeration()
{
    static const Enumeration corners = {
        { Aspect_TOTP_LEFT_UPPER, AppModuleProperties::textId("TopLeft") },
        { Aspect_TOTP_RIGHT_UPPER, AppModuleProperties::textId("TopRight") },
        { Aspect_TOTP_LEFT_LOWER, AppModuleProperties::textId("BottomLeft") },
        { Aspect_TOTP_RIGHT_LOWER, AppModuleProperties::textId("BottomRight") }
    };
    return corners;
}

} // namespace

AppModuleProperties::AppModuleProperties(Settings* settings)
    : PropertyGroup(settings),
      groupId_system(settings->addGroup(textId("system"))),
      groupId_application(settings->addGroup(textId("application"))),
      groupId_meshing(settings->addGroup(textId("meshing"))),
      groupId_graphics(settings->addGroup(textId("graphics"))),
      language(this, textId("language"), &AppModule::languages()),
      viewCubeCorner(this, textId("viewCubeCorner"), &cornerEnumeration()),
      m_settings(settings)
{
    const auto sectionId_systemUnits = settings->addSection(groupId_system, textId("units"));
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
    this->actionOnDocumentFileChange.mutableEnumeration().changeTrContext(AppModuleProperties::textIdContext());
    settings->addSetting(&this->language, groupId_application);
    settings->addSetting(&this->recentFiles, groupId_application);
    settings->addSetting(&this->lastOpenDir, groupId_application);
    settings->addSetting(&this->lastSelectedFormatFilter, groupId_application);
    settings->addSetting(&this->actionOnDocumentFileChange, groupId_application);
    settings->addSetting(&this->linkWithDocumentSelector, groupId_application);
    settings->addSetting(&this->forceOpenGlFallbackWidget, groupId_application);
    settings->addSetting(&this->appUiState, groupId_application);
    this->recentFiles.setUserVisible(false);
    this->lastOpenDir.setUserVisible(false);
    this->lastSelectedFormatFilter.setUserVisible(false);
    this->appUiState.setUserVisible(false);

    // Meshing
    this->meshingQuality.mutableEnumeration().changeTrContext(AppModuleProperties::textIdContext());
    settings->addSetting(&this->meshingQuality, groupId_meshing);
    settings->addSetting(&this->meshingChordalDeflection, groupId_meshing);
    settings->addSetting(&this->meshingAngularDeflection, groupId_meshing);
    settings->addSetting(&this->meshingRelative, groupId_meshing);

    // Graphics
    settings->addSetting(&this->navigationStyle, groupId_graphics);
    settings->addSetting(&this->viewCubeCorner, groupId_graphics);
    settings->addSetting(&this->defaultShowOriginTrihedron, groupId_graphics);
    settings->addSetting(&this->instantZoomFactor, groupId_graphics);
    settings->addSetting(&this->turnViewAngleIncrement, groupId_graphics);
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
        this->actionOnDocumentFileChange.setValue(ActionOnDocumentFileChange::None);
        this->linkWithDocumentSelector.setValue(true);
        this->appUiState.setValue({});
#ifndef MAYO_OS_MAC
        this->forceOpenGlFallbackWidget.setValue(false);
#else
        this->forceOpenGlFallbackWidget.setValue(true);
#endif
    });
    settings->addResetFunction(groupId_graphics, [=]{
        this->navigationStyle.setValue(View3dNavigationStyle::Mayo);
        this->viewCubeCorner.setValue(Aspect_TOTP_LEFT_LOWER);
        this->defaultShowOriginTrihedron.setValue(true);
        this->instantZoomFactor.setValue(5.);
        this->turnViewAngleIncrement.setQuantity(5 * Quantity_Degree);
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
    // System
    this->unitSystemSchema.mutableEnumeration().changeTrContext(AppModuleProperties::textIdContext());

    // Application
    this->language.setDescription(
        textIdTr("Language used for the application. Change will take effect after application restart")
    );
    const auto& enumActionOnDocumentFileChange = this->actionOnDocumentFileChange.enumeration();
    this->actionOnDocumentFileChange.setDescription(
        fmt::format(textIdTr("Action to be done after some opened document file is changed(modified) externally\n\n"
                             "Select options `{0}` or `{1}` so the application monitors changes made to opened files\n\n"
                             "When such a change is detected then the application proposes to reload(open again) the document\n\n"
                             "Select `{1}` to automatically reload documents without any user interaction"
                             ),
                    enumActionOnDocumentFileChange.findItemByValue(ActionOnDocumentFileChange::ReloadIfUserConfirm)->name.tr(),
                    enumActionOnDocumentFileChange.findItemByValue(ActionOnDocumentFileChange::ReloadSilently)->name.tr()
    ));
    this->linkWithDocumentSelector.setDescription(
        textIdTr("In case where multiple documents are opened, make sure the document displayed in "
                 "the 3D view corresponds to what is selected in the model tree")
    );
    this->forceOpenGlFallbackWidget.setDescription(
        textIdTr("Force usage of the fallback Qt widget to display OpenGL graphics.\n\n"
                 "When `OFF` the application will try to use OpenGL framebuffer for rendering, "
                 "this allows to display overlay widgets(eg measure tools panel) with translucid "
                 "background. "
                 "However using OpenGL framebuffer might cause troubles for some users(eg empty "
                 "3D window) especially on macOS.\n\n"
                 "When `ON` the application will use a regular Qt widget for rendering which "
                 "proved to be more supported.\n\n"
                 "This option is applicable when OpenCascade ≥ 7.6 version. "
                 "Change will take effect after application restart")
    );

    // Meshing
    this->meshingQuality.setDescription(
        textIdTr("Controls precision of the mesh to be computed from the BRep shape")
    );
    this->meshingChordalDeflection.setDescription(
        textIdTr("For the tessellation of faces the chordal deflection limits the distance between "
                 "a curve and its tessellation")
    );
    this->meshingAngularDeflection.setDescription(
        textIdTr("For the tessellation of faces the angular deflection limits the angle between "
                 "subsequent segments in a polyline")
    );
    this->meshingRelative.setDescription(
        textIdTr("Relative computation of edge tolerance\n\n"
                 "If activated, deflection used for the polygonalisation of each edge will be "
                 "`ChordalDeflection` &#215; `SizeOfEdge`. The deflection used for the faces will be "
                 "the maximum deflection of their edges.")
    );

    // Graphics
    this->navigationStyle.setDescription(
        textIdTr("3D view manipulation shortcuts configuration to mimic other common CAD applications")
    );
    this->turnViewAngleIncrement.setDescription(
        textIdTr("Angle increment used to turn(rotate) the 3D view around the normal of the view plane(Z axis frame reference)")
    );
    this->viewCubeCorner.setDescription(textIdTr("Corner where 3D view cube is located"));

    // -- Graphics/ClipPlanes
    this->defaultShowOriginTrihedron.setDescription(
        textIdTr("Show or hide by default the trihedron centered at world origin. "
                 "This doesn't affect 3D view of currently opened documents")
    );
    this->clipPlanesCappingOn.setDescription(
        textIdTr("Enable capping of currently clipped graphics")
    );
    this->clipPlanesCappingHatchOn.setDescription(
        textIdTr("Enable capping hatch texture of currently clipped graphics")
    );
}

void AppModuleProperties::onPropertyChanged(Property* prop)
{
    if (prop == &this->meshDefaultsColor
        || prop == &this->meshDefaultsEdgeColor
        || prop == &this->meshDefaultsMaterial
        || prop == &this->meshDefaultsShowEdges
        || prop == &this->meshDefaultsShowNodes
       )
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

Aspect_TypeOfTriedronPosition AppModuleProperties::graphicsViewCubeCornerValue() const
{
    return static_cast<Aspect_TypeOfTriedronPosition>(this->viewCubeCorner.value());
}

} // namespace Mayo

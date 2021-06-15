/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_module.h"

#include "../base/application.h"
#include "../base/bnd_utils.h"
#include "../base/brep_utils.h"
#include "../base/io_reader.h"
#include "../base/io_writer.h"
#include "../base/io_system.h"
#include "../base/occt_enums.h"
#include "../base/settings.h"
#include "../graphics/graphics_object_driver.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"

#include <BRepBndLib.hxx>
#include <QtCore/QDir>
#include <QtGui/QGuiApplication>
#include <iterator>

namespace Mayo {

static inline const Enumeration enumLanguages = {
    { 0, AppModule::textId("en") },
    { 1, AppModule::textId("fr") }
};

AppModule::AppModule(Application* app)
    : QObject(app),
      PropertyGroup(app->settings()),
      m_app(app),
      groupId_system(app->settings()->addGroup(textId("system"))),
      sectionId_systemUnits(app->settings()->addSection(this->groupId_system, textId("units"))),
      groupId_application(app->settings()->addGroup(textId("application"))),
      language(this, textId("language"), enumLanguages),
      groupId_meshing(app->settings()->addGroup(textId("meshing"))),
      groupId_graphics(app->settings()->addGroup(textId("graphics"))),
      sectionId_graphicsClipPlanes(
          app->settings()->addSection(this->groupId_graphics, textId("clipPlanes"))),
      sectionId_graphicsMeshDefaults(
          app->settings()->addSection(this->groupId_graphics, textId("meshDefaults")))
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
    this->language.setDescription(
                tr("Language used for the application. Change will take effect after application restart"));
    this->linkWithDocumentSelector.setDescription(
                tr("In case where multiple documents are opened, make sure the document displayed in "
                   "the 3D view corresponds to what is selected in the model tree"));
    settings->addSetting(&this->language, this->groupId_application);
    settings->addSetting(&this->recentFiles, this->groupId_application);
    settings->addSetting(&this->lastOpenDir, this->groupId_application);
    settings->addSetting(&this->lastSelectedFormatFilter, this->groupId_application);
    settings->addSetting(&this->linkWithDocumentSelector, this->groupId_application);
    this->recentFiles.setUserVisible(false);
    this->lastOpenDir.setUserVisible(false);
    this->lastSelectedFormatFilter.setUserVisible(false);

    // Meshing
    this->meshingQuality.setDescription(
                tr("Controls precision of the mesh to be computed from the BRep shape"));
    this->meshingQuality.mutableEnumeration().changeTrContext(AppModule::textIdContext());
    this->meshingChordalDeflection.setDescription(
                tr("For the tesselation of faces the chordal deflection limits the distance between "
                   "a curve and its tessellation"));
    this->meshingAngularDeflection.setDescription(
                tr("For the tesselation of faces the angular deflection limits the angle between "
                   "subsequent segments in a polyline"));
    this->meshingRelative.setDescription(
                tr("Relative computation of edge tolerance\n\n"
                   "If activated, deflection used for the polygonalisation of each edge will be "
                   "`ChordalDeflection` &#215; `SizeOfEdge`. The deflection used for the faces will be "
                   "the maximum deflection of their edges."));
    settings->addSetting(&this->meshingQuality, this->groupId_meshing);
    settings->addSetting(&this->meshingChordalDeflection, this->groupId_meshing);
    settings->addSetting(&this->meshingAngularDeflection, this->groupId_meshing);
    settings->addSetting(&this->meshingRelative, this->groupId_meshing);

    // Graphics
    this->defaultShowOriginTrihedron.setDescription(
                tr("Show or hide by default the trihedron centered at world origin. "
                   "This doesn't affect 3D view of currently opened documents"));
    settings->addSetting(&this->defaultShowOriginTrihedron, this->groupId_graphics);
    settings->addSetting(&this->instantZoomFactor, this->groupId_graphics);
    // -- Clip planes
    this->clipPlanesCappingOn.setDescription(
                tr("Enable capping of currently clipped graphics"));
    this->clipPlanesCappingHatchOn.setDescription(
                tr("Enable capping hatch texture of currently clipped graphics"));
    settings->addSetting(&this->clipPlanesCappingOn, this->sectionId_graphicsClipPlanes);
    settings->addSetting(&this->clipPlanesCappingHatchOn, this->sectionId_graphicsClipPlanes);
    // -- Mesh defaults
    settings->addSetting(&this->meshDefaultsColor, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsEdgeColor, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsMaterial, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowEdges, this->sectionId_graphicsMeshDefaults);
    settings->addSetting(&this->meshDefaultsShowNodes, this->sectionId_graphicsMeshDefaults);
    // Import
    auto groupId_Import = settings->addGroup(textId("import"));
    for (IO::Format format : app->ioSystem()->readerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Import, IO::formatIdentifier(format));
        const IO::FactoryReader* factory = app->ioSystem()->findFactoryReader(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            settings->addResetFunction(sectionId_format, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatReaderParameters.insert({ format, rawPtrGroup });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }

    // Export
    auto groupId_Export = settings->addGroup(textId("export"));
    for (IO::Format format : app->ioSystem()->writerFormats()) {
        auto sectionId_format = settings->addSection(groupId_Export, IO::formatIdentifier(format));
        const IO::FactoryWriter* factory = app->ioSystem()->findFactoryWriter(format);
        std::unique_ptr<PropertyGroup> ptrGroup = factory->createProperties(format, settings);
        if (ptrGroup) {
            for (Property* property : ptrGroup->properties())
                settings->addSetting(property, sectionId_format);

            PropertyGroup* rawPtrGroup = ptrGroup.get();
            settings->addResetFunction(sectionId_format, [=]{ rawPtrGroup->restoreDefaults(); });
            m_mapFormatWriterParameters.insert({ format, ptrGroup.get() });
            m_vecPtrPropertyGroup.push_back(std::move(ptrGroup));
        }
    }

    // Register reset functions
    settings->addResetFunction(this->sectionId_systemUnits, [=]{
        this->unitSystemDecimals.setValue(2);
        this->unitSystemSchema.setValue(UnitSystem::SI);
    });
    settings->addResetFunction(this->groupId_application, [&]{
        this->language.setValue(enumLanguages.findValue("en"));
        this->recentFiles.setValue({});
        this->lastOpenDir.setValue(QString());
        this->lastSelectedFormatFilter.setValue(QString());
        this->linkWithDocumentSelector.setValue(true);
    });
    settings->addResetFunction(this->groupId_graphics, [=]{
        this->defaultShowOriginTrihedron.setValue(true);
        this->instantZoomFactor.setValue(5.);
    });
    settings->addResetFunction(this->groupId_meshing, [&]{
        this->meshingQuality.setValue(BRepMeshQuality::Normal);
        this->meshingChordalDeflection.setQuantity(1 * Quantity_Millimeter);
        this->meshingAngularDeflection.setQuantity(20 * Quantity_Degree);
        this->meshingRelative.setValue(false);
    });
    settings->addResetFunction(this->sectionId_graphicsClipPlanes, [=]{
        this->clipPlanesCappingOn.setValue(true);
        this->clipPlanesCappingHatchOn.setValue(true);
    });
    settings->addResetFunction(this->sectionId_graphicsMeshDefaults, [=]{
        const GraphicsMeshObjectDriver::DefaultValues meshDefaults;
        this->meshDefaultsColor.setValue(meshDefaults.color);
        this->meshDefaultsEdgeColor.setValue(meshDefaults.edgeColor);
        this->meshDefaultsMaterial.setValue(meshDefaults.material);
        this->meshDefaultsShowEdges.setValue(meshDefaults.showEdges);
        this->meshDefaultsShowNodes.setValue(meshDefaults.showNodes);
    });
}

StringUtils::TextOptions AppModule::defaultTextOptions() const
{
    StringUtils::TextOptions opts;
    opts.locale = m_app->settings()->locale();
    opts.unitDecimals = this->unitSystemDecimals;
    opts.unitSchema = this->unitSystemSchema;
    return opts;
}

QString AppModule::qmFilePath(const QByteArray& languageCode)
{
    return QString(":/i18n/mayo_%1.qm").arg(QString::fromUtf8(languageCode));
}

QByteArray AppModule::languageCode(const ApplicationPtr& app)
{
    const char keyLang[] = "application/language";
    const Settings* settings = app->settings();
    const QByteArray code = app ? settings->findValueFromKey(keyLang).toByteArray() : QByteArray();
    return !code.isEmpty() ? code : enumLanguages.findName(0);
}

bool AppModule::excludeSettingPredicate(const Property& prop)
{
    return !prop.isUserVisible();
}

const PropertyGroup* AppModule::findReaderParameters(IO::Format format) const
{
    auto it = m_mapFormatReaderParameters.find(format);
    return it != m_mapFormatReaderParameters.cend() ? it->second : nullptr;
}

const PropertyGroup* AppModule::findWriterParameters(IO::Format format) const
{
    auto it = m_mapFormatWriterParameters.find(format);
    return it != m_mapFormatWriterParameters.cend() ? it->second : nullptr;
}

QVariant AppModule::toVariant(const Property& prop) const
{
    if (isType<PropertyRecentFiles>(prop)) {
        const auto& filesProp = constRef<PropertyRecentFiles>(prop);
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream << filesProp.value();
        return blob;
    }
    else {
        return PropertyValueConversion::toVariant(prop);
    }
}

bool AppModule::fromVariant(Property* prop, const QVariant& variant) const
{
    if (isType<PropertyRecentFiles>(prop)) {
        if (qobject_cast<QGuiApplication*>(QCoreApplication::instance()) == nullptr)
            return true;

        const QByteArray blob = variant.toByteArray();
        QDataStream stream(blob);
        RecentFiles recentFiles;
        stream >> recentFiles;
        ptr<PropertyRecentFiles>(prop)->setValue(recentFiles);
        return true;
    }
    else {
        return PropertyValueConversion::fromVariant(prop, variant);
    }
}

void AppModule::prependRecentFile(const FilePath& fp)
{
    const RecentFile* ptrRecentFile = this->findRecentFile(fp);
    RecentFiles newRecentFiles = this->recentFiles.value();
    if (ptrRecentFile) {
        RecentFile& firstRecentFile = newRecentFiles.front();
        RecentFile& recentFile = newRecentFiles.at(ptrRecentFile - &this->recentFiles.value().front());
        std::swap(firstRecentFile, recentFile);
    }
    else {
        RecentFile recentFile;
        recentFile.filepath = fp;
        newRecentFiles.insert(newRecentFiles.begin(), std::move(recentFile));
        constexpr int sizeLimit = 15;
        while (newRecentFiles.size() > sizeLimit)
            newRecentFiles.pop_back();
    }

    this->recentFiles.setValue(newRecentFiles);
}

const RecentFile* AppModule::findRecentFile(const FilePath& fp) const
{
    const RecentFiles& listRecentFile = this->recentFiles.value();
    auto itFound =
            std::find_if(
                listRecentFile.cbegin(),
                listRecentFile.cend(),
                [=](const RecentFile& recentFile) {
        return filepathEquivalent(fp, recentFile.filepath);
    });
    return itFound != listRecentFile.cend() ? &(*itFound) : nullptr;
}

void AppModule::recordRecentFileThumbnail(GuiDocument* guiDoc)
{
    if (!guiDoc)
        return;

    const RecentFile* recentFile = this->findRecentFile(guiDoc->document()->filePath());
    if (!recentFile)
        return;

    if (!recentFile->isThumbnailOutOfSync())
        return;

    RecentFile newRecentFile = *recentFile;
    const bool okRecord = newRecentFile.recordThumbnail(guiDoc, this->recentFileThumbnailSize());
    if (!okRecord)
        return;

    const RecentFiles& listRecentFile = this->recentFiles.value();
    RecentFiles newListRecentFile = listRecentFile;
    const auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
    newListRecentFile.at(indexRecentFile) = newRecentFile;
    this->recentFiles.setValue(newListRecentFile);
}

void AppModule::recordRecentFileThumbnails(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

    const RecentFiles& listRecentFile = this->recentFiles.value();
    RecentFiles newListRecentFile = listRecentFile;
    for (GuiDocument* guiDoc : guiApp->guiDocuments()) {
        const RecentFile* recentFile = this->findRecentFile(guiDoc->document()->filePath());
        if (!recentFile || !recentFile->isThumbnailOutOfSync())
            continue; // Skip

        RecentFile newRecentFile = *recentFile;
        if (newRecentFile.recordThumbnail(guiDoc, this->recentFileThumbnailSize())) {
            auto indexRecentFile = std::distance(&listRecentFile.front(), recentFile);
            newListRecentFile.at(indexRecentFile) = newRecentFile;
        }
    }

    this->recentFiles.setValue(newListRecentFile);
}

static QuantityLength shapeChordalDeflection(const TopoDS_Shape& shape)
{
    // Excerpted from Prs3d::GetDeflection(...)
    constexpr QuantityLength baseDeviation = 1 * Quantity_Millimeter;

    Bnd_Box bndBox;
    constexpr bool useTriangulation = true;
    BRepBndLib::Add(shape, bndBox, !useTriangulation);
    if (bndBox.IsVoid())
        return baseDeviation;

    if (BndUtils::isOpen(bndBox)) {
        if (!BndUtils::hasFinitePart(bndBox))
            return baseDeviation;

        bndBox = BndUtils::finitePart(bndBox);
    }

    const auto coords = BndBoxCoords::get(bndBox);
    const gp_XYZ diag = coords.maxVertex().XYZ() - coords.minVertex().XYZ();
    const double diagMaxComp = std::max({ diag.X(), diag.Y(), diag.Z() });
    return 4 * diagMaxComp * baseDeviation;
}

OccBRepMeshParameters AppModule::brepMeshParameters(const TopoDS_Shape& shape) const
{
    OccBRepMeshParameters params;
    params.InParallel = true;
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    params.AllowQualityDecrease = true;
#endif
    if (this->meshingQuality == BRepMeshQuality::UserDefined) {
        params.Deflection = UnitSystem::meters(this->meshingChordalDeflection.quantity());
        params.Angle = UnitSystem::radians(this->meshingAngularDeflection.quantity());
        params.Relative = this->meshingRelative;
    }
    else {
        struct Coefficients {
            double chordalDeflection;
            double angularDeflection;
        };
        auto fnCoefficients = [](BRepMeshQuality meshQuality) -> Coefficients {
            switch (meshQuality) {
            case BRepMeshQuality::VeryCoarse: return { 8, 4 };
            case BRepMeshQuality::Coarse: return { 4, 2 };
            case BRepMeshQuality::Normal: return { 1, 1 };
            case BRepMeshQuality::Precise: return { 1/4., 1/2. };
            case BRepMeshQuality::VeryPrecise: return { 1/8., 1/4. };
            case BRepMeshQuality::UserDefined: return { -1, -1 };
            }
            return { 1, 1 };
        };
        const Coefficients coeffs = fnCoefficients(this->meshingQuality);
        params.Deflection = UnitSystem::meters(coeffs.chordalDeflection * shapeChordalDeflection(shape));
        params.Angle = UnitSystem::radians(coeffs.angularDeflection * (20 * Quantity_Degree));
    }

    return params;
}

void AppModule::computeBRepMesh(const TopoDS_Shape& shape, TaskProgress* progress)
{
    BRepUtils::computeMesh(shape, this->brepMeshParameters(shape), progress);
}

void AppModule::computeBRepMesh(const TDF_Label& labelEntity, TaskProgress* progress)
{
    if (XCaf::isShape(labelEntity))
        this->computeBRepMesh(XCaf::shape(labelEntity), progress);
}

AppModule* AppModule::get(const ApplicationPtr& app)
{
    if (app)
        return app->findChild<AppModule*>(QString(), Qt::FindDirectChildrenOnly);

    return nullptr;
}

void AppModule::onPropertyChanged(Property* prop)
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

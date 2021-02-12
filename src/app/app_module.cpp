/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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
#include "../graphics/graphics_object_driver.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"

#include <QtCore/QDir>
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
      groupId_graphics(app->settings()->addGroup(textId("graphics"))),
      sectionId_graphicsClipPlanes(
          app->settings()->addSection(this->groupId_graphics, textId("clipPlanes"))),
      sectionId_graphicsMeshDefaults(
          app->settings()->addSection(this->groupId_graphics, textId("meshDefaults")))
{
    auto settings = app->settings();

    qRegisterMetaTypeStreamOperators<RecentFiles>("RecentFiles");
    qRegisterMetaTypeStreamOperators<RecentFiles>("Mayo::RecentFiles");

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
        this->language.setValue(enumLanguages.findValue("en"));
        this->recentFiles.setValue({});
        this->lastOpenDir.setValue(QString());
        this->lastSelectedFormatFilter.setValue(QString());
        this->linkWithDocumentSelector.setValue(true);
    });
    settings->addGroupResetFunction(this->groupId_graphics, [&]{
        this->defaultShowOriginTrihedron.setValue(true);
        this->instantZoomFactor.setValue(5.);
        this->clipPlanesCappingOn.setValue(true);
        this->clipPlanesCappingHatchOn.setValue(true);
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

void AppModule::prependRecentFile(const QString& filepath)
{
    const RecentFile* ptrRecentFile = this->findRecentFile(filepath);
    RecentFiles newRecentFiles = this->recentFiles.value();
    if (ptrRecentFile) {
        RecentFile& firstRecentFile = newRecentFiles.front();
        RecentFile& recentFile = newRecentFiles.at(ptrRecentFile - &this->recentFiles.value().front());
        std::swap(firstRecentFile, recentFile);
    }
    else {
        RecentFile recentFile;
        recentFile.filepath = filepath;
        newRecentFiles.insert(newRecentFiles.begin(), std::move(recentFile));
        constexpr int sizeLimit = 15;
        while (newRecentFiles.size() > sizeLimit)
            newRecentFiles.pop_back();
    }

    this->recentFiles.setValue(newRecentFiles);
}

const RecentFile* AppModule::findRecentFile(const QString& filepath) const
{
    const QFileInfo fileInfo(filepath);
    const RecentFiles& listRecentFile = this->recentFiles.value();
    auto itFound =
            std::find_if(
                listRecentFile.cbegin(),
                listRecentFile.cend(),
                [=](const RecentFile& recentFile) {
        return fileInfo == QFileInfo(recentFile.filepath);
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

    PropertyGroup::onPropertyChanged(prop);
}

} // namespace Mayo

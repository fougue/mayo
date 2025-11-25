/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_measure.h"
#include "app_module.h"
#include "theme.h"
#include "ui_widget_measure.h"

#include "../gui/gui_document.h"
#include "../measure/measure_tool_brep.h"
#include "../qtcommon/qstring_conv.h"

#include <QtCore/QtDebug>
#include <QtGui/QFontDatabase>

namespace Mayo {

namespace {

using IMeasureToolPtr = std::unique_ptr<IMeasureTool>;

// Get global array of available measurement tool objects
std::vector<IMeasureToolPtr>& getMeasureTools()
{
    static std::vector<IMeasureToolPtr> vecTool;
    return vecTool;
}

// Returns the tool object adapted for the graphics object 'gfxObject' and measure type
// Note: the search is performed in the array returned by getMeasureTools() function
IMeasureTool* findSupportingMeasureTool(const GraphicsObjectPtr& gfxObject, MeasureType measureType)
{
    for (const IMeasureToolPtr& ptr : getMeasureTools()) {
        if (ptr->supports(measureType) && ptr->supports(gfxObject))
            return ptr.get();
    }

    return nullptr;
}

// Helper function to iterate and execute function 'fn' on all the graphics objects owned by a
// measure display
template<typename Function>
void foreachGraphicsObject(const IMeasureDisplay* ptr, Function fn)
{
    if (ptr) {
        const int count = ptr->graphicsObjectsCount();
        for (int i = 0; i < count; ++i)
            fn(ptr->graphicsObjectAt(i));
    }
}

// Overload of the helper function above
template<typename Function>
void foreachGraphicsObject(const std::unique_ptr<IMeasureDisplay>& ptr, Function fn) {
    foreachGraphicsObject(ptr.get(), fn);
}

} // namespace

WidgetMeasure::WidgetMeasure(GuiDocument* guiDoc, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetMeasure),
      m_guiDoc(guiDoc)
{
    if (getMeasureTools().empty())
        getMeasureTools().push_back(std::make_unique<MeasureToolBRep>());

    m_ui->setupUi(this);
    QObject::connect(
        m_ui->combo_MeasureType, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &WidgetMeasure::onMeasureTypeChanged
    );
    QObject::connect(
        m_ui->combo_LengthUnit, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &WidgetMeasure::onMeasureUnitsChanged
    );
    QObject::connect(
        m_ui->combo_AngleUnit, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &WidgetMeasure::onMeasureUnitsChanged
    );
    QObject::connect(
        m_ui->combo_AreaUnit, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &WidgetMeasure::onMeasureUnitsChanged
    );
    QObject::connect(
        m_ui->combo_VolumeUnit, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &WidgetMeasure::onMeasureUnitsChanged
    );

    this->onMeasureTypeChanged(m_ui->combo_MeasureType->currentIndex());
    this->updateMessagePanel();
}

WidgetMeasure::~WidgetMeasure()
{
    m_connGraphicsSelectionChanged.disconnect();
    m_connDocumentEntityAdded.disconnect();
    delete m_ui;
}

void WidgetMeasure::setMeasureOn(bool on)
{
    m_errorMessage.clear();
    auto gfxScene = m_guiDoc->graphicsScene();
    if (on) {
        this->onMeasureTypeChanged(m_ui->combo_MeasureType->currentIndex());
        m_connGraphicsSelectionChanged = gfxScene->signalSelectionChanged.connectSlot(
            &WidgetMeasure::onGraphicsSelectionChanged, this
        );
        m_connDocumentEntityAdded = m_guiDoc->document()->signalEntityAdded.connectSlot(
            &WidgetMeasure::onDocumentEntityAdded, this
        );
    }
    else {
        gfxScene->foreachDisplayedObject([=](const GraphicsObjectPtr& gfxObject) {
            gfxScene->deactivateObjectSelection(gfxObject);
            gfxScene->activateObjectSelection(gfxObject, 0);
        });
        gfxScene->clearSelection();
        m_connGraphicsSelectionChanged.disconnect();
        m_connDocumentEntityAdded.disconnect();
    }
}

void WidgetMeasure::addTool(std::unique_ptr<IMeasureTool> tool)
{
    if (tool)
        getMeasureTools().push_back(std::move(tool));
}

MeasureType WidgetMeasure::toMeasureType(int comboBoxId)
{
    switch (comboBoxId) {
    case 0: return MeasureType::VertexPosition;
    case 1: return MeasureType::CircleCenter;
    case 2: return MeasureType::CircleDiameter;
    case 3: return MeasureType::MinDistance;
    case 4: return MeasureType::CenterDistance;
    case 5: return MeasureType::Angle;
    case 6: return MeasureType::Length;
    case 7: return MeasureType::Area;
    case 8: return MeasureType::BoundingBox;
    }
    return MeasureType::None;
}

LengthUnit WidgetMeasure::toMeasureLengthUnit(int comboBoxId)
{
    switch (comboBoxId) {
    case 0: return LengthUnit::Millimeter;
    case 1: return LengthUnit::Centimeter;
    case 2: return LengthUnit::Meter;
    case 3: return LengthUnit::Inch;
    case 4: return LengthUnit::Foot;
    case 5: return LengthUnit::Yard;
    }
    return {};
}

AngleUnit WidgetMeasure::toMeasureAngleUnit(int comboBoxId)
{
    switch (comboBoxId) {
    case 0: return AngleUnit::Degree;
    case 1: return AngleUnit::Radian;
    }
    return {};
}

AreaUnit WidgetMeasure::toMeasureAreaUnit(int comboBoxId)
{
    switch (comboBoxId) {
    case 0: return AreaUnit::SquareMillimeter;
    case 1: return AreaUnit::SquareCentimeter;
    case 2: return AreaUnit::SquareMeter;
    case 3: return AreaUnit::SquareInch;
    case 4: return AreaUnit::SquareFoot;
    case 5: return AreaUnit::SquareYard;
    }
    return {};
}

VolumeUnit WidgetMeasure::toMeasureVolumeUnit(int comboBoxId)
{
    switch (comboBoxId) {
    case 0: return VolumeUnit::CubicMillimeter;
    case 1: return VolumeUnit::CubicCentimeter;
    case 2: return VolumeUnit::CubicMeter;
    case 3: return VolumeUnit::CubicInch;
    case 4: return VolumeUnit::CubicFoot;
    case 5: return VolumeUnit::Liter;
    case 6: return VolumeUnit::ImperialGallon;
    case 7: return VolumeUnit::USGallon;
    }
    return {};
}

void WidgetMeasure::onMeasureTypeChanged(int id)
{
    // Update widgets visibility
    const MeasureType measureType = WidgetMeasure::toMeasureType(id);
    const bool measureIsLengthBased = measureType != MeasureType::Angle;
    const bool measureIsAngle = measureType == MeasureType::Angle;
    const bool measureIsArea = measureType == MeasureType::Area;
    const bool measureIsVolume = measureType == MeasureType::BoundingBox;
    // Note: don't call "ui->comboUnit->setVisible(labelUnit->isVisible())" because at this point
    //       QWidget::isVisible() might not be effective(probably needs to process eventloop)
    m_ui->label_LengthUnit->setVisible(measureIsLengthBased && !measureIsArea);
    m_ui->combo_LengthUnit->setVisible(measureIsLengthBased && !measureIsArea);
    m_ui->label_AngleUnit->setVisible(measureIsAngle);
    m_ui->combo_AngleUnit->setVisible(measureIsAngle);
    m_ui->label_AreaUnit->setVisible(measureIsArea);
    m_ui->combo_AreaUnit->setVisible(measureIsArea);
    m_ui->label_VolumeUnit->setVisible(measureIsVolume);
    m_ui->combo_VolumeUnit->setVisible(measureIsVolume);

    auto gfxScene = m_guiDoc->graphicsScene();

    // Find measure tool
    m_tool = nullptr;
    gfxScene->foreachDisplayedObject([=](const GraphicsObjectPtr& gfxObject) {
        if (!m_tool)
            m_tool = findSupportingMeasureTool(gfxObject, measureType);
    });

    // Apply 3D selection modes required by the measure tool
    gfxScene->clearSelection();
    gfxScene->foreachDisplayedObject([=](const GraphicsObjectPtr& gfxObject) {
        if (GuiDocument::isAisViewCubeObject(gfxObject))
            return; // Skip

        gfxScene->deactivateObjectSelection(gfxObject);
        if (m_tool) {
            for (GraphicsObjectSelectionMode mode : m_tool->selectionModes(measureType))
                gfxScene->activateObjectSelection(gfxObject, mode);
        }
    });
    gfxScene->redraw();
}

void WidgetMeasure::onMeasureUnitsChanged()
{
    const MeasureDisplayConfig config = this->currentMeasureDisplayConfig();
    for (IMeasureDisplayPtr& measure : m_vecMeasureDisplay) {
        measure->update(config);
        foreachGraphicsObject(measure, [](const GraphicsObjectPtr& gfxObject) {
            gfxObject->Redisplay();
        });
    }

    m_guiDoc->graphicsScene()->redraw();
    this->updateMessagePanel();
}

MeasureType WidgetMeasure::currentMeasureType() const
{
    return WidgetMeasure::toMeasureType(m_ui->combo_MeasureType->currentIndex());
}

MeasureDisplayConfig WidgetMeasure::currentMeasureDisplayConfig() const
{
    MeasureDisplayConfig cfg;
    cfg.lengthUnit = WidgetMeasure::toMeasureLengthUnit(m_ui->combo_LengthUnit->currentIndex());
    cfg.angleUnit = WidgetMeasure::toMeasureAngleUnit(m_ui->combo_AngleUnit->currentIndex());
    cfg.areaUnit = WidgetMeasure::toMeasureAreaUnit(m_ui->combo_AreaUnit->currentIndex());
    cfg.volumeUnit = WidgetMeasure::toMeasureVolumeUnit(m_ui->combo_VolumeUnit->currentIndex());
    cfg.doubleToStringOptions.locale = AppModule::get()->stdLocale();
    cfg.doubleToStringOptions.decimalCount = AppModule::get()->defaultTextOptions().unitDecimals;
    cfg.devicePixelRatio = this->devicePixelRatioF();
    return cfg;
}

void WidgetMeasure::onGraphicsSelectionChanged()
{
    auto gfxScene = m_guiDoc->graphicsScene();
    const std::vector<GraphicsOwnerPtr> vecSelectedOwner_onEntry = m_vecSelectedOwner;
    std::vector<GraphicsOwnerPtr> vecNewSelected;
    std::vector<GraphicsOwnerPtr> vecDeselected;
    {
        // Store currently selected graphics
        std::vector<GraphicsOwnerPtr> vecSelected;
        gfxScene->foreachSelectedOwner([&](const GraphicsOwnerPtr& owner) {
            vecSelected.push_back(owner);
        });

        // Find new selected graphics
        for (const GraphicsOwnerPtr& owner : vecSelected) {
            auto itFound = std::find(m_vecSelectedOwner.begin(), m_vecSelectedOwner.end(), owner);
            if (itFound == m_vecSelectedOwner.end())
                vecNewSelected.push_back(owner);
        }

        // Find deselected graphics
        for (const GraphicsOwnerPtr& owner : m_vecSelectedOwner) {
            auto itFound = std::find(vecSelected.begin(), vecSelected.end(), owner);
            if (itFound == vecSelected.end())
                vecDeselected.push_back(owner);
        }

        m_vecSelectedOwner = std::move(vecSelected);
    }

    // Erase objects associated to deselected graphics
    for (const GraphicsOwnerPtr& owner : vecDeselected) {
        for (auto link = this->findLink(owner); link != nullptr; link = this->findLink(owner)) {
            this->eraseMeasureDisplay(link->measureDisplay);
            this->eraseLink(link);
        }
    }

    m_guiDoc->graphicsScene()->redraw();
    m_errorMessage.clear();

    // Exit if no measure tool available
    if (!m_tool)
        return;

    // Holds MeasureDisplay objects created for new selected graphics
    std::vector<IMeasureDisplayPtr> vecNewMeasureDisplay;

    // Helper to ease addition(registration) of MeasureDisplay object
    auto fnAddMeasureDisplay = [&](
            std::unique_ptr<IMeasureDisplay> ptr,
            std::initializer_list<GraphicsOwnerPtr> listOwner)
    {
        if (ptr) {
            vecNewMeasureDisplay.push_back(std::move(ptr));
            for (const GraphicsOwnerPtr& owner : listOwner)
                this->addLink(owner, vecNewMeasureDisplay.back());
        }
    };

    const MeasureType measureType = this->currentMeasureType();
    // Create MeasureDisplay objects needing a newly single selected graphics object
    for (const GraphicsOwnerPtr& owner : vecNewSelected) {
        try {
            const MeasureValue value = IMeasureTool_computeValue(*m_tool, measureType, owner);
            if (MeasureValue_isValid(value))
                fnAddMeasureDisplay(BaseMeasureDisplay::createFrom(measureType, value), { owner });
        } catch (const IMeasureError& err) {
            m_errorMessage = to_QString(err.message());
        }
    }

    // Create MeasureDisplay objects needing currently two selected graphics objects
    if (vecSelectedOwner_onEntry.size() >= 1) {
        for (const GraphicsOwnerPtr& owner : vecNewSelected) {
            const int indexOwner = Span_itemIndex(vecNewSelected, owner);
            const GraphicsOwnerPtr& prevOwner =
                indexOwner == 0 ?
                    vecSelectedOwner_onEntry.back() :
                    vecNewSelected.at(indexOwner - 1)
                ;
            try {
                const MeasureValue value = IMeasureTool_computeValue(*m_tool, measureType, prevOwner, owner);
                if (MeasureValue_isValid(value))
                    fnAddMeasureDisplay(BaseMeasureDisplay::createFrom(measureType, value), { prevOwner, owner });
            } catch (const IMeasureError& err) {
                m_errorMessage = to_QString(err.message());
            }
        }
    }

    // Display new measure graphics objects
    auto measureDisplayConfig = this->currentMeasureDisplayConfig();
    for (IMeasureDisplayPtr& measure : vecNewMeasureDisplay) {
        measure->update(measureDisplayConfig);
        measure->adaptGraphics(gfxScene->v3dViewer()->Driver());
        foreachGraphicsObject(measure, [=](const GraphicsObjectPtr& gfxObject) {
            gfxScene->addObject(gfxObject, GraphicsScene::AddObjectDisableSelectionMode);
        });

        m_vecMeasureDisplay.push_back(std::move(measure));
    }

    this->updateMessagePanel();
}

void WidgetMeasure::onDocumentEntityAdded(TreeNodeId entityNodeId)
{
    if (!m_tool)
        return;

    auto measureType = this->currentMeasureType();
    if (measureType == MeasureType::None)
        return;

    m_guiDoc->foreachGraphicsObject(entityNodeId, [=](const GraphicsObjectPtr& gfxObject) {
        for (GraphicsObjectSelectionMode mode : m_tool->selectionModes(measureType))
            m_guiDoc->graphicsScene()->activateObjectSelection(gfxObject, mode);
    });
}

void WidgetMeasure::updateMessagePanel()
{
    // Clear message panel
    while (m_ui->layout_Message->count() > 0) {
        QLayoutItem* item = m_ui->layout_Message->takeAt(m_ui->layout_Message->count() - 1);
        delete item->widget();
        delete item;
    }

    // Error message takes precedence
    if (!m_errorMessage.isEmpty() || m_vecMeasureDisplay.empty()) {
        auto labelMessage = new QLabel(m_ui->widget_Message);
        labelMessage->setContentsMargins(m_ui->layout_Main->contentsMargins());
        m_ui->layout_Message->addWidget(labelMessage);
        const auto msgTextColorRole =
                m_errorMessage.isEmpty() ?
                    Theme::Color::MessageIndicator_InfoText :
                    Theme::Color::MessageIndicator_ErrorText;
        const auto msgBackgroundColorRole =
                m_errorMessage.isEmpty() ?
                    Theme::Color::MessageIndicator_InfoBackground :
                    Theme::Color::MessageIndicator_ErrorBackground;
        labelMessage->setStyleSheet(
                    QString("QLabel { color: %1; background-color: %2 }")
                    .arg(mayoTheme()->color(msgTextColorRole).name(),
                         mayoTheme()->color(msgBackgroundColorRole).name())
        );
        const QString msg = m_errorMessage.isEmpty() ? tr("Select entities to measure") : m_errorMessage;
        labelMessage->setText(msg);
    }
    else {
        // The font to be used for measure results
        const QFont fontResult = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        // Helper function to add the text of an IMeasureDisplay object in the message panel
        auto fnAddMeasureText = [=](const IMeasureDisplayPtr& measure) {
            auto label = new QLabel(to_QString(measure->text()), m_ui->widget_Message);
            label->setFont(fontResult);
            m_ui->layout_Message->addWidget(label);
        };

        // Add measure texts to the message panel
        for (const IMeasureDisplayPtr& measure : m_vecMeasureDisplay)
            fnAddMeasureText(measure);

        // Handle the case where there are many measures and sum is a supported operation
        if (m_vecMeasureDisplay.size() > 1) {
            auto sumMeasure = BaseMeasureDisplay::createEmptySum(this->currentMeasureType());
            if (sumMeasure && sumMeasure->isSumSupported()) {
                for (const IMeasureDisplayPtr& measure : m_vecMeasureDisplay)
                    sumMeasure->sumAdd(*measure);

                sumMeasure->update(this->currentMeasureDisplayConfig());
                fnAddMeasureText(sumMeasure);
            }
        }
    }

    emit this->sizeAdjustmentRequested();
}

void WidgetMeasure::eraseMeasureDisplay(const IMeasureDisplay* measure)
{
    if (!measure)
        return;

    auto it = std::find_if(
                m_vecMeasureDisplay.begin(),
                m_vecMeasureDisplay.end(),
                [=](const IMeasureDisplayPtr& ptr) { return ptr.get() == measure; }
    );
    if (it != m_vecMeasureDisplay.end()) {
        foreachGraphicsObject(measure, [=](const GraphicsObjectPtr& gfxObject) {
            m_guiDoc->graphicsScene()->eraseObject(gfxObject);
        });

        m_vecMeasureDisplay.erase(it);
    }
}

void WidgetMeasure::addLink(const GraphicsOwnerPtr& owner, const IMeasureDisplayPtr& measure)
{
    if (owner && measure) {
        m_vecLinkGfxOwnerMeasure.push_back({ owner, measure.get() });
    }
}

void WidgetMeasure::eraseLink(const GraphicsOwner_MeasureDisplay* link)
{
    if (!link)
        return;

    m_vecLinkGfxOwnerMeasure.erase(m_vecLinkGfxOwnerMeasure.begin() + (link - &m_vecLinkGfxOwnerMeasure.front()));
}

const WidgetMeasure::GraphicsOwner_MeasureDisplay* WidgetMeasure::findLink(const GraphicsOwnerPtr& owner) const
{
    auto itFound = std::find_if(
        m_vecLinkGfxOwnerMeasure.begin(),
        m_vecLinkGfxOwnerMeasure.end(),
        [=](const GraphicsOwner_MeasureDisplay& link) { return link.gfxOwner == owner; }
    );
    return itFound != m_vecLinkGfxOwnerMeasure.end() ? &(*itFound) : nullptr;
}

} // namespace Mayo

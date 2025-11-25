/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_clip_planes.h"

#include "../base/bnd_utils.h"
#include "../base/math_utils.h"
#include "../base/settings.h"
#include "../base/tkernel_utils.h"
#include "../graphics/graphics_texture2d.h"
#include "../graphics/graphics_utils.h"
#include "app_module.h"
#include "ui_widget_clip_planes.h"

#include <QtCore/QFile>
#include <Bnd_Box.hxx>
#include <Graphic3d_ClipPlane.hxx>
#include <Image_AlienPixMap.hxx>
#include <V3d_View.hxx>
#include <cmath>

namespace Mayo {

WidgetClipPlanes::WidgetClipPlanes(GraphicsViewPtr view, QWidget* parent)
    : QWidget(parent),
      m_ui(new Ui_WidgetClipPlanes),
      m_view(view)
{
    m_ui->setupUi(this);
    this->createPlaneCappingTexture();

    m_vecClipPlaneData = {
        {
            new Graphic3d_ClipPlane(gp_Pln(gp::Origin(), gp::DX())),
            UiClipPlane(m_ui->check_X, m_ui->widget_X)
        },
        {
            new Graphic3d_ClipPlane(gp_Pln(gp::Origin(), gp::DY())),
            UiClipPlane(m_ui->check_Y, m_ui->widget_Y)
        },
        {
            new Graphic3d_ClipPlane(gp_Pln(gp::Origin(), gp::DZ())),
            UiClipPlane(m_ui->check_Z, m_ui->widget_Z)
        },
        {
            new Graphic3d_ClipPlane(gp_Pln(gp::Origin(), gp_Dir(1, 1, 1))),
            UiClipPlane(m_ui->check_Custom, m_ui->widget_Custom)
        }
    };

    const auto appModule = AppModule::get();
    for (ClipPlaneData& data : m_vecClipPlaneData) {
        data.ui.widget_Control->setEnabled(data.ui.check_On->isChecked());
        this->connectUi(&data);
        data.graphics->SetCapping(appModule->properties()->clipPlanesCappingOn.value());
        data.graphics->SetUseObjectMaterial(true);
        if (m_textureCapping && appModule->properties()->clipPlanesCappingHatchOn)
            data.graphics->SetCappingTexture(m_textureCapping);
    }

    appModule->settings()->signalChanged.connectSlot([=](const Property* property) {
        if (property == &appModule->properties()->clipPlanesCappingOn) {
            for (ClipPlaneData& data : m_vecClipPlaneData)
                data.graphics->SetCapping(appModule->properties()->clipPlanesCappingOn);

            m_view.redraw();
        }
        else if (property == &appModule->properties()->clipPlanesCappingHatchOn) {
            OccHandle<Graphic3d_TextureMap> hatchTexture;
            if (m_textureCapping && appModule->properties()->clipPlanesCappingHatchOn)
                hatchTexture = m_textureCapping;

            for (ClipPlaneData& data : m_vecClipPlaneData)
                data.graphics->SetCappingTexture(hatchTexture);

            m_view.redraw();
        }
    });

    m_ui->widget_CustomDir->setVisible(false);
}

WidgetClipPlanes::~WidgetClipPlanes()
{
    delete m_ui;
}

void WidgetClipPlanes::setRanges(const Bnd_Box& bndBox)
{
    m_bndBox = bndBox;
    const bool isBndBoxVoid = bndBox.IsVoid();
    const auto bbc = BndBoxCoords::get(bndBox);
    for (ClipPlaneData& data : m_vecClipPlaneData) {
        const gp_Dir& n = data.graphics->ToPlane().Axis().Direction();
        this->setPlaneRange(&data, MathUtils::planeRange(bbc, n));
        data.ui.check_On->setEnabled(!isBndBoxVoid);
        if (isBndBoxVoid)
            data.ui.check_On->setChecked(false);
    }

    m_view.redraw();
}

void WidgetClipPlanes::setClippingOn(bool on)
{
    for (ClipPlaneData& data : m_vecClipPlaneData)
        data.graphics->SetOn(on ? data.ui.check_On->isChecked() : false);

    m_view.redraw();
}

void WidgetClipPlanes::connectUi(ClipPlaneData* data)
{
    UiClipPlane& ui = data->ui;
    const OccHandle<Graphic3d_ClipPlane>& gfx = data->graphics;
    QAbstractSlider* posSlider = ui.posSlider();
    QDoubleSpinBox* posSpin = ui.posSpin();
    auto signalSpinValueChanged = qOverload<double>(&QDoubleSpinBox::valueChanged);

    QObject::connect(ui.check_On, &QAbstractButton::toggled, this, [=](bool on) {
        ui.widget_Control->setEnabled(on);
        this->setPlaneOn(gfx, on);
        m_view.redraw();
    });

    if (data->ui.customXDirSpin()) {
        auto widgetDir = ui.widget_Control->findChild<QWidget*>("widget_CustomDir");
        QObject::connect(ui.check_On, &QAbstractButton::toggled, this, [=](bool on) {
            widgetDir->setVisible(on);
            QWidget* panel = this->parentWidget() ? this->parentWidget() : this;
            if (on) {
                panel->adjustSize();
            }
            else {
                const QSize sz = panel->size();
                const int offset = ui.customXDirSpin()->frameGeometry().height();
                panel->resize(sz.width(), sz.height() - offset);
            }
        });
    }

    QObject::connect(posSpin, signalSpinValueChanged, this, [=](double pos) {
        [[maybe_unused]] QSignalBlocker sigBlock(posSlider);
        const double dPct = ui.spinValueToSliderValue(pos);
        posSlider->setValue(qRound(dPct));
        GraphicsUtils::Gfx3dClipPlane_setPosition(gfx, pos);
        m_view.redraw();
    });

    QObject::connect(posSlider, &QSlider::valueChanged, this, [=](int pct) {
        const double pos = ui.sliderValueToSpinValue(pct);
        [[maybe_unused]] QSignalBlocker sigBlock(posSpin);
        posSpin->setValue(pos);
        GraphicsUtils::Gfx3dClipPlane_setPosition(gfx, pos);
        m_view.redraw();
    });

    QObject::connect(ui.inverseBtn(), &QAbstractButton::clicked, this, [=]{
        const gp_Dir invNormal = gfx->ToPlane().Axis().Direction().Reversed();
        GraphicsUtils::Gfx3dClipPlane_setNormal(gfx, invNormal);
        GraphicsUtils::Gfx3dClipPlane_setPosition(gfx, data->ui.posSpin()->value());
        m_view.redraw();
    });

    // Custom plane normal
    QDoubleSpinBox* customXDirSpin = ui.customXDirSpin();
    QDoubleSpinBox* customYDirSpin = ui.customYDirSpin();
    QDoubleSpinBox* customZDirSpin = ui.customZDirSpin();
    auto fnConnectDirSpin = [=](QDoubleSpinBox* dirSpin) {
        QObject::connect(dirSpin, signalSpinValueChanged, this, [=]{
            const gp_Vec vecNormal(
                customXDirSpin->value(), customYDirSpin->value(), customZDirSpin->value()
            );
            if (vecNormal.Magnitude() > Precision::Confusion()) {
                const gp_Dir normal(vecNormal);
                const auto bbc = BndBoxCoords::get(m_bndBox);
                this->setPlaneRange(data, MathUtils::planeRange(bbc, normal));
                GraphicsUtils::Gfx3dClipPlane_setNormal(gfx, normal);
                m_view.redraw();
            }
        });
    };
    if (customXDirSpin) {
        fnConnectDirSpin(customXDirSpin);
        fnConnectDirSpin(customYDirSpin);
        fnConnectDirSpin(customZDirSpin);
        QObject::connect(ui.inverseBtn(), &QAbstractButton::clicked, this, [=]{
            [[maybe_unused]] QSignalBlocker sigBlockX(customXDirSpin);
            [[maybe_unused]] QSignalBlocker sigBlockY(customYDirSpin);
            [[maybe_unused]] QSignalBlocker sigBlockZ(customZDirSpin);
            const gp_Dir& n = gfx->ToPlane().Axis().Direction();
            customXDirSpin->setValue(n.X());
            customYDirSpin->setValue(n.Y());
            customZDirSpin->setValue(n.Z());
        });
    }
}

void WidgetClipPlanes::setPlaneOn(const OccHandle<Graphic3d_ClipPlane>& plane, bool on)
{
    plane->SetOn(on);
    if (!GraphicsUtils::V3dView_hasClipPlane(m_view.v3dView(), plane))
        m_view->AddClipPlane(plane);
}

void WidgetClipPlanes::setPlaneRange(ClipPlaneData* data, const Range& range)
{
    const double rmin = range.first;
    const double rmax = range.second;
    QDoubleSpinBox* posSpin = data->ui.posSpin();
    QAbstractSlider* posSlider = data->ui.posSlider();
    const double currPlanePos = posSpin->value();
    const double gap = (rmax - rmin) * 0.01;
    const double mid = rmin + (rmax - rmin) / 2.;
    const bool isCurrPlanePosValid = rmin <= currPlanePos && currPlanePos <= rmax;
    const bool isEmptyPosSpinRange =
        std::abs(posSpin->maximum() - posSpin->minimum()) < Precision::Confusion();
    const bool useMidValue = isEmptyPosSpinRange || !isCurrPlanePosValid;
    const double newPlanePos = useMidValue ? mid : currPlanePos;
    posSpin->setRange(rmin - gap, rmax + gap);
    posSpin->setSingleStep(std::abs(posSpin->maximum() - posSpin->minimum()) / 100.);
    if (useMidValue) {
        GraphicsUtils::Gfx3dClipPlane_setPosition(data->graphics, newPlanePos);
        posSpin->setValue(newPlanePos);
        posSlider->setValue(data->ui.spinValueToSliderValue(newPlanePos));
    }
}

void WidgetClipPlanes::createPlaneCappingTexture()
{
    if (!m_textureCapping.IsNull())
        return;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
    QFile file(":/images/graphics/opencascade_hatch_1.png");
    if (file.open(QIODevice::ReadOnly)) {
        const QByteArray fileContents = file.readAll();
        const QByteArray filenameUtf8 = file.fileName().toUtf8();
        auto fileContentsData = reinterpret_cast<const Standard_Byte*>(fileContents.constData());
        auto imageCapping = makeOccHandle<Image_AlienPixMap>();
        imageCapping->Load(fileContentsData, fileContents.size(), filenameUtf8.constData());
        m_textureCapping = new GraphicsTexture2D(imageCapping);
        m_textureCapping->EnableModulate();
        m_textureCapping->EnableRepeat();
        m_textureCapping->GetParams()->SetScale(Graphic3d_Vec2(0.05f, -0.05f));
    }
#else
    // TODO Copy the image resource to a temporary file and call Image_AlienPixMap::Load(tempFilePath)
#endif
}

WidgetClipPlanes::UiClipPlane::UiClipPlane(QCheckBox* checkOn, QWidget* widgetControl)
    : check_On(checkOn), widget_Control(widgetControl)
{ }

QDoubleSpinBox* WidgetClipPlanes::UiClipPlane::posSpin() const
{
    return this->widget_Control->findChild<QDoubleSpinBox*>({}, Qt::FindDirectChildrenOnly);
}

QAbstractSlider* WidgetClipPlanes::UiClipPlane::posSlider() const
{
    return this->widget_Control->findChild<QSlider*>({}, Qt::FindDirectChildrenOnly);
}

QAbstractButton* WidgetClipPlanes::UiClipPlane::inverseBtn() const
{
    return this->widget_Control->findChild<QToolButton*>({}, Qt::FindDirectChildrenOnly);
}

QDoubleSpinBox* WidgetClipPlanes::UiClipPlane::customXDirSpin() const
{
    return this->widget_Control->findChild<QDoubleSpinBox*>("spin_CustomDirX");
}

QDoubleSpinBox* WidgetClipPlanes::UiClipPlane::customYDirSpin() const
{
    return this->widget_Control->findChild<QDoubleSpinBox*>("spin_CustomDirY");
}

QDoubleSpinBox* WidgetClipPlanes::UiClipPlane::customZDirSpin() const
{
    return this->widget_Control->findChild<QDoubleSpinBox*>("spin_CustomDirZ");
}

double WidgetClipPlanes::UiClipPlane::spinValueToSliderValue(double val) const
{
    const QDoubleSpinBox* spin = this->posSpin();
    const QAbstractSlider* slider = this->posSlider();
    return MathUtils::mappedValue(
        val, spin->minimum(), spin->maximum(), slider->minimum(), slider->maximum()
    );
}

double WidgetClipPlanes::UiClipPlane::sliderValueToSpinValue(double val) const
{
    const QDoubleSpinBox* spin = this->posSpin();
    const QAbstractSlider* slider = this->posSlider();
    return MathUtils::mappedValue(
        val, slider->minimum(), slider->maximum(), spin->minimum(), spin->maximum()
    );
}

} // namespace Mayo

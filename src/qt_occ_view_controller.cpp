/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qt_occ_view_controller.h"

#include "widget_occ_view.h"

#include <QtCore/QDebug>
#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <V3d_View.hxx>

// For QtOccViewController
#include <QtWidgets/QRubberBand>
#include <QtWidgets/QStyleFactory>

namespace Mayo {

namespace Internal {

static const QCursor& rotateCursor()
{
    static QCursor cursor;
    if (!cursor.bitmap()) {
        const int rotateCursorWidth = 16;
        const int rotateCursorHeight = 16;
        const int rotateCursorByteCount =
                ((rotateCursorWidth + 7) / 8) * rotateCursorHeight;
        const int rotateCursorHotX = 6;
        const int rotateCursorHotY = 8;

        static unsigned char rotateCursorBitmap[rotateCursorByteCount] = {
            0xf0, 0xef, 0x18, 0xb8, 0x0c, 0x90, 0xe4, 0x83,
            0x34, 0x86, 0x1c, 0x83, 0x00, 0x81, 0x00, 0xff,
            0xff, 0x00, 0x81, 0x00, 0xc1, 0x38, 0x61, 0x2c,
            0xc1, 0x27, 0x09, 0x30, 0x1d, 0x18, 0xf7, 0x0f
        };

        static unsigned char rotateCursorMaskBitmap[rotateCursorByteCount] = {
            0xf0,0xef,0xf8,0xff,0xfc,0xff,0xfc,0xff,0x3c,0xfe,0x1c,0xff,0x00,0xff,0x00,
            0xff,0xff,0x00,0xff,0x00,0xff,0x38,0x7f,0x3c,0xff,0x3f,0xff,0x3f,0xff,0x1f,
            0xf7,0x0f
        };

        const QBitmap cursorBmp = QBitmap::fromData(
                    QSize(rotateCursorWidth, rotateCursorHeight),
                    rotateCursorBitmap);
        const QBitmap maskBmp = QBitmap::fromData(
                    QSize(rotateCursorWidth, rotateCursorHeight),
                    rotateCursorMaskBitmap);
        const QCursor tempCursor(
                    cursorBmp, maskBmp, rotateCursorHotX, rotateCursorHotY);
        cursor = std::move(tempCursor);
    }
    return cursor;
}

} // namespace Internal

QtOccViewController::QtOccViewController(WidgetOccView* widgetView)
    : BaseV3dViewController(widgetView->v3dView(), widgetView),
      m_widgetView(widgetView)
{
    widgetView->installEventFilter(this);
}

bool QtOccViewController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_widgetView)
        return false;
    Handle_V3d_View view = m_widgetView->v3dView();

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = m_widgetView->mapFromGlobal(mouseEvent->globalPos());
        m_prevPos = currPos;
        break;
    }
    case QEvent::MouseMove: {
        auto mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = m_widgetView->mapFromGlobal(mouseEvent->globalPos());
        const QPoint prevPos = m_prevPos;
        m_prevPos = currPos;
        if (mouseEvent->buttons() == Qt::LeftButton) {
            if (!this->isRotationStarted()) {
                this->setViewCursor(Internal::rotateCursor());
                this->startDynamicAction(DynamicAction::Rotation);
                view->StartRotation(prevPos.x(), prevPos.y());
            }

            view->Rotation(currPos.x(), currPos.y());
        }
        else if (mouseEvent->buttons() == Qt::RightButton) {
            if (!this->isPanningStarted()) {
                this->setViewCursor(Qt::SizeAllCursor);
                this->startDynamicAction(DynamicAction::Panning);
            }

            view->Pan(currPos.x() - prevPos.x(), prevPos.y() - currPos.y());
        }
        else if (mouseEvent->buttons() == Qt::MiddleButton) {
            if (!this->isWindowZoomingStarted()) {
                this->setViewCursor(Qt::SizeBDiagCursor);
                this->startDynamicAction(DynamicAction::WindowZoom);
                m_posRubberBandStart = currPos;
            }

            this->drawRubberBand(m_posRubberBandStart, currPos);
        }

        emit mouseMoved(currPos);
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto mouseEvent = static_cast<const QMouseEvent*>(event);
        const bool hadDynamicAction = this->hasCurrentDynamicAction();
        if (this->isWindowZoomingStarted()) {
            const QPoint currPos = m_widgetView->mapFromGlobal(mouseEvent->globalPos());
            this->windowFitAll(m_posRubberBandStart, currPos);
            this->hideRubberBand();
        }

        this->setViewCursor(Qt::ArrowCursor);
        this->stopDynamicAction();
        if (!hadDynamicAction)
            emit mouseClicked(mouseEvent->button());

        break;
    }
    case QEvent::Wheel: {
        auto wheelEvent = static_cast<const QWheelEvent*>(event);
        if (wheelEvent->delta() > 0)
            this->zoomIn();
        else
            this->zoomOut();

        break;
    }
    default:
        break;
    } // end switch

    return false;
}

void QtOccViewController::setViewCursor(const QCursor &cursor)
{
    if (m_widgetView)
        m_widgetView->setCursor(cursor);
}

struct QtOccViewController::RubberBand : public BaseV3dViewController::AbstractRubberBand {
    RubberBand(QWidget* parent)
        : m_rubberBand(QRubberBand::Rectangle, parent)
    {
        // QWidget::setStyle() is important, set to windows style will just draw
        // rectangle frame, otherwise will draw a solid rectangle.
        m_rubberBand.setStyle(QStyleFactory::create("windows"));
    }

    void updateGeometry(const QRect& rect) override {
        m_rubberBand.setGeometry(rect);
    }

    void setVisible(bool on) override {
        m_rubberBand.setVisible(on);
    }

private:
    QRubberBand m_rubberBand;
};

BaseV3dViewController::AbstractRubberBand* QtOccViewController::createRubberBand()
{
    return new RubberBand(m_widgetView);
}

BaseV3dViewController::BaseV3dViewController(
        const Handle_V3d_View &view, QObject *parent)
    : QObject(parent),
      m_view(view)
{
}

BaseV3dViewController::~BaseV3dViewController()
{
    delete m_rubberBand;
}

void BaseV3dViewController::zoomIn()
{
    m_view->SetScale(m_view->Scale() * 1.1); // +10%
    emit viewScaled();
}

void BaseV3dViewController::zoomOut()
{
    m_view->SetScale(m_view->Scale() / 1.1); // -10%
    emit viewScaled();
}

void BaseV3dViewController::startDynamicAction(DynamicAction dynAction)
{
    if (dynAction == DynamicAction::None)
        return;

    if (m_dynamicAction != DynamicAction::None)
        return;

    m_dynamicAction = dynAction;
    emit dynamicActionStarted(dynAction);
}

void BaseV3dViewController::stopDynamicAction()
{
    if (m_dynamicAction != DynamicAction::None) {
        emit dynamicActionEnded(m_dynamicAction);
        m_dynamicAction = DynamicAction::None;
    }
}

bool BaseV3dViewController::isRotationStarted() const
{
    return m_dynamicAction == DynamicAction::Rotation;
}

bool BaseV3dViewController::isPanningStarted() const
{
    return m_dynamicAction == DynamicAction::Panning;
}

bool BaseV3dViewController::isWindowZoomingStarted() const
{
    return m_dynamicAction == DynamicAction::WindowZoom;
}

void BaseV3dViewController::drawRubberBand(const QPoint& posMin, const QPoint& posMax)
{
    if (!m_rubberBand)
        m_rubberBand = this->createRubberBand();

    QRect rect;
    rect.setX(std::min(posMin.x(), posMax.x()));
    rect.setY(std::min(posMin.y(), posMax.y()));
    rect.setWidth(std::abs(posMax.x() - posMin.x()));
    rect.setHeight(std::abs(posMax.y() - posMin.y()));
    m_rubberBand->updateGeometry(rect);
    m_rubberBand->setVisible(true);
}

void BaseV3dViewController::hideRubberBand()
{
    if (m_rubberBand)
        m_rubberBand->setVisible(false);
}

void BaseV3dViewController::windowFitAll(const QPoint& posMin, const QPoint& posMax)
{
    if (std::abs(posMin.x() - posMax.x()) > 1 || std::abs(posMin.y() - posMax.y()) > 1)
        m_view->WindowFitAll(posMin.x(), posMin.y(), posMax.x(), posMax.y());
}

BaseV3dViewController::DynamicAction BaseV3dViewController::currentDynamicAction() const
{
    return m_dynamicAction;
}

bool BaseV3dViewController::hasCurrentDynamicAction() const
{
    return m_dynamicAction != DynamicAction::None;
}

} // namespace Mayo

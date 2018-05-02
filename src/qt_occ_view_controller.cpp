/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "qt_occ_view_controller.h"

#include "widget_occ_view.h"

#include <QtCore/QDebug>
#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

#include <V3d_View.hxx>

namespace Mayo {

namespace Internal {

static const QCursor& rotateCursor()
{
    static QCursor cursor;
    if (cursor.bitmap() == nullptr) {
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
        return BaseV3dViewController::eventFilter(watched, event);
    Handle_V3d_View view = m_widgetView->v3dView();

    //const Qt::KeyboardModifiers keybMods = QApplication::queryKeyboardModifiers();

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = m_widgetView->mapFromGlobal(mouseEvent->globalPos());
        m_prevPos = currPos;
        if (mouseEvent->button() == Qt::LeftButton) {
            view->StartRotation(currPos.x(), currPos.y());
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        auto mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = m_widgetView->mapFromGlobal(mouseEvent->globalPos());
        const QPoint prevPos = m_prevPos;
        m_prevPos = currPos;
        emit mouseMoved(currPos);

        if (QApplication::mouseButtons() == Qt::LeftButton) {
            this->setViewCursor(Internal::rotateCursor());
            view->Rotation(currPos.x(), currPos.y());
            this->setStateRotation(true);
            return true;
        }
        else if (QApplication::mouseButtons() == Qt::RightButton) {
            this->setViewCursor(Qt::SizeAllCursor);
            view->Pan(currPos.x() - prevPos.x(), prevPos.y() - currPos.y());
            this->setStatePanning(true);
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        this->setViewCursor(Qt::ArrowCursor);
        this->setStateRotation(false);
        this->setStatePanning(false);
        return true;
    }
    case QEvent::Wheel: {
        auto wheelEvent = static_cast<const QWheelEvent*>(event);
        Standard_Real currentScale = view->Scale();
        if (wheelEvent->delta() > 0)
            currentScale *= 1.1; // +10%
        else
            currentScale /= 1.1; // -10%
        view->SetScale(currentScale);
        emit viewScaled();
        return true;
    }
    default: return false;
    } // end switch

    return false;
}

void QtOccViewController::setViewCursor(const QCursor &cursor)
{
    if (m_widgetView != nullptr)
        m_widgetView->setCursor(cursor);
}

BaseV3dViewController::BaseV3dViewController(
        const Handle_V3d_View &view, QObject *parent)
    : QObject(parent),
      m_view(view)
{
}

bool BaseV3dViewController::isRotating() const
{
    return m_stateRotation;
}

bool BaseV3dViewController::isPanning() const
{
    return m_statePanning;
}

void BaseV3dViewController::setStateRotation(bool on)
{
    if (m_stateRotation != on) {
        if (on)
            emit viewRotationStarted();
        else
            emit viewRotationEnded();
        m_stateRotation = on;
    }
}

void BaseV3dViewController::setStatePanning(bool on)
{
    if (m_statePanning != on) {
        if (on)
            emit viewPanningStarted();
        else
            emit viewPanningEnded();
        m_statePanning = on;
    }
}

} // namespace Mayo

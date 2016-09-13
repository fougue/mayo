#include "qt_occ_view_controller.h"

#include "qt_occ_view.h"

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

QtOccViewController::QtOccViewController(QtOccView* view)
    : QObject(view),
      m_view(view)
{
    if (view != nullptr)
        view->installEventFilter(this);
}

bool QtOccViewController::eventFilter(QObject* watched, QEvent* event)
{
    auto view = qobject_cast<const QtOccView*>(watched);
    if (view != m_view)
        return QObject::eventFilter(watched, event);
    Handle_V3d_View occView = view->occV3dView();

    //const Qt::KeyboardModifiers keybMods = QApplication::queryKeyboardModifiers();

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        const QMouseEvent* mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = view->mapFromGlobal(mouseEvent->globalPos());
        m_prevPos = currPos;
        if (mouseEvent->button() == Qt::LeftButton) {
            occView->StartRotation(currPos.x(), currPos.y());
            return true;
        }
        break;
    }
    case QEvent::MouseMove: {
        const QMouseEvent* mouseEvent = static_cast<const QMouseEvent*>(event);
        const QPoint currPos = view->mapFromGlobal(mouseEvent->globalPos());
        const QPoint prevPos = m_prevPos;
        m_prevPos = currPos;

        if (QApplication::mouseButtons() == Qt::LeftButton) {
            this->setViewCursor(Internal::rotateCursor());
            occView->Rotation(currPos.x(), currPos.y());
            return true;
        }
        else if (QApplication::mouseButtons() == Qt::RightButton) {
            this->setViewCursor(Qt::SizeAllCursor);
            occView->Pan(currPos.x() - prevPos.x(), prevPos.y() - currPos.y());
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        this->setViewCursor(Qt::ArrowCursor);
        return true;
    }
    case QEvent::Wheel: {
        const QWheelEvent* wheelEvent = static_cast<const QWheelEvent*>(event);
        Standard_Real currentScale = occView->Scale();
        if (wheelEvent->delta() > 0)
            currentScale *= 1.1; // +10%
        else
            currentScale /= 1.1; // -10%
        occView->SetScale(currentScale);
        return true;
    }
    default: return false;
    } // end switch

    return false;
}

void QtOccViewController::setViewCursor(const QCursor &cursor)
{
    if (m_view != nullptr)
        m_view->setCursor(cursor);
}

} // namespace Mayo

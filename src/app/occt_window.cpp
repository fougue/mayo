#include "occt_window.h"

// --
// -- Copy from $OCC7.5.0/samples/qt/Common/src/OcctWindow.h
// --

IMPLEMENT_STANDARD_RTTIEXT(OcctWindow,Aspect_Window)

// =======================================================================
// function : OcctWindow
// purpose  :
// =======================================================================
OcctWindow::OcctWindow ( QWidget* theWidget, const Quantity_NameOfColor theBackColor )
: Aspect_Window(),
  myWidget( theWidget )
{
  SetBackground (theBackColor);
  const auto scale = myWidget->devicePixelRatioF();
  myXLeft   = qRound(scale * myWidget->rect().left());
  myYTop    = qRound(scale * myWidget->rect().top());
  myXRight  = qRound(scale * myWidget->rect().right());
  myYBottom = qRound(scale * myWidget->rect().bottom());
}

// =======================================================================
// function : Destroy
// purpose  :
// =======================================================================
void OcctWindow::Destroy()
{
  myWidget = NULL;
}

// =======================================================================
// function : NativeParentHandle
// purpose  :
// =======================================================================
Aspect_Drawable OcctWindow::NativeParentHandle() const
{
  QWidget* aParentWidget = myWidget->parentWidget();
  if ( aParentWidget != NULL )
    return (Aspect_Drawable)aParentWidget->winId();
  else
    return 0;
}

// =======================================================================
// function : NativeHandle
// purpose  :
// =======================================================================
Aspect_Drawable OcctWindow::NativeHandle() const
{
  return (Aspect_Drawable)myWidget->winId();
}

// =======================================================================
// function : IsMapped
// purpose  :
// =======================================================================
Standard_Boolean OcctWindow::IsMapped() const
{
  return !( myWidget->isMinimized() || myWidget->isHidden() );
}

// =======================================================================
// function : Map
// purpose  :
// =======================================================================
void OcctWindow::Map() const
{
  myWidget->show();
  myWidget->update();
}

// =======================================================================
// function : Unmap
// purpose  :
// =======================================================================
void OcctWindow::Unmap() const
{
  myWidget->hide();
  myWidget->update();
}

// =======================================================================
// function : DoResize
// purpose  :
// =======================================================================
#if OCC_VERSION_HEX >= 0x070500
Aspect_TypeOfResize OcctWindow::DoResize()
#else
Aspect_TypeOfResize OcctWindow::DoResize() const
#endif
{
  int                 aMask = 0;
  Aspect_TypeOfResize aMode = Aspect_TOR_UNKNOWN;

  const auto scale = myWidget->devicePixelRatioF();
  if ( !myWidget->isMinimized() )
  {
    if ( Abs ( scale * myWidget->rect().left()   - myXLeft   ) > 2 ) aMask |= 1;
    if ( Abs ( scale * myWidget->rect().right()  - myXRight  ) > 2 ) aMask |= 2;
    if ( Abs ( scale * myWidget->rect().top()    - myYTop    ) > 2 ) aMask |= 4;
    if ( Abs ( scale * myWidget->rect().bottom() - myYBottom ) > 2 ) aMask |= 8;

    switch ( aMask )
    {
      case 0:
        aMode = Aspect_TOR_NO_BORDER;
        break;
      case 1:
        aMode = Aspect_TOR_LEFT_BORDER;
        break;
      case 2:
        aMode = Aspect_TOR_RIGHT_BORDER;
        break;
      case 4:
        aMode = Aspect_TOR_TOP_BORDER;
        break;
      case 5:
        aMode = Aspect_TOR_LEFT_AND_TOP_BORDER;
        break;
      case 6:
        aMode = Aspect_TOR_TOP_AND_RIGHT_BORDER;
        break;
      case 8:
        aMode = Aspect_TOR_BOTTOM_BORDER;
        break;
      case 9:
        aMode = Aspect_TOR_BOTTOM_AND_LEFT_BORDER;
        break;
      case 10:
        aMode = Aspect_TOR_RIGHT_AND_BOTTOM_BORDER;
        break;
      default:
        break;
    }  // end switch

#if OCC_VERSION_HEX >= 0x070500
    OcctWindow* mutableThis = this;
#else
    OcctWindow* mutableThis = const_cast<OcctWindow*>(this);
#endif

    mutableThis->myXLeft   = qRound(scale * myWidget->rect().left());
    mutableThis->myXRight  = qRound(scale * myWidget->rect().right());
    mutableThis->myYTop    = qRound(scale * myWidget->rect().top());
    mutableThis->myYBottom = qRound(scale * myWidget->rect().bottom());
  }

  return aMode;
}

// =======================================================================
// function : Ratio
// purpose  :
// =======================================================================
Standard_Real OcctWindow::Ratio() const
{
  QRect aRect = myWidget->rect();
  return Standard_Real( aRect.right() - aRect.left() ) / Standard_Real( aRect.bottom() - aRect.top() );
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
void OcctWindow::Size ( Standard_Integer& theWidth, Standard_Integer& theHeight ) const
{
  QRect aRect = myWidget->rect();
  const auto scale = myWidget->devicePixelRatioF();
  theWidth  = qRound(scale * aRect.width());
  theHeight = qRound(scale * aRect.height());
}

// =======================================================================
// function : Position
// purpose  :
// =======================================================================
void OcctWindow::Position ( Standard_Integer& theX1, Standard_Integer& theY1,
                            Standard_Integer& theX2, Standard_Integer& theY2 ) const
{
  const auto scale = myWidget->devicePixelRatioF();
  theX1 = qRound(scale * myWidget->rect().left());
  theX2 = qRound(scale * myWidget->rect().right());
  theY1 = qRound(scale * myWidget->rect().top());
  theY2 = qRound(scale * myWidget->rect().bottom());
}

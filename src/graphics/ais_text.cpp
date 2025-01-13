/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "ais_text.h"

#include <gp_Pnt.hxx>
#include <Graphic3d_AspectText3d.hxx>
#include <OSD_Environment.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_TextAspect.hxx>
#include <SelectMgr_SelectableObject.hxx>
#include <SelectMgr_Selection.hxx>
#include <Standard_Version.hxx>
#include <Standard_CString.hxx>
#include <Standard_Integer.hxx>
#include <Standard_IStream.hxx>
#include <Standard_OStream.hxx>
#include <Standard_Real.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>

#if OCC_VERSION_HEX < 0x070600
#  include <Quantity_Factor.hxx>
#  include <Quantity_PlaneAngle.hxx>
#endif

#include <vector>

namespace Mayo {

AIS_Text::AIS_Text(const TCollection_ExtendedString &text, const gp_Pnt& pos)
{
    TextProperties defaultProps;
    m_textProps.push_back(defaultProps);
    this->setText(text);
    this->setPosition(pos);
}

OccHandle<Prs3d_TextAspect> AIS_Text::presentationTextAspect(unsigned i) const
{
    return this->isValidTextIndex(i) ? m_textProps.at(i).m_aspect : OccHandle<Prs3d_TextAspect>();
}

OccHandle<Graphic3d_AspectText3d> AIS_Text::graphicTextAspect(unsigned i) const
{
    return this->isValidTextIndex(i) ? m_textProps.at(i).m_aspect->Aspect() : OccHandle<Graphic3d_AspectText3d>();
}

gp_Pnt AIS_Text::position(unsigned i) const
{
    return this->isValidTextIndex(i) ? m_textProps.at(i).m_position : gp_Pnt();
}

TCollection_ExtendedString AIS_Text::text(unsigned i) const
{
    return this->isValidTextIndex(i) ? m_textProps.at(i).m_text : TCollection_ExtendedString();
}

bool AIS_Text::isValidTextIndex(unsigned i) const
{
    return i < this->textCount();
}

unsigned AIS_Text::textCount() const
{
    return static_cast<unsigned>(m_textProps.size());
}

void AIS_Text::addText(const TCollection_ExtendedString& text, const gp_Pnt& pos)
{
    TextProperties newProps;
    m_textProps.push_back(newProps);
    const unsigned i = this->textCount() - 1;

    this->presentationTextAspect(i)->SetColor(m_defaultColor);
    this->presentationTextAspect(i)->SetFont(m_defaultFont);

    this->setPosition(pos, i);
    this->setText(text, i);
    this->setTextDisplayMode(m_defaultTextDisplayMode, i);
    this->setTextStyle(m_defaultTextStyle, i);
    this->setTextBackgroundColor(m_defaultTextBackgroundColor, i);
}

void AIS_Text::setPosition(const gp_Pnt& pos, unsigned i)
{
    if (this->isValidTextIndex(i))
        m_textProps.at(i).m_position = pos;
}

void AIS_Text::setText(const TCollection_ExtendedString &v, unsigned i)
{
    if (this->isValidTextIndex(i))
        m_textProps.at(i).m_text = v;
}

//! Only works when the i-th text display mode is set to Aspect_TODT_SUBTITLE
void AIS_Text::setTextBackgroundColor(const Quantity_Color& color, unsigned i)
{
    if (this->isValidTextIndex(i))
        this->graphicTextAspect(i)->SetColorSubTitle(color);
}

void AIS_Text::setTextDisplayMode(Aspect_TypeOfDisplayText mode, unsigned i)
{
    if (this->isValidTextIndex(i))
        this->graphicTextAspect(i)->SetDisplayType(mode);
}

void AIS_Text::setTextStyle(Aspect_TypeOfStyleText style, unsigned i)
{
    if (this->isValidTextIndex(i))
        this->graphicTextAspect(i)->SetStyle(style);
}

void AIS_Text::setDefaultColor(const Quantity_Color &c)
{
    m_defaultColor = c;
}

void AIS_Text::setDefaultFont(const char *fontName)
{
    m_defaultFont = fontName;
}

void AIS_Text::setDefaultTextBackgroundColor(const Quantity_Color& c)
{
    m_defaultTextBackgroundColor = c;
}

void AIS_Text::setDefaultTextDisplayMode(Aspect_TypeOfDisplayText mode)
{
    m_defaultTextDisplayMode = mode;
}

void AIS_Text::setDefaultTextStyle(Aspect_TypeOfStyleText style)
{
    m_defaultTextStyle = style;
}

void AIS_Text::Compute(
        const OccHandle<PrsMgr_PresentationManager>&,
        const OccHandle<Prs3d_Presentation>& pres,
        const int)
{
    for (unsigned i = 0; i < this->textCount(); ++i) {
        Prs3d_Text::Draw(
#if OCC_VERSION_HEX >= 0x070400
                    pres->CurrentGroup(),
#else
                    Prs3d_Root::CurrentGroup(pres),
#endif
                    this->presentationTextAspect(i),
                    this->text(i),
                    this->position(i)
        );
    }
}

void AIS_Text::ComputeSelection(const OccHandle<SelectMgr_Selection>&, const int)
{
}

AIS_Text::TextProperties::TextProperties()
    : m_aspect(new Prs3d_TextAspect)
{
}

bool AIS_Text::TextProperties::operator==(const AIS_Text::TextProperties& other) const
{
    return m_font == other.m_font
            && m_position.SquareDistance(other.m_position) < Precision::Confusion()
            && m_text == other.m_text
            && m_aspect == other.m_aspect;
}

} // namespace Mayo

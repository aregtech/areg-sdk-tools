#ifndef LUSAN_VIEW_SM_SMKINDGLYPH_HPP
#define LUSAN_VIEW_SM_SMKINDGLYPH_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMKindGlyph.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM design surfaces: the kind marks of a stimulus or operation.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QColor;
class QIcon;
class QPainter;
class QRectF;
class SMOperationBase;
class SMTransitionEntry;

/**
 * \namespace   SMKindGlyph
 * \brief       The one place that decides how a state machine surface announces WHAT a row is:
 *              an event send, a timer start or stop, a trigger, a plain action, or the
 *              enter/exit/do band a row belongs to.
 *
 *              Every design surface -- the state box body rows, the transition edge label, and
 *              the Properties stimulus picker -- draws its marks from here, so a change lands on
 *              all three at once. \ref Style selects between the drawn glyphs and a plain-word
 *              fallback (`event`, `timer`), which is the escape hatch if the pictographs read
 *              badly on a dense diagram.
 **/
namespace SMKindGlyph
{
    /**
     * \enum    eGlyph
     * \brief   What a mark stands for. The first three are BAND marks (which operation list a row
     *          belongs to); the rest are KIND marks (what the row itself does).
     **/
    enum class eGlyph
    {
          None          //!< No mark at all.
        , Entry         //!< The state's entry band: an arrow running INTO a bar, `->|`.
        , Exit          //!< The state's exit band: an arrow running away from a bar, `<-|`.
        , ExitAlt       //!< The alternative exit mark, `|<-` -- see \c ExitBandGlyph in the .cpp.
        , Internal      //!< The do band and an internal transition: a self-loop.
        , Trigger       //!< A trigger method as a stimulus: a small inbound call arrow.
        , Event         //!< An event, sent or awaited: a lightning bolt.
        , TimerStart    //!< A timer start: a clock face with a play triangle.
        , TimerStop     //!< A timer stop: a clock face with a stop square.
    };

    /**
     * \enum    eStyle
     * \brief   How a kind mark is rendered.
     **/
    enum class eStyle
    {
          Glyph //!< The drawn pictograph (default).
        , Word  //!< A lowercase word instead -- `event`, `timer`, `trigger`.
    };

    /**
     * \brief   THE switch. Change this one line to move every design surface from the drawn
     *          pictographs to plain words; nothing else in the code has to change, because both
     *          \ref paint and \ref prefix honour it.
     **/
    constexpr eStyle    Style       { eStyle::Glyph };

    //!< The side of a drawn mark in device-independent pixels (the body-row and edge-label size).
    constexpr int       GlyphSize   { 12 };

    /**
     * \brief   Draws the mark centered in \p rect, in \p color. Does nothing for \c eGlyph::None,
     *          and nothing at all while \ref Style is \c Word -- the caller then uses \ref prefix.
     **/
    void paint(QPainter& painter, const QRectF& rect, eGlyph glyph, const QColor& color);

    /**
     * \brief   The mark as a standalone icon, for a widget that takes a QIcon rather than a
     *          painter (the stimulus picker rows, a signature label).
     * \param   glyph   The mark to render.
     * \param   color   The stroke / fill color, normally the widget's text color.
     * \param   size    The icon side in device-independent pixels.
     **/
    QIcon icon(eGlyph glyph, const QColor& color, int size = GlyphSize);

    /**
     * \brief   The lowercase word for a kind mark (`event`, `timer`, `trigger`), or empty for a
     *          band mark and for \c None. Independent of \ref Style, so a tooltip may spell out
     *          what a drawn mark means.
     **/
    QString word(eGlyph glyph);

    /**
     * \brief   The text a surface puts in FRONT of a name to announce its kind: `"event "` while
     *          \ref Style is \c Word, and nothing while it is \c Glyph (the drawn mark says it).
     *          A surface that cannot draw at all -- a plain text line, a tooltip -- calls
     *          \ref word directly instead.
     **/
    QString prefix(eGlyph glyph);

    /**
     * \brief   True when \p glyph reserves drawing room, i.e. a mark is drawn for it. False for
     *          \c None and for the whole \c Word style, so a caller can lay out either way.
     **/
    bool isDrawn(eGlyph glyph);

    //!< The exit-band mark actually in use. Two are implemented (`<-|` and `|<-`); the choice is
    //!< the \c ExitBandGlyph line in the .cpp, and every surface asks here rather than picking one.
    eGlyph exitGlyph();

    //!< The mark of a transition's stimulus (trigger / event / timer). \c None when it has none.
    eGlyph stimulusGlyph(const SMTransitionEntry& transition);

    //!< The mark of one operation: the lightning for a send, a clock for a timer, \c None otherwise
    //!< (a plain action call and an attribute assignment take their band's mark instead).
    eGlyph operationGlyph(const SMOperationBase& op);
}

#endif  // LUSAN_VIEW_SM_SMKINDGLYPH_HPP

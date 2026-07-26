#ifndef LUSAN_VIEW_SM_SMSUBMACHINEPEEK_HPP
#define LUSAN_VIEW_SM_SMSUBMACHINEPEEK_HPP
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
 *  \file        lusan/view/sm/SMSubmachinePeek.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \brief       Lusan application, FSM design canvas submachine quick view.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMState.hpp"

#include <QList>
#include <QRectF>
#include <QString>

/**
 * \class   SMSubmachinePeek
 * \brief   The submachine quick view: a fixed-size popup that draws the SHAPES of one nested
 *          level -- Start marker, states, Final marker -- and nothing else. No names, no
 *          transitions, no detail of any kind.
 *
 *          It answers one question, the one the state box itself cannot: "which of my composite
 *          states is the one I am thinking of?" A user who remembers a level by its silhouette
 *          finds it here without navigating into every candidate in turn. Fixed size is the whole
 *          point -- a level with a million states must cost exactly what a level with three costs,
 *          so the content is scaled to the popup rather than the popup grown to the content.
 **/
class SMSubmachinePeek : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \struct  Shape
     * \brief   One substate reduced to what the quick view draws: where it sits and what it is.
     **/
    struct Shape
    {
        QRectF                      rect;   //!< The substate's layout box, in the level's units.
        SMStateEntry::eStateKind    kind;   //!< Marker or normal state -- it picks the outline.
    };

    //!< The popup's fixed content size, in device-independent pixels.
    static constexpr int    PeekWidth   { 260 };
    static constexpr int    PeekHeight  { 170 };

    /**
     * \brief   How many shapes are drawn at most. Past this the popup is a texture, not an
     *          overview -- and the point of a fixed-size view is a fixed cost. The caller reports
     *          the true total, so the count line stays honest about what was left out.
     **/
    static constexpr int    MaxShapes   { 300 };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMSubmachinePeek(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows the quick view of \p title near \p globalPos, kept fully on its screen.
     * \param   title       The owning state's name (the one text the view carries).
     * \param   shapes      The substate shapes to draw; more than \ref MaxShapes are dropped.
     * \param   total       The true substate count, which may exceed \p shapes.
     * \param   globalPos   The pointer position the popup is placed beside.
     **/
    void showFor(const QString& title, const QList<Shape>& shapes, int total, const QPoint& globalPos);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void paintEvent(QPaintEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString         mTitle;     //!< The owning state's name.
    QList<Shape>    mShapes;    //!< The drawn shapes (already capped).
    int             mTotal;     //!< The true substate count.
};

#endif  // LUSAN_VIEW_SM_SMSUBMACHINEPEEK_HPP

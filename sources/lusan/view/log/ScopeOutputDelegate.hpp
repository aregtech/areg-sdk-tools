#ifndef LUSAN_VIEW_LOG_SCOPEOUTPUTDELEGATE_HPP
#define LUSAN_VIEW_LOG_SCOPEOUTPUTDELEGATE_HPP
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
 *  \file        lusan/view/log/ScopeOutputDelegate.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the call structure drawn beside the scope output rows.
 *
 ************************************************************************/

#include <QStyledItemDelegate>

/**
 * \brief   Draws the call structure of the scope output window: the rails of the calls that
 *          enclose a row, the bracket that runs from a scope enter to its exit, and the handle
 *          that folds a call away.
 **/
class ScopeOutputDelegate : public QStyledItemDelegate
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< The width of the zone the structure is drawn in, in pixels.
    static constexpr int    GutterWidth { 16 };

    //!< The distance between the rails of two neighbouring call levels, in pixels.
    static constexpr int    LevelStep   { 4 };

    //!< The deepest level that still moves its rail. Everything below shares the last one.
    static constexpr int    LevelMax    { 4 };

    //!< The edge of the square the fold handle is drawn in, in pixels.
    static constexpr int    HandleSide  { 9 };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ScopeOutputDelegate(QObject* parent = nullptr);
    virtual ~ScopeOutputDelegate(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Draws the cell, keeping the text of the leading column clear of the structure.
     **/
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * \brief   Opens or closes the call of the row when the fold handle is pressed.
     * \return  True when the press belonged to the handle.
     **/
    virtual bool editorEvent( QEvent* event
                            , QAbstractItemModel* model
                            , const QStyleOptionViewItem& option
                            , const QModelIndex& index) override;

    /**
     * \brief   Returns the square the fold handle of a row is drawn in.
     * \param   rect    The rectangle of the cell of the leading column.
     **/
    static QRect handleRect(const QRect& rect);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Draws the rails, the bracket and the fold handle of the row.
    void _paintStructure(QPainter& painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif  // LUSAN_VIEW_LOG_SCOPEOUTPUTDELEGATE_HPP

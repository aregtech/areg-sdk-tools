#ifndef LUSAN_VIEW_COMMON_SCOPENAMEDELEGATE_HPP
#define LUSAN_VIEW_COMMON_SCOPENAMEDELEGATE_HPP
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
 *  \file        lusan/view/common/ScopeNameDelegate.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The delegate that marks the matched part of a scope name.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include <QStyledItemDelegate>

#include "areg/base/areg_global.h"

#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QModelIndex;
class QRect;
class QPainter;
class QStyleOptionViewItem;

//////////////////////////////////////////////////////////////////////////
// ScopeNameDelegate class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   Draws the scope name the way the active style would, and adds the two marks the
 *          name column carries: the wash over the parts that match the filter or the find
 *          box, and the dot that says what a running target knows about its priorities.
 **/
class ScopeNameDelegate : public QStyledItemDelegate
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit ScopeNameDelegate(QObject* parent = nullptr);
    virtual ~ScopeNameDelegate(void) = default;

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the text to mark inside the scope names.
     * \param   needle      The text to mark. An empty text marks nothing.
     * \param   sensitivity Whether the case of the text matters.
     * \return  True if the mark changed and the tree has to be repainted.
     **/
    bool setNeedle(const QString& needle, Qt::CaseSensitivity sensitivity);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:

    //!< Washes every part of the name that matches the text the marks are drawn on.
    void paintMatches(QPainter& painter, const QStyleOptionViewItem& option, const QRect& box) const;

    //!< Draws the dot that says what the target of a process knows about its priorities.
    void paintTargetState(QPainter& painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QString             mNeedle;        //!< The text the marks are drawn on
    Qt::CaseSensitivity mSensitivity;   //!< Whether the case of the text matters

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
    AREG_NOCOPY_NOMOVE(ScopeNameDelegate);
};

#endif  // LUSAN_VIEW_COMMON_SCOPENAMEDELEGATE_HPP

#ifndef LUSAN_VIEW_SM_SMFIXBAR_HPP
#define LUSAN_VIEW_SM_SMFIXBAR_HPP
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
 *  \file        lusan/view/sm/SMFixBar.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard quick-fix bar (v7 B2).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QHBoxLayout;
class QLabel;

/**
 * \class   SMFixBar
 * \brief   The transient quick-fix row under the status line (B2): a leading `(!)` message and
 *          a set of flat link-buttons the field builds per diagnostic (did-you-mean, declare,
 *          keep-as-raw, shadowing fixes, warning suppression; `Map arguments...` shows disabled
 *          with the Session U3 tooltip). The bar is a dumb presenter: it renders the fix list it
 *          is given and emits the chosen fix's id + payload; the field owns the resolution.
 *          Esc / the `x` button hides it; the field re-opens it at the caret with Ctrl+.
 **/
class SMFixBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * \struct  Fix
     * \brief   One offered quick-fix.
     **/
    struct Fix
    {
        QString id;                 //!< The stable id the field switches on.
        QString label;              //!< The button caption.
        QString payload;            //!< Fix-specific data (e.g. the suggested name).
        bool    enabled { true };   //!< False shows the button disabled (e.g. Session U3).
        QString tooltip;            //!< The button tooltip.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMFixBar(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Shows \p message + the \p fixes; an empty list hides the bar.
    void setFixes(const QString& message, const QList<Fix>& fixes);

    //!< Hides the bar (Esc / commit / no diagnostic).
    void dismiss();

signals:
    //!< A fix button was pressed.
    void triggered(const QString& id, const QString& payload);
    //!< The bar was dismissed (Esc / `x`).
    void dismissed();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void clearButtons();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QHBoxLayout*    mLayout;     //!< The row layout.
    QLabel*         mMessage;    //!< The `(!)` diagnostic message.
};

#endif  // LUSAN_VIEW_SM_SMFIXBAR_HPP

#ifndef LUSAN_VIEW_COMMON_EDITCANCELFILTER_HPP
#define LUSAN_VIEW_COMMON_EDITCANCELFILTER_HPP
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
 *  \file        lusan/view/common/EditCancelFilter.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Escape-to-cancel behaviour for details line editors.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLineEdit;

//////////////////////////////////////////////////////////////////////////
// EditCancelFilter class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The EditCancelFilter adds "operation cancel" semantics to a QLineEdit that is
 *          synchronized live with a model / table while the user types.
 *
 *          A details field mirrors every keystroke into the model and the list row, so pressing
 *          Escape must roll the value back to what it was before the edit began. A bare QLineEdit
 *          ignores Escape entirely, so the typed text would otherwise stick. This filter:
 *
 *          - captures the baseline value when the field gains focus and again whenever the edit is
 *            committed (Enter / editing finished);
 *          - on Escape, restores the baseline via setText(), which re-emits textChanged and lets the
 *            existing live-sync push the original value back into the model, the table and the field.
 *
 *          The filter is controller-agnostic: it only touches the line edit's own text and relies on
 *          the already-wired live-sync signal, so it works identically for the direct-mutation and
 *          the undo-command page controllers. It owns nothing and is parented to the line edit.
 **/
class EditCancelFilter : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Installs Escape-to-cancel behaviour on the given line edit. The created filter is
     *          parented to (and destroyed with) the line edit, so the caller need not retain it.
     * \param   edit    The line edit to guard; ignored when nullptr.
     **/
    static void install(QLineEdit* edit);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit EditCancelFilter(QLineEdit* edit);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Tracks focus / commit to keep the baseline current and reverts the field on Escape.
     * \param   watched The guarded line edit.
     * \param   event   The event to inspect.
     * \return  True to consume the event (Escape), false to pass it through.
     **/
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    QLineEdit*  mEdit;          //!< The guarded line edit.
    QString     mBaseline;      //!< The value to restore on Escape (last committed / focus-in text).
    bool        mHasBaseline;   //!< True once a baseline has been captured for the current edit.
};

#endif // LUSAN_VIEW_COMMON_EDITCANCELFILTER_HPP

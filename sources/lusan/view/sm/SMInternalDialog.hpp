#ifndef LUSAN_VIEW_SM_SMINTERNALDIALOG_HPP
#define LUSAN_VIEW_SM_SMINTERNALDIALOG_HPP
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
 *  \file        lusan/view/sm/SMInternalDialog.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM internal transitions dialog (context-menu access path).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QDialog>

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class SMInternalEditor;

/**
 * \class   SMInternalDialog
 * \brief   A thin dialog wrapper around \ref SMInternalEditor, opened from the canvas context menu
 *          (state `Internal Transitions...`), exactly as \ref SMOperationsDialog wraps the
 *          operations editor for `Enter Actions...` and `Exit Actions...`. The three entries of a
 *          state's context menu therefore behave alike, and each hosts the very same editor the
 *          Properties panel embeds -- one implementation and one undo path per construct.
 **/
class SMInternalDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * \brief   Opens the internal transitions of \p stateId, with \p transitionId selected when it
     *          is one of them (0 selects the first).
     **/
    SMInternalDialog( StateMachineModel& model
                    , const QString& title
                    , uint32_t stateId
                    , uint32_t transitionId = 0u
                    , QWidget* parent = nullptr);

    inline SMInternalEditor* editor() const;

private:
    SMInternalEditor* mEditor;
};

inline SMInternalEditor* SMInternalDialog::editor() const
{
    return mEditor;
}

#endif  // LUSAN_VIEW_SM_SMINTERNALDIALOG_HPP

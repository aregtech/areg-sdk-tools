#ifndef LUSAN_VIEW_SM_SMOPERATIONSDIALOG_HPP
#define LUSAN_VIEW_SM_SMOPERATIONSDIALOG_HPP
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
 *  \file        lusan/view/sm/SMOperationsDialog.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operations editor dialog (context-menu access path).
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
class SMOperationsEditor;
class SMOperationList;
class ElementBase;
enum class eDocElementKind;

/**
 * \class   SMOperationsDialog
 * \brief   A thin dialog wrapper around \ref SMOperationsEditor, opened from the canvas
 *          context menu (transition "Operations...", state "Enter/Exit Actions..."). It hosts
 *          the very same editor the Properties panel embeds, so both access paths share one
 *          implementation and one undo path.
 **/
class SMOperationsDialog : public QDialog
{
    Q_OBJECT

public:
    SMOperationsDialog( StateMachineModel& model
                      , const QString& title
                      , uint32_t ownerId
                      , eDocElementKind ownerKind
                      , uint32_t scopeTransition
                      , ElementBase* owner
                      , SMOperationList* list
                      , QWidget* parent = nullptr);

    inline SMOperationsEditor* editor() const;

private:
    SMOperationsEditor* mEditor;
};

inline SMOperationsEditor* SMOperationsDialog::editor() const
{
    return mEditor;
}

#endif  // LUSAN_VIEW_SM_SMOPERATIONSDIALOG_HPP

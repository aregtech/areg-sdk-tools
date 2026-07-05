#ifndef LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP
#define LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/model/sm/SMLayoutCommands.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM layout mutation commands.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/sm/SMCommand.hpp"
#include "lusan/data/sm/SMLayoutData.hpp"

#include <QList>

/**
 * \class   SMMoveNodeCommand
 * \brief   Moves/resizes a state node. Interactive drags push one command per step; they
 *          coalesce via mergeWith() keyed by a gesture ID so the whole drag is a single
 *          undo step and the model is untouched until the gesture is applied.
 **/
class SMMoveNodeCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5401 };

    SMMoveNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t owner, uint32_t gestureId
                      , double x, double y, double width, double height
                      , const QString& text, QUndoCommand* parent = nullptr);

    virtual void redo(void) override;
    virtual void undo(void) override;
    virtual int id(void) const override;
    virtual bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(const SMLayoutNode& geometry);

private:
    uint32_t        mOwner;
    uint32_t        mGesture;
    SMLayoutNode    mNew;
    SMLayoutNode    mOld;
    bool            mCaptured { false };
};

/**
 * \class   SMRemoveLayoutCommand
 * \brief   Removes the View/Node/Edge layout entries owned by a set of element IDs and
 *          restores them on undo. Used as a child of the delete-state composite so a
 *          logical element and its layout are deleted (and undeleted) in one step.
 **/
class SMRemoveLayoutCommand : public SMCommand
{
public:
    SMRemoveLayoutCommand(StateMachineData& data, DocModelNotifier& notifier, const QList<uint32_t>& ownerIds, const QString& text, QUndoCommand* parent = nullptr);

    virtual void redo(void) override;
    virtual void undo(void) override;

private:
    QList<uint32_t>         mIds;
    QList<SMLayoutView>     mViews;
    QList<SMLayoutNode>     mNodes;
    QList<SMLayoutEdge>     mEdges;
    bool                    mCaptured { false };
};

#endif  // LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP

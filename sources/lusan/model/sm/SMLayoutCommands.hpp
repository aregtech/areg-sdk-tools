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
#include <QRectF>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMStateEntry;
class SMTransitionEntry;

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

    /**
     * \brief   Returns a fresh gesture ID. All move sources share this counter so two
     *          different gestures can never coalesce into one undo step.
     **/
    static uint32_t takeNextGesture();

    SMMoveNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                      , uint32_t owner, uint32_t gestureId
                      , double x, double y, double width, double height
                      , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

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
 * \class   SMAttachNodeCommand
 * \brief   Creates the Node layout entry of a freshly added state. The owner ID is read
 *          from the state at redo time, because the ID is allocated only when the sibling
 *          add-state command inserts the entry — so this command always runs as the child
 *          following that insertion inside one composite.
 **/
class SMAttachNodeCommand : public SMCommand
{
public:
    /**
     * \brief   Creates the command.
     * \param   data        The document root.
     * \param   notifier    The change-notification hub.
     * \param   state       The state whose Node entry is created; must outlive the command.
     * \param   geometry    The initial box geometry in scene coordinates.
     * \param   text        The undo-stack display text.
     * \param   parent      The owning composite command.
     **/
    SMAttachNodeCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , const SMStateEntry& state, const QRectF& geometry
                        , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    const SMStateEntry& mState;     //!< The state owning the Node entry.
    QRectF              mGeometry;  //!< The initial box geometry.
};

/**
 * \class   SMSetNodeExpandedCommand
 * \brief   Toggles a state node's expanded/collapsed flag (body visibility).
 **/
class SMSetNodeExpandedCommand : public SMCommand
{
public:
    SMSetNodeExpandedCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t owner, bool expanded
                             , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void apply(bool expanded);

private:
    uint32_t    mOwner;                 //!< The owning state ID.
    bool        mNew;                   //!< The expanded flag to apply.
    bool        mOldExpanded { true };  //!< The previous expanded flag.
    bool        mOldHas      { false }; //!< Whether the flag was present before.
    bool        mCaptured    { false };
};

/**
 * \class   SMAttachEdgeCommand
 * \brief   Creates the Edge layout entry of a freshly added transition. The owner ID is
 *          read from the transition at redo time, because the ID is allocated only when the
 *          sibling add-transition command inserts the entry — so this command always runs as
 *          the child following that insertion inside one composite.
 **/
class SMAttachEdgeCommand : public SMCommand
{
public:
    SMAttachEdgeCommand(  StateMachineData& data, DocModelNotifier& notifier
                        , const SMTransitionEntry& transition, const SMLayoutEdge& geometry
                        , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;

private:
    const SMTransitionEntry&    mTransition;    //!< The transition owning the Edge entry.
    SMLayoutEdge                mGeometry;      //!< The initial edge geometry (owner set at redo).
};

/**
 * \class   SMSetEdgeGeometryCommand
 * \brief   Sets a transition's Edge layout (points, shape, bulge, color, label). Interactive
 *          waypoint/endpoint/label drags push one command per step; they coalesce via
 *          mergeWith() keyed by a gesture ID so the whole drag is a single undo step.
 **/
class SMSetEdgeGeometryCommand : public SMCommand
{
public:
    static constexpr int CMD_ID { 0x5402 };

    SMSetEdgeGeometryCommand(  StateMachineData& data, DocModelNotifier& notifier
                             , uint32_t owner, uint32_t gestureId, const SMLayoutEdge& geometry
                             , const QString& text, QUndoCommand* parent = nullptr);

    void redo() override;
    void undo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyTo(const SMLayoutEdge& geometry, bool present);

private:
    uint32_t        mOwner;
    uint32_t        mGesture;
    SMLayoutEdge    mNew;
    SMLayoutEdge    mOld;
    bool            mHadOld { false };  //!< Whether an Edge entry existed before the first redo.
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

    void redo() override;
    void undo() override;

private:
    QList<uint32_t>         mIds;
    QList<SMLayoutView>     mViews;
    QList<SMLayoutNode>     mNodes;
    QList<SMLayoutEdge>     mEdges;
    bool                    mCaptured { false };
};

#endif  // LUSAN_MODEL_SM_SMLAYOUTCOMMANDS_HPP

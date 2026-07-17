#ifndef LUSAN_VIEW_SM_SMSTRUCTURELENS_HPP
#define LUSAN_VIEW_SM_SMSTRUCTURELENS_HPP
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
 *  \file        lusan/view/sm/SMStructureLens.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard structure lens (v7 B7 / S5): the collapsible
 *               clause-pill rendering of the guard tree, plus the clause popover and the
 *               explain-this-guard list (B11 / S13).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMGuardTree.hpp"

#include <QHash>
#include <QList>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QMenu;
class QTimer;
class QToolButton;
class SMFlowLayout;
class SMHoverCard;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMStructureLens
 * \brief   The structure lens (S5): owner-hued clause pills in a flow layout, clickable
 *          joiner words (one flip per group), `not` prefixes and parens mirroring the
 *          canonical render, a right-click ladder/context menu per pill, Alt+Up/Alt+Down
 *          reorder and Del delete, the `[+ and v]` clause popover (the novice path that
 *          VISIBLY inserts the text it builds into the field), and the explain toggle
 *          (S13). Simple structural edits (negate, flip, wrap, reorder, delete) push guard
 *          commands directly; ladder steps and text insertion are emitted as signals -- the
 *          guard bar owns those flows so the island editor and the lens share one path.
 **/
class SMStructureLens : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    //!< Everything the lens needs to act on one pill.
    struct Pill
    {
        QList<int>          path;           //!< The node's child-index path from the root.
        SMGuardNode::eKind  kind;           //!< The node kind.
        uint32_t            symbolId;       //!< The referenced symbol (Call/Attr/Const/Param).
        QString             body;           //!< The island body (Lambda pills only).
        int                 spanStart;      //!< The node's span in the canonical text.
        int                 spanLength;     //!< The span length.
        QList<int>          groupPath;      //!< The owning group's path (== path when root).
        int                 indexInGroup;   //!< The child index inside the group, or -1.
        int                 groupCount;     //!< The owning group's child count, or 0.
        int                 owner;          //!< The owner hue (restored when a tint clears).
    };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMStructureLens(StateMachineModel& model, QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Points the lens at a transition (0 clears it) and rebuilds.
    void setTransition(uint32_t transitionId);

    //!< The shared hover card (owned by the bar).
    void setHoverCard(SMHoverCard* card);

    /**
     * \brief   The Try-it truth overlay (B9): pill pathName -> truth (1 true, 0 false).
     *          Pills with an entry tint green/red; an empty hash restores the owner hues.
     **/
    void setTruthTints(const QHash<QString, int>& tints);

signals:
    //!< A pill click: select this span of the field text.
    void caretRequested(int start, int length);
    //!< A call pill's second click / menu action: open the mapping grid (S9).
    void gridRequested(const QList<int>& callPath, const QPoint& globalPos);
    //!< The clause popover built a clause: insert it VISIBLY into the field and commit.
    void clauseTextBuilt(const QString& combiner, const QString& clauseText);
    //!< The `+ lambda` menu entry: append an island to the field and open its editor.
    void lambdaAppendRequested();
    //!< Ladder: name the island at \p islandPath (dialog + command are the bar's).
    void nameIslandRequested(const QList<int>& islandPath, const QString& body);
    //!< Ladder: move the lambda condition \p methodId to a handler.
    void moveToHandlerRequested(uint32_t methodId);
    //!< Ladder: adopt a body for the handler condition \p methodId.
    void adoptBodyRequested(uint32_t methodId);
    //!< Ladder: inline the named lambda called at \p callPath back into an island.
    void inlineBodyRequested(const QList<int>& callPath);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementAdded(uint32_t id, eDocElementKind kind);
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();

private:
    void scheduleRebuild();
    void rebuild();

    //!< Recursively emits the widgets of \p node into the flow, mirroring the render walk.
    void buildNode(const SMGuardNode& node, const QList<int>& path, int ctx);

    //!< One clause pill for \p node.
    void addPill(const SMGuardNode& node, const QList<int>& path);

    //!< A clickable joiner / `not` word.
    QToolButton* addWord(const QString& text, const QString& objectName);

    //!< A plain punctuation label (parens).
    void addPunct(const QString& text);

    //!< The pill styling for an owner hue (border 60%, fill 12%, 4 px radius -- B15).
    void stylePill(QToolButton* pill, int owner, const QString& qssClass) const;

    //!< Applies the owner style, overridden by the Try-it truth tint when one is set.
    void applyPillStyle(QToolButton* pill, const Pill& info) const;

    //!< The dominant owner of a clause (the first reference leaf's owner).
    int clauseOwner(const SMGuardNode& node) const;

    void onPillClicked(QToolButton* pill);
    void showPillMenu(QToolButton* pill, const QPoint& globalPos);
    void openClausePopover(const QString& combiner, bool grouped);
    void rebuildExplain();

    //!< The pill record of a lens widget, or nullptr.
    const Pill* pillOf(QObject* widget) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&      mModel;         //!< The document facade.
    uint32_t                mTransitionId;  //!< The shown transition (0 = none).
    QToolButton*            mToggle;        //!< The `v Structure` collapse toggle.
    QToolButton*            mExplainBtn;    //!< The explain toggle (S13).
    QToolButton*            mAddBtn;        //!< The `[+ and v]` split button.
    QWidget*                mContent;       //!< The pill flow host.
    SMFlowLayout*           mFlow;          //!< The pill flow layout.
    QLabel*                 mExplain;       //!< The explain list (S13, hidden by default).
    QLabel*                 mEmptyNote;     //!< Shown when there is no tree to draw.
    SMHoverCard*            mHover;         //!< The shared hover card (not owned).
    QTimer*                 mHoverTimer;    //!< The 300 ms hover delay.
    QToolButton*            mHoverPill;     //!< The pill under the pending hover.
    QList<Pill>             mPills;         //!< The pill records, parallel to the buttons.
    QHash<QObject*, int>    mPillIndex;     //!< Widget -> \ref mPills index.
    QList<int>              mLastClicked;   //!< The last clicked pill's path (grid-on-2nd-click).
    bool                    mRebuildPending;//!< Coalesces deferred rebuilds.
    QHash<QString, int>     mTruthTints;    //!< The Try-it overlay (pathName -> 0/1), empty when closed.
};

#endif  // LUSAN_VIEW_SM_SMSTRUCTURELENS_HPP

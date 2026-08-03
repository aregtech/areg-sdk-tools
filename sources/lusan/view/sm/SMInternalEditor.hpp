#ifndef LUSAN_VIEW_SM_SMINTERNALEDITOR_HPP
#define LUSAN_VIEW_SM_SMINTERNALEDITOR_HPP
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
 *  \file        lusan/view/sm/SMInternalEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM: the internal transitions of one state, edited in place.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMReferences.hpp"

#include <QList>

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class SMTransitionEntry;
class QComboBox;
class QListWidget;
class QLabel;
class QTabWidget;
class QToolButton;
class SMGuardBar;
class SMOperationsEditor;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMInternalEditor
 * \brief   Every internal transition of one state, picked and edited in place: its stimulus, the
 *          operations it runs, and the guard that lets it run.
 *
 *          Laid out as the author reads a transition, and as flat as that allows: WHICH one (a
 *          one-line picker, not a list box that reserves rows a state rarely has), WHAT fires it
 *          (one more line), then `Actions | Conditions` as tabs -- the same two editors the
 *          transition page shows, presented the same way, each taking the whole height that is
 *          left. Accordion sections were tried first and are the wrong container here: a section
 *          holding an editor that carries its OWN accordion stacks three toolbars, and the last
 *          section header then falls off the bottom of a short dock, unreachable without closing
 *          another one. A tab bar cannot be pushed off.
 *
 *          It is the widget BOTH access paths host, exactly as \ref SMOperationsEditor is hosted by
 *          the Properties panel and by \ref SMOperationsDialog: the state page's `Internal` tab
 *          embeds it, and \ref SMInternalDialog wraps it for the canvas context menu. One
 *          implementation, one undo path, and the two can never drift.
 *
 *          The editors it hosts are the real ones -- \ref SMOperationsEditor and \ref SMGuardBar --
 *          because an internal transition IS a transition, and a second, lesser editor for it
 *          would drift from the first.
 **/
class SMInternalEditor : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMInternalEditor(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMInternalEditor();

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds to a state's internal transitions; 0 empties the editor. Re-binding the state
     *          already shown keeps the selected transition, so a refresh never moves the author.
     **/
    void bind(uint32_t stateId);

    //!< Re-reads the bound state's list, keeping \ref currentTransition selected where it can.
    void refresh();

    //!< The internal transition being edited (0 = none).
    inline uint32_t currentTransition() const;

    //!< Selects one of the listed transitions; anything else falls back to the first.
    void setCurrentTransition(uint32_t transitionId);

    //!< How many internal transitions the bound state has.
    int count() const;

    //!< The bound state (0 = none).
    inline uint32_t stateId() const;

    //!< Test/host accessors.
    inline QListWidget* list() const;
    inline QComboBox* stimulusCombo() const;
    inline QLabel* signature() const;
    inline SMOperationsEditor* operations() const;
    inline SMGuardBar* guard() const;
    inline QTabWidget* tabs() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    //!< Carries the Ctrl+Shift click on the stimulus status line to its declaration.
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    //!< The number of internal transitions changed (or a rebind reported it). The Properties tab
    //!< puts it in its label, which is the discoverability the construct never had.
    void countChanged(int count);

    //!< Relayed from the hosted guard bar: a Ctrl+Shift click on a referenced symbol.
    void signalNavigateToDefinition(SMReferences::eTarget kind, uint32_t declId);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onSelected();
    void onStimulusCommit();
    void onAdd();
    void onRemove();
    void onElementChanged(uint32_t id, eDocElementKind kind);

    //!< Moves the selected transition one place up or down its state's PRIORITY order.
    void onMoveUp();
    void onMoveDown();

private:
    //!< Binds the Stimulus row and the Actions / Conditions editors to \p transitionId, or empties
    //!< them for 0.
    void showTransition(uint32_t transitionId);

    //!< The bound state's internal transitions, in priority (document) order.
    QList<SMTransitionEntry*> internals() const;

    /**
     * \brief   One picker row: `#N on <stimulus> [<guard chip>]`.
     *
     *          The number IS the priority -- document order decides which of several transitions on
     *          one stimulus runs -- so it is shown always, not only when two rows would otherwise
     *          collide, and it leads the row so the numbers form a readable column. It is dropped
     *          for a lone transition, where it says nothing. The guard is rendered by the shared
     *          \ref SMGuardRender::chipText, so a long or raw-C++ guard collapses instead of being
     *          chopped, and two transitions on one stimulus never read alike.
     **/
    QString rowLabel(const SMTransitionEntry& transition, int ordinal, int total) const;

    //!< Fills the status line under the Stimulus row: what the stimulus IS, in its own terms.
    void showSignature(const SMTransitionEntry* transition);

    //!< Shows/hides the list and enables the toolbar: a lone transition needs no list to choose
    //!< from and nothing to reorder, but the toolbar stays put so add and remove never move.
    void updateListVisibility(int total);

    //!< Moves the current transition to the position of its \p delta-th internal neighbour.
    void moveBy(int delta);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    QLabel*             mListLabel;     //!< The `Transitions:` caption, hidden with the list.
    QListWidget*        mList;          //!< WHICH internal transition, and in what PRIORITY order.
    QToolButton*        mBtnUp;         //!< Raises the selected one's priority.
    QToolButton*        mBtnDown;       //!< Lowers it.
    QToolButton*        mBtnAdd;        //!< Adds one to the bound state.
    QToolButton*        mBtnRemove;     //!< Removes the selected one.
    QComboBox*          mStimulus;      //!< Its stimulus (kind + name in one closed picker).
    QLabel*             mSignature;     //!< What the stimulus IS: timing, payload or signature.
    QTabWidget*         mTabs;          //!< Actions | Conditions, each at full remaining height.
    SMOperationsEditor* mOperations;    //!< Its operations.
    SMGuardBar*         mGuard;         //!< Its guard -- the same editor the transition page uses.
    uint32_t            mStateId;       //!< The bound state (0 = none).
    uint32_t            mCurrentId;     //!< The internal transition being edited (0 = none).
    bool                mUpdating;      //!< Guards field population against commit signals.

    SMReferences::eTarget mSignatureKind;   //!< What the status line names, for the Ctrl+Shift click.
    uint32_t              mSignatureDecl;   //!< Its declaration ID (0 = nothing to jump to).
};

//////////////////////////////////////////////////////////////////////////
// SMInternalEditor inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMInternalEditor::currentTransition() const
{
    return mCurrentId;
}

inline uint32_t SMInternalEditor::stateId() const
{
    return mStateId;
}

inline QListWidget* SMInternalEditor::list() const
{
    return mList;
}

inline QComboBox* SMInternalEditor::stimulusCombo() const
{
    return mStimulus;
}

inline QLabel* SMInternalEditor::signature() const
{
    return mSignature;
}

inline SMOperationsEditor* SMInternalEditor::operations() const
{
    return mOperations;
}

inline SMGuardBar* SMInternalEditor::guard() const
{
    return mGuard;
}

inline QTabWidget* SMInternalEditor::tabs() const
{
    return mTabs;
}

#endif  // LUSAN_VIEW_SM_SMINTERNALEDITOR_HPP

#ifndef LUSAN_VIEW_SM_SMPROPERTIESPANEL_HPP
#define LUSAN_VIEW_SM_SMPROPERTIESPANEL_HPP
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
 *  \file        lusan/view/sm/SMPropertiesPanel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Design page properties panel.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMReferences.hpp"
#include "lusan/view/common/IEditCommit.hpp"

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QAction;
class QComboBox;
class QEvent;
class QFormLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QScrollArea;
class QSpinBox;
class QStackedWidget;
class QTabWidget;
class SMGuardBar;
class SMGuardField;
class SMGuardStatusLine;
class SMInternalEditor;
class SMOperationsEditor;
class QToolButton;
class SMSectionChrome;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMPropertiesPanel
 * \brief   The Design page's right panel: a stacked editor that switches on the selected
 *          element. For a state it edits name (the same atomic rename as canvas F2),
 *          description, and lists its transitions with drag reorder (priority); for a
 *          transition it edits the stimulus (kind + name picker with inline registry
 *          creation), the target, and the description. A registry entry shows a read-only
 *          summary. It reads the shared selection model, so it always agrees with the
 *          canvas and outline; every edit commits as an undoable command.
 **/
class SMPropertiesPanel : public QWidget
                        , public IEditCommit
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    ePage
     * \brief   The stacked page shown for the current selection.
     **/
    enum ePage
    {
          PageEmpty     = 0   //!< No single-element selection.
        , PageState     = 1   //!< A state is selected.
        , PageTransition= 2   //!< A transition is selected.
        , PageRegistry  = 3   //!< A registry entry is selected.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMPropertiesPanel(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMPropertiesPanel();

//////////////////////////////////////////////////////////////////////////
// Attributes (tests and the owning page)
//////////////////////////////////////////////////////////////////////////
public:
    inline ePage currentPage() const;
    inline uint32_t currentElementId() const;

    /**
     * \brief   Hands over the description text the panel is still holding, for the state or the
     *          transition it is showing. The box applies its text when it loses the focus, which a
     *          save from the keyboard never causes.
     **/
    void commitPendingEdits(void) override;

    /**
     * \brief   Selects the transition and focuses the Conditions tab's guard field
     *          (edge-label double-click; validation-entry navigation).
     **/
    void focusConditions(uint32_t transitionId);

    /**
     * \brief   Selects the transition and opens its stimulus picker on the General tab. A
     *          transition has no name of its own, the stimulus is what identifies it.
     **/
    void focusStimulus(uint32_t transitionId);

    /**
     * \brief   Selects the transition's OWNING STATE and opens its Internal tab on that transition
     *          (a click on the `on <stimulus>` row inside a state box). It deliberately does not
     *          select the transition itself: an internal transition is something the state does,
     *          the author got here from the state's own box, and the Internal tab is where all four
     *          of a state's in-place activities now live.
     * \param   transitionId    The internal transition to edit. No-op for anything else.
     **/
    void focusInternal(uint32_t transitionId);

    /**
     * \brief   Binds the four submachine buttons on the State/General header to the Design page's
     *          own actions. One QAction per operation: two action objects would mean two
     *          enable-state computations, and they would drift.
     **/
    void bindSubmachineActions(QAction* enterOrAdd, QAction* goToParent, QAction* addSubmachine, QAction* removeSubmachine);

    inline QLineEdit* stateNameEdit() const;
    inline QComboBox* stateHistoryCombo() const;
    inline QComboBox* stateSubmachineCombo() const;
    inline QComboBox* stateOnFinalCombo() const;
    inline QListWidget* transitionList() const;
    inline QComboBox* stimulusNameCombo() const;
    inline QComboBox* targetCombo() const;
    inline QComboBox* transitionKindCombo() const;
    inline QComboBox* sourceCombo() const;
    inline SMInternalEditor* internalEditor() const;

signals:
    //!< A Ctrl+Shift click on a referenced symbol in the Conditions guard field asks the host to
    //!< open that declaration's registry page. Relayed up to the design, which switches pages.
    void signalNavigateToDefinition(SMReferences::eTarget kind, uint32_t declId);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The panel's own, fixed minimum size (\ref NESMDesign::PanelMinWidth by
     *          \ref NESMDesign::PanelMinHeight), independent of what the current selection puts
     *          on screen -- the hosting dock reads this to decide how far the user may shrink it.
     *          The content scrolls inside the panel below that size.
     **/
    virtual QSize minimumSizeHint() const override;

protected:
    /**
     * \brief   Commits a multi-line description editor (state / transition) when it loses
     *          focus - the same editing-finished commit contract as the single-line fields,
     *          which QPlainTextEdit does not signal on its own.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onModelSelectionChanged();
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onNameChanged(uint32_t id, const QString& oldName, const QString& newName);
    void onListReordered(uint32_t ownerId, eDocElementKind kind);
    void onDocumentReloaded();
    void onGuardBadgeChanged(bool isDraft, bool hasWarnings);
    void onStateNamePreview(uint32_t stateId, const QString& text);

    void onStateNameCommit();
    void onStateHistoryCommit();
    void onStateSubmachineCommit();
    void onStateOnFinalCommit();
    void onStateDescriptionCommit();
    void onTransitionDescriptionCommit();
    void onStimulusCommit();
    void onTransKindCommit();
    void onTargetCommit();
    void onSourceCommit();
    void onTransitionActivated();

    //!< The Internal tab's editor gained or lost a transition; the tab label carries the count.
    void onInternalCountChanged(int count);

private:
    void buildStatePage();

    /**
     * \brief   Enter, Exit and Internal are the three things a state does without leaving
     *          itself, and until now only two of them had a tab -- Internal was reachable only
     *          by double-clicking a row in a collapsible list on the General tab. The canvas
     *          context menu opens the SAME editor in an SMInternalDialog.
     **/
    void buildInternalTab();
    void buildTransitionPage();
    void buildRegistryPage();

    //!< Refreshes each state-action tab's tooltip with a summary of its bound list (`not set` when empty).
    void refreshActionSummaries();

    void refresh();
    void showEmpty();
    void showState(uint32_t stateId);
    void showTransition(uint32_t transitionId);
    void showRegistry(uint32_t elementId);
    void populateTransitionList(uint32_t stateId);

    /**
     * \brief   Reorders the current state's transition at \p from to \p to as one undo step
     *          (priority), through a chain of adjacent swaps.
     **/
    void reorderTransition(int from, int to);

    /**
     * \brief   True while a descendant editor has focus (a repopulate would clobber typing).
     **/
    bool isEditing() const;

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \enum    eOpList
     * \brief   Which of a state's operation lists a state-action tab is bound to.
     **/
    enum class eOpList
    {
          Entry     //!< The state's EntryList (`Enter` tab).
        , Exit      //!< The state's ExitList (`Exit` tab).
    };

    /**
     * \struct  ActionSlot
     * \brief   One state-action tab: its operation-list role, its editor and its index in the state
     *          tab widget. showState() binds each editor and refreshActionSummaries() re-tips each
     *          tab from this table, so a further list is a one-line addition.
     **/
    struct ActionSlot
    {
        eOpList             role;       //!< Which state list this tab edits.
        SMOperationsEditor* editor;     //!< The bound operation-list editor.
        int                 tabIndex;   //!< The tab index in mStateTabs (for the tooltip summary).
    };

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    QStackedWidget*     mStack;         //!< The page container.
    QScrollArea*        mScroll;        //!< Scrolls the page container when the panel is short.
    ePage               mPage;          //!< The shown page.
    uint32_t            mCurrentId;     //!< The shown element ID (0 = none).
    bool                mUpdating;      //!< Guards field population against commit signals.

    // State page.
    QTabWidget*         mStateTabs;     //!< The General / Enter / Exit / Internal tab host.
    SMSectionChrome*    mStateGeneral;  //!< The General tab chrome (Details / Transitions sections).
    QToolButton*        mBtnEnterSubmachine;  //!< Enter / open / add a substate, per selection.
    QToolButton*        mBtnGoToParent;
    QToolButton*        mBtnAddSubmachine;
    QToolButton*        mBtnRemoveSubmachine;
    QLineEdit*          mStateName;     //!< The state name (atomic rename on commit).
    QLabel*             mStateKind;     //!< The state kind (read-only).
    QComboBox*          mStateHistory;  //!< The history mode; only a composite may carry one.
    QComboBox*          mStateSubmachine;   //!< The hosted import alias; empty means no import.
    QComboBox*          mStateOnFinal;      //!< The event sent when the submachine finishes.
    QPlainTextEdit*     mStateDesc;     //!< The state description (multi-line).
    QFormLayout*        mStateForm;     //!< The Details form, so the rows a Start has no use for
                                        //!< (submachine, completion, history, description) can be
                                        //!< hidden rather than shown disabled.
    SMOperationsEditor* mEnterOps;      //!< The On-Enter operations editor (Actions tab).
    SMOperationsEditor* mExitOps;       //!< The On-Exit operations editor (Actions tab).
    QListWidget*        mTransitions;   //!< The state's transitions, drag-reorderable.
    QList<ActionSlot>   mActionSlots;   //!< The State-Actions sections, in display order.

    // State page, `Internal` tab -- the fourth thing a state does without leaving itself. The
    // editor is SHARED with the canvas context menu, which opens it in SMInternalDialog.
    SMInternalEditor*   mInternal;          //!< The state's internal transitions, edited in place.
    int                 mInternalTab;       //!< The Internal tab's index in mStateTabs.

    // Transition page.
    SMSectionChrome*    mTransGeneral;  //!< The General tab chrome (Trigger / Description sections).
    QLabel*             mStimulusSig;   //!< Read-only stimulus signature (`walk(count)`).
    QComboBox*          mStimulusName;  //!< The stimulus picker (fixed list of triggers/events/
                                        //!< timers; editing is search-only, no free rename).
    QFormLayout*        mTransForm;     //!< The transition Trigger form, so rows can be hidden by kind.
    QComboBox*          mTransKind;     //!< What the transition IS: External / Internal / Initial.
                                        //!< Locked to Initial on a Start, which owns nothing else.
    QComboBox*          mTarget;        //!< The target sibling state (or not connected). Start states
                                        //!< are omitted: a Start has no incoming transition.
    QComboBox*          mSource;        //!< The source sibling state (the transition's owner). Final
                                        //!< states are omitted: a Final has no outgoing transition.
    QPlainTextEdit*     mTransDesc;     //!< The transition description (multi-line).
    SMGuardBar*         mConditions;    //!< The Conditions tab guard bar.
    SMOperationsEditor* mTransOps;      //!< The transition operations editor (Actions tab).
    QTabWidget*         mTransTabs;     //!< The General / Conditions / Actions tab host.

    // Registry page.
    QLabel*             mRegistryInfo;  //!< The selected registry entry summary (read-only).
};

//////////////////////////////////////////////////////////////////////////
// SMPropertiesPanel inline methods
//////////////////////////////////////////////////////////////////////////

inline SMPropertiesPanel::ePage SMPropertiesPanel::currentPage() const
{
    return mPage;
}

inline uint32_t SMPropertiesPanel::currentElementId() const
{
    return mCurrentId;
}

inline QLineEdit* SMPropertiesPanel::stateNameEdit() const
{
    return mStateName;
}

inline QComboBox* SMPropertiesPanel::stateHistoryCombo() const
{
    return mStateHistory;
}

inline QComboBox* SMPropertiesPanel::stateSubmachineCombo() const
{
    return mStateSubmachine;
}

inline QComboBox* SMPropertiesPanel::stateOnFinalCombo() const
{
    return mStateOnFinal;
}

inline QListWidget* SMPropertiesPanel::transitionList() const
{
    return mTransitions;
}

inline QComboBox* SMPropertiesPanel::stimulusNameCombo() const
{
    return mStimulusName;
}

inline QComboBox* SMPropertiesPanel::targetCombo() const
{
    return mTarget;
}

inline QComboBox* SMPropertiesPanel::transitionKindCombo() const
{
    return mTransKind;
}

inline QComboBox* SMPropertiesPanel::sourceCombo() const
{
    return mSource;
}

inline SMInternalEditor* SMPropertiesPanel::internalEditor() const
{
    return mInternal;
}

#endif  // LUSAN_VIEW_SM_SMPROPERTIESPANEL_HPP

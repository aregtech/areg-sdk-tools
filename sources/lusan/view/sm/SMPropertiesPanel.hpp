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

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QComboBox;
class QEvent;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QStackedWidget;
class QTabWidget;
class SMGuardBar;
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
     * \brief   Selects the transition and focuses the Conditions tab's guard field
     *          (edge-label double-click, B13; validation-entry navigation, S15).
     **/
    void focusConditions(uint32_t transitionId);
    inline QLineEdit* stateNameEdit() const;
    inline QListWidget* transitionList() const;
    inline QComboBox* stimulusNameCombo() const;
    inline QComboBox* targetCombo() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
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

    void onStateNameCommit();
    void onStateDescriptionCommit();
    void onTransitionDescriptionCommit();
    void onStimulusCommit();
    void onTargetCommit();
    void onTransitionActivated();

private:
    void buildStatePage();
    void buildTransitionPage();
    void buildRegistryPage();

    void refresh();
    void showEmpty();
    void showState(uint32_t stateId);
    void showTransition(uint32_t transitionId);
    void showRegistry(uint32_t elementId);
    void populateTransitionList(uint32_t stateId);

    /**
     * \brief   Applies the stimulus picked from the fixed list (a trigger, event, or timer) to
     *          the current transition, as one undo step. The picker carries the real registry
     *          name and its kind per row; a value that does not match a listed entry is rejected
     *          (reverted) - the panel never creates or renames a registry entry.
     **/
    void applyStimulus();

    /**
     * \brief   Fills the stimulus picker with every trigger (by name), event ("on_event_<name>"),
     *          and timer ("on_timer_<name>"), each row carrying its kind and real name so the
     *          same name across an event and a timer stays unambiguous. Returns the display label
     *          for the given (kind, name) so the current transition's stimulus can be shown.
     **/
    QString populateStimulusPicker(int currentKind, const QString& currentName);

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
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    QStackedWidget*     mStack;         //!< The page container.
    ePage               mPage;          //!< The shown page.
    uint32_t            mCurrentId;     //!< The shown element ID (0 = none).
    bool                mUpdating;      //!< Guards field population against commit signals.

    // State page.
    QLineEdit*          mStateName;     //!< The state name (atomic rename on commit).
    QLabel*             mStateKind;     //!< The state kind (read-only).
    QPlainTextEdit*     mStateDesc;     //!< The state description (multi-line, at the bottom).
    QLabel*             mStateEntry;    //!< The entry-operation summary (read-only).
    QLabel*             mStateExit;     //!< The exit-operation summary (read-only).
    QListWidget*        mTransitions;   //!< The state's transitions, drag-reorderable.

    // Transition page.
    QComboBox*          mStimulusName;  //!< The stimulus picker (fixed list of triggers/events/
                                        //!< timers; editing is search-only, no free rename).
    QComboBox*          mTarget;        //!< The target sibling state (or internal).
    QPlainTextEdit*     mTransDesc;     //!< The transition description (multi-line).
    SMGuardBar*         mConditions;    //!< The Conditions tab guard bar.
    QTabWidget*         mTransTabs;     //!< The General / Conditions tab host (for the badge).

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

#endif  // LUSAN_VIEW_SM_SMPROPERTIESPANEL_HPP

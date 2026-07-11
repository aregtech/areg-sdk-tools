#ifndef LUSAN_VIEW_SM_SMEVENT_HPP
#define LUSAN_VIEW_SM_SMEVENT_HPP
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
 *  \file        lusan/view/sm/SMEvent.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Events page.
 *
 ************************************************************************/

#include <QScrollArea>
#include <cstdint>

class DocumentElem;
class QEvent;
class QTreeWidgetItem;
class SMEventDetails;
class SMEventEntry;
class SMEventList;
class SMEventModel;
class SMEventParamDetails;
class SMTimerDetails;
class SMTimerEntry;
class SMTimerModel;

/**
 * \brief   The FSM Events page: machine-global events with optional typed payload parameters,
 *          and timers, edited in the shared list-left / details-right page layout. The left
 *          panel is one tree with two permanent group nodes ("Events", "Timers") under a
 *          single toolbar whose Add split button creates the kind matching the selection; the
 *          right panel swaps between the event, parameter and timer detail forms. Every edit
 *          goes through the page models' undo commands; the page refreshes by rebuilding the
 *          group children from the live models on every relevant DocModelNotifier signal and
 *          restoring the selection by element ID.
 **/
class SMEvent : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    //!< What the currently selected tree row represents; drives the detail form swap and
    //!< the toolbar enable state.
    enum class eRowKind : int
    {
          None          = 0 //!< No selection.
        , GroupEvents   = 1 //!< The permanent "Events" group node.
        , GroupTimers   = 2 //!< The permanent "Timers" group node.
        , Event         = 3 //!< An event entry.
        , Param         = 4 //!< A payload parameter of an event.
        , Timer         = 5 //!< A timer entry.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMEvent(SMEventModel& eventModel, SMTimerModel& timerModel, QWidget* parent = nullptr);
    virtual ~SMEvent() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel (its Add-dropdown actions let a caller start a new
     *          event or timer, e.g. from the Design page's Declare dropdown).
     **/
    SMEventList* getList() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    //!< Plain click on the Add split button: creates the kind matching the selection.
    void onAddClicked();
    void onInsertClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onEventNameTextChanged(const QString& text);
    void onEventNameCommitted();
    void onEventDeprecatedToggled(bool checked);
    void onEventDeprecateHintCommitted();

    void onParamNameTextChanged(const QString& text);
    void onParamNameCommitted();
    void onParamTypeChanged(int index);
    void onParamHasDefaultToggled(bool checked);
    void onParamValueCommitted();
    void onParamDeprecatedToggled(bool checked);
    void onParamDeprecateHintCommitted();

    void onTimerNameTextChanged(const QString& text);
    void onTimerNameCommitted();
    void onTimeoutCommitted();
    void onRepeatCommitted();
    void onContinuousToggled(bool checked);
    void onTimerDeprecatedToggled(bool checked);
    void onTimerDeprecateHintCommitted();

    void onNotifierChanged();
    //!< Repopulates the parameter type combo on any DataType-kind notifier signal (a
    //!< declared type may have been added, renamed, converted or removed).
    void onDataTypesChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    //!< Creates a new event at the end of the list, selects it and focuses its name.
    void addNewEvent();
    //!< Creates a new timer at the end of the list, selects it and focuses its name.
    void addNewTimer();
    //!< Creates a new payload parameter for the selected event, selects it and focuses its name.
    void addNewParam();

    //!< Rebuilds both group subtrees from the live models and restores the selection by ID.
    void refreshAll();
    //!< Selects the event / parameter row by ID; returns false if not found.
    bool selectEvent(uint32_t eventId, uint32_t paramId = 0);
    //!< Selects the timer row by ID; returns false if not found.
    bool selectTimer(uint32_t timerId);

    //!< Clears and disables the visible detail form; used for empty and group selections.
    void showCleanForm();
    //!< Shows exactly one of the three detail forms, matching the selected row kind.
    void showDetails(eRowKind kind);
    //!< Applies the toolbar enable matrix and the Add button's default target for the kind.
    void updateToolbar(eRowKind kind);

    void selectedGroup(eRowKind kind);
    void selectedEvent(SMEventEntry* event);
    void selectedParam(SMEventEntry* owner, uint32_t paramId);
    void selectedTimer(const SMTimerEntry* entry);
    void updateMoveButtons(int row, int rowCount);
    void populateTypeCombo();
    //!< Puts keyboard focus into the visible detail form's name field.
    void focusNameField();

    //!< Empty string if the name is free in the shared stimulus name space, otherwise a
    //!< short, user-facing collision reason. \p selfId excludes the edited element itself.
    QString stimulusCollisionReason(const QString& name, uint32_t selfId) const;
    //!< Empty string if the name is free within \p owner's parameter list, otherwise a
    //!< short, user-facing collision reason. \p selfId excludes the parameter itself.
    QString paramNameCollisionReason(const SMEventEntry* owner, const QString& name, uint32_t selfId) const;

    QTreeWidgetItem* createEventNode(SMEventEntry* event) const;
    QTreeWidgetItem* createTimerNode(const SMTimerEntry& entry) const;
    void setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const;

    //!< The kind of the currently selected tree row.
    eRowKind currentKind() const;
    //!< The event owning the current selection (event or parameter row), or nullptr.
    SMEventEntry* currentEvent() const;
    //!< The selected parameter ID, or 0 if the selection is not a parameter row.
    uint32_t currentParamId() const;
    //!< The selected timer ID, or 0 if the selection is not a timer row.
    uint32_t currentTimerId() const;

    QString genEventName();
    QString genTimerName();
    QString genParamName(const SMEventEntry* event) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMEventModel&           mEventModel;        //!< Events page model (undo-command mutators).
    SMTimerModel&           mTimerModel;        //!< Timers page model (undo-command mutators).
    SMEventList*            mList;              //!< Left panel: grouped tree + toolbar.
    SMEventDetails*         mDetails;           //!< Right panel: selected event form.
    SMEventParamDetails*    mParamDetails;      //!< Right panel: selected parameter form.
    SMTimerDetails*         mTimerDetails;      //!< Right panel: selected timer form.
    uint32_t                mEventNameCounter;  //!< Suffix counter for generated event names.
    uint32_t                mTimerNameCounter;  //!< Suffix counter for generated timer names.
};

#endif  // LUSAN_VIEW_SM_SMEVENT_HPP

#ifndef LUSAN_VIEW_SM_SMOPERATIONSEDITOR_HPP
#define LUSAN_VIEW_SM_SMOPERATIONSEDITOR_HPP
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
 *  \file        lusan/view/sm/SMOperationsEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation-list editor (entry/exit/transition actions).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/data/sm/SMOperation.hpp"

#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineModel;
class MethodBase;
class ElementBase;
enum class eDocElementKind;

class QComboBox;
class QFormLayout;
class QLabel;
class QVBoxLayout;

/**
 * \class   SMOperationsEditor
 * \brief   The simple, opinionated editor for one operation list -- a state's `EntryList` or
 *          `ExitList`, or a transition's `OperationList`. It presents a fixed three-part form
 *          (all parts optional): one **Action** (a declared action call), one **Event** (an
 *          event send), and any number of **Timers** (start/stop). There is no ordering UI --
 *          the action runs first, the event second, timers after -- so the form has no list or
 *          reorder controls. Each action/event parameter is one row with a single editable
 *          combo: pick a stimulus parameter or a machine attribute, or type a free value.
 *          Every edit is one undoable command; the stimulus-parameter source is offered only in
 *          a transition scope (spec 6.8). The same widget is hosted in the Properties panel's
 *          Actions tab and in the context-menu dialog.
 **/
class SMOperationsEditor : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMOperationsEditor(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMOperationsEditor();

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Binds the editor to an operation list.
     * \param   ownerId         The owning state or transition ID.
     * \param   ownerKind       eDocElementKind::State or ::Transition.
     * \param   scopeTransition The transition ID for the stimulus-parameter scope, or 0.
     * \param   owner           The owning element (list parent), for ID delegation.
     * \param   list            The live operation list, or nullptr to clear.
     **/
    void bind( uint32_t ownerId
             , eDocElementKind ownerKind
             , uint32_t scopeTransition
             , ElementBase* owner
             , SMOperationList* list);

    //!< Clears the binding and empties the editor.
    void clearBinding();

    //!< Test/host accessors.
    inline QComboBox* actionCombo() const;
    inline QComboBox* eventCombo() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Opens a parameter mapping combo's dropdown on a click in its edit field, so the
     *          allowed values (stimulus parameters / attributes) are one click away rather than
     *          only reachable through the tiny drop arrow.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private slots:
    void onNotifierChanged(uint32_t id, eDocElementKind kind);

private:
    void buildUi();
    void rebuild();
    void rebuildAction();
    void rebuildEvent();
    void rebuildTimers();
    void buildParamRows(QFormLayout* form, uint32_t opId, bool isEvent);

    // Finders.
    SMActionCall* findAction() const;
    SMEventSend* findEvent() const;
    const MethodBase* calleeFor(const SMOperationBase* op, bool isEvent) const;

    // Mutations.
    void setActionName(const QString& name);
    void setEventName(const QString& name);
    void addTimer(bool start);
    void removeTimer(uint32_t opId);
    void setTimerName(uint32_t opId, const QString& name);
    void commitParam(uint32_t opId, bool isEvent, const QString& paramName, const QString& text);

    // Registry / mapping helpers.
    QStringList actionNames() const;
    QStringList eventNames() const;
    QStringList timerNames() const;
    QStringList stimulusParamNames() const;
    QStringList attributeNames() const;
    SMArgumentEntry::eValueSource resolveSource(const QString& name, QString& mappedName) const;

    bool isEditing() const;
    void scheduleRebuild();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;
    uint32_t            mOwnerId;
    eDocElementKind     mOwnerKind;
    uint32_t            mScopeTransition;
    bool                mAllowParam;
    ElementBase*        mOwner;
    SMOperationList*    mList;
    bool                mApplying;
    bool                mRebuildQueued;

    QComboBox*          mActionCombo;
    QFormLayout*        mActionParams;
    QComboBox*          mEventCombo;
    QFormLayout*        mEventParams;
    QVBoxLayout*        mTimersList;
    QLabel*             mTimersEmpty;
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline QComboBox* SMOperationsEditor::actionCombo() const
{
    return mActionCombo;
}

inline QComboBox* SMOperationsEditor::eventCombo() const
{
    return mEventCombo;
}

#endif  // LUSAN_VIEW_SM_SMOPERATIONSEDITOR_HPP

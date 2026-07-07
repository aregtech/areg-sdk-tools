#ifndef LUSAN_VIEW_SM_SMMETHOD_HPP
#define LUSAN_VIEW_SM_SMMETHOD_HPP
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
 *  \file        lusan/view/sm/SMMethod.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Methods page.
 *
 ************************************************************************/

#include <QScrollArea>
#include <cstdint>

#include "lusan/data/sm/SMMethodData.hpp"

class DocumentElem;
class QEvent;
class QTreeWidgetItem;
class SMEventParamDetails;
class SMMethodDetails;
class SMMethodList;
class SMMethodModel;

/**
 * \brief   The FSM Methods page: triggers, actions and conditions with typed parameters and
 *          optional defaults, edited in the shared list-left / details-right page layout. The
 *          left panel is one tree whose top-level rows are methods and whose child rows are
 *          the method parameters, under a single toolbar; the right panel swaps between the
 *          method and parameter detail forms. Conditions additionally carry a return type, an
 *          implementation mode and — when embedded — a verbatim C++ body. Every edit goes
 *          through the page model's undo commands; the page refreshes by rebuilding the tree
 *          from the live model on every relevant DocModelNotifier signal and restoring the
 *          selection by element ID.
 **/
class SMMethod : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
private:
    //!< What the currently selected tree row represents; drives the detail form swap and the
    //!< toolbar enable state.
    enum class eRowKind : int
    {
          None      = 0 //!< No selection.
        , Method    = 1 //!< A method entry.
        , Param     = 2 //!< A parameter of a method.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMMethod(SMMethodModel& model, QWidget* parent = nullptr);
    virtual ~SMMethod() = default;

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

    void onAddClicked();
    void onInsertClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onMethodNameTextChanged(const QString& text);
    void onMethodNameCommitted();
    void onMethodTypeToggled(bool checked);
    void onReturnCommitted();
    void onImplementToggled(bool checked);

    void onParamNameTextChanged(const QString& text);
    void onParamNameCommitted();
    void onParamTypeChanged(int index);
    void onParamHasDefaultToggled(bool checked);
    void onParamValueCommitted();

    void onNotifierChanged();
    //!< Repopulates the type combos on any DataType-kind notifier signal.
    void onDataTypesChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    void addNewMethod(SMMethodEntry::eMethodType type);
    void addNewParam();

    //!< Rebuilds the tree from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the method / parameter row by ID; returns false if not found.
    bool selectMethod(uint32_t methodId, uint32_t paramId = 0);

    void showCleanForm();
    //!< Shows exactly one of the two detail forms, matching the selected row kind.
    void showDetails(eRowKind kind);
    void updateToolbar(eRowKind kind);

    void selectedMethod(SMMethodEntry* method);
    void selectedParam(SMMethodEntry* owner, uint32_t paramId);
    void updateMoveButtons(int row, int rowCount);
    //!< Fills the method detail form from \p method and applies the condition-only rows.
    void showMethodForm(SMMethodEntry* method);
    //!< Updates the code editor signature and note from the current method state.
    void updateBodyEditor(SMMethodEntry* method);
    void populateParamTypeCombo();
    void populateReturnCombo();

    //!< Empty string if the name is free, otherwise a short, user-facing collision reason —
    //!< another method with the same name, or (for triggers) an event/timer stimulus clash.
    QString nameCollisionReason(const SMMethodEntry* method, const QString& name, uint32_t selfId) const;
    QString paramNameCollisionReason(const SMMethodEntry* owner, const QString& name, uint32_t selfId) const;

    QTreeWidgetItem* createMethodNode(SMMethodEntry* method) const;
    void setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const;

    eRowKind currentKind() const;
    //!< The method owning the current selection (method or parameter row), or nullptr.
    SMMethodEntry* currentMethod() const;
    //!< The selected parameter ID, or 0 if the selection is not a parameter row.
    uint32_t currentParamId() const;

    QString genMethodName();
    QString genParamName(const SMMethodEntry* method) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMMethodModel&          mModel;             //!< Methods page model (undo-command mutators).
    SMMethodList*           mList;              //!< Left panel: methods/parameters tree + toolbar.
    SMMethodDetails*        mDetails;           //!< Right panel: selected method form.
    SMEventParamDetails*    mParamDetails;      //!< Right panel: selected parameter form.
    uint32_t                mMethodNameCounter; //!< Suffix counter for generated method names.
};

#endif  // LUSAN_VIEW_SM_SMMETHOD_HPP

#ifndef LUSAN_VIEW_SM_SMCONDITIONEDITOR_HPP
#define LUSAN_VIEW_SM_SMCONDITIONEDITOR_HPP
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
 *  \file        lusan/view/sm/SMConditionEditor.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition condition builder (Conditions tab).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QHash>
#include <cstdint>
#include <memory>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QVBoxLayout;
class SMConditionEntry;
class SMConditionGroup;
class StateMachineData;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMConditionEditor
 * \brief   The Conditions tab of the transition properties: a nested And/Or group builder
 *          with a unified operand picker (arg::/attr::/const::/cond::/val::/expr::/lambda::),
 *          verbatim code cells, condition-method argument mapping, a live plain-text preview,
 *          and non-blocking per-row type feedback. Every edit routes through an SM-21-03
 *          condition command on the document undo stack; the widget rebuilds itself from the
 *          canonical model on change, so click and typed input converge on the same tree.
 **/
class SMConditionEditor : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMConditionEditor(StateMachineModel& model, QWidget* parent = nullptr);
    virtual ~SMConditionEditor();

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Points the editor at a transition (0 clears it) and rebuilds the tree.
     **/
    void setTransition(uint32_t transitionId);

    inline uint32_t transitionId() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Commits a verbatim code cell (expression / lambda body) when it loses focus --
     *          the code editor has no editing-finished signal of its own.
     **/
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onElementChanged(uint32_t id, eDocElementKind kind);
    void onElementRemoved(uint32_t id, eDocElementKind kind);
    void onDocumentReloaded();

private:
    struct LeafRow;     //!< Per-row widget bundle (defined in the .cpp).

    void scheduleRebuild();
    void rebuild();

    void buildGroup(QVBoxLayout* parentLayout, SMConditionGroup& group, SMConditionGroup* parent, int depth);
    void buildLeaf(QVBoxLayout* parentLayout, SMConditionEntry& leaf, SMConditionGroup& group);

    //!< Adds up/down buttons that reorder a child within its parent group (evaluation order).
    void addReorderButtons(class QHBoxLayout* line, uint32_t parentGroupId, int index, int count);

    void commitLeaf(const std::shared_ptr<LeafRow>& row);
    void updateFeedback(const std::shared_ptr<LeafRow>& row);

    //!< The transition's condition tree root, or nullptr when nothing valid is selected.
    SMConditionGroup* rootGroup() const;

    //!< Fills an operand picker (a combo) with the machine's operands; skips arg:: for timers.
    void fillOperandPicker(class QComboBox* combo, bool includeVerbatim) const;

    //!< The declared type of an operand kind+ref (empty when it cannot be judged), for feedback.
    QString operandType(int kind, const QString& ref) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&  mModel;         //!< The document facade.
    uint32_t            mTransitionId;  //!< The edited transition (0 = none).
    QVBoxLayout*        mTreeLayout;    //!< Holds the built group/leaf widgets.
    QLabel*             mPreview;       //!< The live plain-text guard preview.
    bool                mRebuildPending;//!< Coalesces deferred rebuilds.
    QHash<QObject*, std::shared_ptr<LeafRow>> mCodeRows; //!< Verbatim code body -> its row.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t SMConditionEditor::transitionId() const
{
    return mTransitionId;
}

#endif  // LUSAN_VIEW_SM_SMCONDITIONEDITOR_HPP

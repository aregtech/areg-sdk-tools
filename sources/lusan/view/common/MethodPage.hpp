#ifndef LUSAN_VIEW_COMMON_METHODPAGE_HPP
#define LUSAN_VIEW_COMMON_METHODPAGE_HPP
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
 *  \file        lusan/view/common/MethodPage.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Methods page shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/MethodEntry.hpp"
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QScrollArea>
#include <QString>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MethodDetailsView;
class MethodListView;
class MethodModel;
class MethodParamDetailsView;
class QAbstractItemModel;
class QEvent;
class QStandardItemModel;
class QTreeWidgetItem;

/**
 * \brief   What a document calls its Methods page. Everything else the page needs -- which kinds
 *          it offers, what each of them carries, how a parameter default is spelled -- it reads
 *          from the model's configuration, so the two can never disagree.
 **/
struct MethodPageConfig
{
    QString headline;           //!< The title above the two panels.
    QString listTitle;          //!< The caption of the list panel.
    bool    hasGuardInfo;       //!< The page can say where a method is used.
};

/**
 * \class   MethodPage
 * \brief   The Methods page: named methods with typed parameters, edited the same way in every
 *          document that declares them. The left panel is one tree whose top-level rows are the
 *          methods and whose child rows are their parameters, under a single toolbar; the right
 *          panel swaps between the method form and the parameter form.
 *
 *          What a method carries beyond a name, parameters and a description follows its kind:
 *          the method that answers it, or the type it returns plus how it is implemented. The
 *          page reads that from the model's configuration, so the service interface and the
 *          state machine use this one controller.
 *
 *          Every edit is committed through the model's undo commands; the page mutates no data
 *          itself. It rebuilds the tree from the live model on the notifier signals the commands
 *          emit, rather than patching rows by hand, so a row can never drift from what was
 *          stored -- including after an undo, a redo or a document reload.
 *
 *          A document with extra behaviour of its own subclasses this: see \ref confirmRemove,
 *          \ref nameCollisionReason, \ref decorateMethodNode and \ref updateExtraFields.
 **/
class MethodPage : public    QScrollArea
                 , public    IEDataTypeConsumer
                 , public    IEditCommit
                 , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
protected:
    //!< What the selected tree row stands for; drives the form swap and the toolbar state.
    enum class eRowKind : int
    {
          None      = 0 //!< Nothing is selected.
        , Method    = 1 //!< A method.
        , Param     = 2 //!< A parameter of a method.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the page.
     * \param   model   The methods model of the document being edited.
     * \param   config  What this document calls its Methods page.
     * \param   parent  The parent widget.
     **/
    explicit MethodPage(MethodModel& model, const MethodPageConfig& config, QWidget* parent = nullptr);

    virtual ~MethodPage(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel, so a caller elsewhere can start a new method of a given
     *          kind through its Add drop-down.
     **/
    MethodListView* getList(void) const;

    /**
     * \brief   Selects and reveals a method, or one of its parameters, by document ID -- a
     *          finding about a parameter names the parameter, not the method that declares it.
     *          Does nothing if nothing has that ID.
     * \param   id      The document ID of the element to reveal.
     * \param   field   The field to accent once the row is selected.
     **/
    void revealElement(uint32_t id, eIssueField field = eIssueField::None);

    /**
     * \brief   Rebuilds the type lists the page offers and re-resolves every declared parameter
     *          type against what the document holds now.
     **/
    virtual void dataTypesChanged(void) override;

    /**
     * \brief   Hands over the description and the body the page is still holding, for whichever
     *          form the selection is on. A multi-line box applies its text when it loses the
     *          focus, which a save from the keyboard never causes.
     **/
    virtual void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Asks whether the method may be deleted. The base page always agrees; a document
     *          that can tell where a method is used warns about the references first.
     * \param   id  The document ID of the method about to be deleted.
     * \return  True to go ahead with the deletion.
     **/
    virtual bool confirmRemove(uint32_t id);

    /**
     * \brief   Empty string if the name is free for a method of this kind, otherwise a short,
     *          user-facing reason. The base page reports another method of the same kind; a
     *          document whose method names live in a wider name space reports that too.
     **/
    virtual QString nameCollisionReason(const MethodEntry* method, const QString& name, uint32_t selfId) const;

    /**
     * \brief   Adds whatever the document shows on a method's row beyond the shared columns.
     *          The base page adds nothing.
     **/
    virtual void decorateMethodNode(QTreeWidgetItem* node, const MethodEntry& method) const;

    /**
     * \brief   Fills the form rows the base page does not know about, and hides them for a
     *          method that has none. The base page has none.
     **/
    virtual void updateExtraFields(MethodEntry* method);

    /**
     * \brief   Refreshes the embedded-body editor for the given method. The base page sets the
     *          signature the body must match.
     **/
    virtual void updateBodyEditor(MethodEntry* method);

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    virtual int getColumnCount(void) const override;

    virtual QString getCellText(const QModelIndex& cell) const override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    void onAddClicked(void);
    void onInsertClicked(void);
    void onRemoveClicked(void);
    void onMoveUpClicked(void);
    void onMoveDownClicked(void);

    void onMethodNameTextChanged(const QString& text);
    void onMethodNameCommitted(void);
    void onMethodKindToggled(bool checked);
    void onReplyChanged(int index);
    void onReturnCommitted(void);
    void onImplementToggled(bool checked);
    void onMethodDeprecatedToggled(bool checked);
    void onMethodDeprecateHintCommitted(void);

    void onParamNameTextChanged(const QString& text);
    void onParamNameCommitted(void);
    void onParamTypeChanged(int index);
    void onParamHasDefaultToggled(bool checked);
    void onParamValueCommitted(void);
    void onParamDeprecatedToggled(bool checked);
    void onParamDeprecateHintCommitted(void);

    //!< Rebuilds the whole tree on any Method-kind notifier signal.
    void onNotifierChanged(void);

    //!< Repopulates the type lists on any DataType-kind notifier signal.
    void onDataTypesChanged(void);

    //!< Applies what was typed into a tree cell. Queued, so the inline editor is fully closed
    //!< before the tree is rebuilt underneath it.
    void onEditorDataChanged(const QModelIndex& index, const QString& newValue);

    //!< Mirrors the name or value being typed in a tree cell into the details panel, leaving the
    //!< inline editor open; the edit itself commits once, through onEditorDataChanged.
    void onEditorTextChanged(const QModelIndex& index, const QString& newText);

    //!< Fills the details panel from the model once an inline editor has closed.
    void onEditorClosed(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< The model this page edits.
    inline MethodModel& getModel(void) const;

    //!< The two detail panels, for a subclass that fills or accents one of their fields.
    inline MethodDetailsView* getDetails(void) const;
    inline MethodParamDetailsView* getParamDetails(void) const;

    //!< What the selected row stands for.
    eRowKind currentKind(void) const;

    //!< The method the selection belongs to, method row or parameter row alike, or nullptr.
    MethodEntry* currentMethod(void) const;

    //!< The selected parameter's ID, or 0 when the selection is not a parameter row.
    uint32_t currentParamId(void) const;

    //!< Selects the method, or one of its parameters; false when the row is not there.
    bool selectMethod(uint32_t methodId, uint32_t paramId = 0);

    //!< Adds a method of the given kind and puts the caret in its name.
    void addNewMethod(int kind);

    //!< Puts the caret in the Name field of the selected row, with its text selected.
    void focusNameField(void);

    //!< Rebuilds the whole tree from the live model and restores the selection by ID. A
    //!< subclass calls this at the end of its own constructor: the first build ran while the
    //!< object was still a MethodPage, so none of its overrides were reached.
    void refreshAll(void);

private:
    void buildUi(void);
    void setupSignals(void);

    void addNewParam(void);

    QTreeWidgetItem* createMethodNode(MethodEntry* method) const;
    void setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const;

    void showCleanForm(void);
    //!< Shows exactly one of the two detail forms, matching the selected row kind.
    void showDetails(eRowKind kind);
    void updateToolbar(eRowKind kind);
    void updateMoveButtons(int row, int rowCount);

    void selectedMethod(MethodEntry* method);
    void selectedParam(MethodEntry* owner, uint32_t paramId);
    void showMethodForm(MethodEntry* method);

    void populateReplyCombo(void);
    void populateReturnCombo(void);
    void populateParamTypeCombo(void);

    QString paramNameCollisionReason(const MethodEntry* owner, const QString& name, uint32_t selfId) const;

    //!< Commits a deferred inline cell edit through the model's undo commands.
    void commitInlineEdit(int kind, uint32_t methodId, uint32_t paramId, int column, const QString& newValue);
    bool isCellEditable(const QModelIndex& index) const;
    QAbstractItemModel* editorModelFor(const QModelIndex& index) const;
    TableCell::eCellValidation validationFor(const QModelIndex& index) const;

    QString genMethodName(void);
    QString genParamName(const MethodEntry* method) const;

    //!< The index of the kind a method may be answered by, or -1 when the document has none.
    int replyKindIndex(void) const;

    //!< The index of the method kind carrying the given label, or -1 when no kind does.
    int kindIndexOf(const QString& label) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MethodModel&            mModel;             //!< The methods of the document being edited.
    const MethodPageConfig  mPageConfig;        //!< What this document calls its Methods page.
    MethodListView*         mList;              //!< The list panel.
    MethodDetailsView*      mDetails;           //!< The selected method's form.
    MethodParamDetailsView* mParamDetails;      //!< The selected parameter's form.
    TableCell*              mTableCell;         //!< The inline editors of the tree.
    QStandardItemModel*     mKindModel;         //!< The method kinds the type cell offers, marked
                                                //!< with the same icons the details radios carry.
    uint32_t                mMethodNameCounter; //!< The counter behind the generated names.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    MethodPage(const MethodPage& /*src*/) = delete;
    MethodPage& operator = (const MethodPage& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// MethodPage inline methods
//////////////////////////////////////////////////////////////////////////

inline MethodModel& MethodPage::getModel(void) const
{
    return mModel;
}

inline MethodDetailsView* MethodPage::getDetails(void) const
{
    return mDetails;
}

inline MethodParamDetailsView* MethodPage::getParamDetails(void) const
{
    return mParamDetails;
}

#endif  // LUSAN_VIEW_COMMON_METHODPAGE_HPP

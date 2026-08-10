#ifndef LUSAN_VIEW_COMMON_ATTRIBUTEPAGE_HPP
#define LUSAN_VIEW_COMMON_ATTRIBUTEPAGE_HPP
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
 *  \file        lusan/view/common/AttributePage.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Attributes page shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/view/common/AttributeListView.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QScrollArea>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class AttributeDetailsView;
class AttributeEntry;
class AttributeModel;
class DataTypeBase;
class DataTypeCustom;
class QEvent;
class QStringListModel;
class QTreeWidgetItem;

/**
 * \class   AttributePage
 * \brief   The Attributes page: named typed attributes, edited the same way in every document that
 *          has them. It drives the shared list and details widgets over an \ref AttributeModel and
 *          knows nothing about the document behind it, so the service interface, the state machine
 *          and a standalone data type document all use this one controller.
 *
 *          The third list column and the matching details row follow the model's configuration:
 *          a default value where the document's attributes carry one, an update notification kind
 *          where they carry that instead.
 *
 *          Every edit is committed through the model's undo commands; the page mutates no data
 *          itself. It rebuilds the list from the live model on the notifier signals the commands
 *          emit, rather than patching rows by hand, so a row can never drift from what was stored
 *          -- including after an undo, a redo or a document reload.
 *
 *          A document with extra behaviour of its own subclasses this: see \ref confirmRemove.
 **/
class AttributePage : public    QScrollArea
                    , public    IEDataTypeConsumer
                    , public    IEditCommit
                    , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the page.
     * \param   model       The attributes model of the document being edited.
     * \param   headline    The title shown above the two panels.
     * \param   parent      The parent widget.
     **/
    explicit AttributePage(AttributeModel& model, const QString& headline, QWidget* parent = nullptr);

    virtual ~AttributePage(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel, so a caller elsewhere can start a new attribute through its
     *          Add button.
     **/
    AttributeListView* getList(void) const;

    /**
     * \brief   Returns the attribute the list is currently on, or nullptr if nothing is selected.
     **/
    AttributeEntry* currentAttribute(void) const;

    /**
     * \brief   The document ID of the currently selected attribute, or 0 if none is selected.
     **/
    uint32_t currentAttributeId(void) const;

    /**
     * \brief   Selects and reveals the attribute with the given document ID. Does nothing if no
     *          attribute has that ID.
     * \param   id      The document ID of the attribute to reveal.
     * \param   field   The field to accent once the row is selected.
     **/
    void revealElement(uint32_t id, eIssueField field = eIssueField::None);

    /**
     * \brief   Repoints every attribute declared with the old data type to the new one, and
     *          refreshes what the page shows.
     **/
    void replaceDataType(DataTypeBase* oldType, DataTypeBase* newType);

    /**
     * \brief   Rebuilds the type list the page offers and re-resolves every attribute's declared
     *          type against what the document holds now. A document that changes its data types
     *          without going through the notifier calls this.
     **/
    virtual void dataTypesChanged(void) override;

    /**
     * \brief   Hands over the description text the page is still holding. The box applies its text
     *          when it loses the focus, which a save from the keyboard never causes.
     **/
    virtual void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    /**
     * \brief   Asks whether the attribute may be deleted. The base page always agrees; a document
     *          that can tell where an attribute is used warns about the references first.
     * \param   id  The document ID of the attribute about to be deleted.
     * \return  True to go ahead with the deletion.
     **/
    virtual bool confirmRemove(uint32_t id);

    /**
     * \brief   Empty string if the literal is valid for the named type, otherwise a short,
     *          user-facing reason. The base page checks the primitive literal syntax and the
     *          enumerators of a declared enumeration.
     **/
    virtual QString valueValidationReason(const QString& typeName, const QString& value) const;

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

    void onNameCommitted(void);
    void onTypeChanged(int index);
    void onValueCommitted(void);
    void onNotificationChanged(int index);

    //!< Mirrors the value into the list row and re-validates while the user types; the model
    //!< commit still happens once on editing-finished.
    void onValueTextChanged(const QString& text);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted(void);

    //!< Applies what was typed into a list cell. Queued, so the inline editor is fully closed
    //!< before the list is rebuilt underneath it.
    void onEditorDataChanged(const QModelIndex& index, const QString& newValue);

    //!< Rebuilds the whole list on any Attribute-kind notifier signal.
    void onNotifierChanged(void);

    //!< Repopulates the type list and re-validates on any DataType-kind notifier signal.
    void onDataTypesChanged(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:
    //!< Puts the caret in the selected attribute's Name field with its text selected.
    void focusNameField(void);

    //!< The model this page edits.
    inline AttributeModel& getModel(void) const;

    //!< The details panel, for a subclass that accents one of its fields.
    inline AttributeDetailsView* getDetails(void) const;

    //!< Finds the declared type among the document's custom types, or nullptr for a predefined or
    //!< unknown type name.
    DataTypeCustom* findCustomType(const QString& typeName) const;

private:
    void buildUi(const QString& headline);
    void setupSignals(void);

    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll(void);

    //!< Selects the attribute by ID; returns false if not found (and selects nothing).
    bool selectAttribute(uint32_t id);

    //!< Populates the details panel for the given attribute.
    void selectedAttribute(const AttributeEntry* entry);

    //!< Clears the details panel and disables the row-only tool buttons.
    void showClean(void);

    //!< Configures the value control (free text, enumerator picker, or disabled) and shows the
    //!< current literal's validation hint for the given attribute.
    void updateValueControl(const AttributeEntry* entry);

    //!< Re-evaluates and displays the value hint for the given type/literal pair.
    void updateValueValidation(const QString& typeName, const QString& value);

    //!< Fills a list row's columns, flagging an invalid stored value with a warning icon.
    void setNodeText(QTreeWidgetItem* node, const AttributeEntry& entry) const;

    void populateTypes(void);
    void updateMoveButtons(int row, int rowCount);

    QString genName(void);

    //!< True while the third column holds a default value rather than a notification kind.
    inline bool hasValueColumn(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    AttributeModel&         mModel;         //!< The attributes of the document being edited.
    const AttributeConfig   mConfig;        //!< What this document's attributes carry.
    AttributeListView*      mList;          //!< The list panel.
    AttributeDetailsView*   mDetails;       //!< The details panel.
    QStringListModel*       mTypeNames;     //!< The type names offered by the inline type editor.
    QStringListModel*       mNotifyNames;   //!< The notification kinds offered by both editors.
    TableCell*              mTableCell;     //!< The inline editors of the list.
    uint32_t                mNameCounter;   //!< The counter behind the generated names.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    AttributePage(const AttributePage& /*src*/) = delete;
    AttributePage& operator = (const AttributePage& /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// AttributePage inline methods
//////////////////////////////////////////////////////////////////////////

inline AttributeModel& AttributePage::getModel(void) const
{
    return mModel;
}

inline AttributeDetailsView* AttributePage::getDetails(void) const
{
    return mDetails;
}

inline bool AttributePage::hasValueColumn(void) const
{
    return mConfig.hasValue;
}

#endif  // LUSAN_VIEW_COMMON_ATTRIBUTEPAGE_HPP

#ifndef LUSAN_VIEW_COMMON_DATATYPEPAGE_HPP
#define LUSAN_VIEW_COMMON_DATATYPEPAGE_HPP
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
 *  \file        lusan/view/common/DataTypePage.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Data Types page shared by every document editor.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/data/common/DataTypeDataSection.hpp"
#include "lusan/model/common/DocIssue.hpp"
#include "lusan/view/common/IEditCommit.hpp"
#include "lusan/view/common/TableCell.hpp"

#include <QScrollArea>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypeCustom;
class DataTypeContainer;
class DataTypeDetailsView;
class DataTypeEnum;
class DataTypeFieldDetailsView;
class DataTypeImported;
class DataTypeListView;
class DataTypeModel;
class DataTypeStructure;
class DocumentElem;
class ElementBase;
class FieldEntry;
class QComboBox;
class QStringListModel;
class QTreeWidgetItem;

/**
 * \class   DataTypePage
 * \brief   The Data Types page: enumerations, structures, imported types and containers, edited
 *          the same way in every document that declares them. It drives the shared list, details
 *          and field widgets over a \ref DataTypeModel and knows nothing about the document
 *          behind it, so the service interface, the state machine and a standalone data type
 *          document all use this one controller.
 *
 *          Every edit is committed through the model's undo commands; the page mutates no data
 *          itself. It rebuilds the tree from the live model on the notifier signals the commands
 *          emit, rather than patching nodes by hand, so a row can never drift from what was
 *          stored -- including after an undo, a redo or a document reload.
 **/
class DataTypePage : public    QScrollArea
                   , public    IEditCommit
                   , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The column indexes of the data type tree.
     **/
    enum eColumn
    {
          ColName   = 0 //!< The data type or field name.
        , ColType   = 1 //!< The declared type: derived, imported object, container or field type.
        , ColValue  = 2 //!< The field value.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the page.
     * \param   model       The data types model of the document being edited.
     * \param   headline    The title shown above the two panels.
     * \param   parent      The parent widget.
     **/
    explicit DataTypePage(DataTypeModel& model, const QString& headline, QWidget* parent = nullptr);

    virtual ~DataTypePage(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel, so a caller elsewhere can start a new data type through
     *          its Add button.
     **/
    DataTypeListView* getList(void) const;

    /**
     * \brief   Selects and reveals a data type, or one of its structure fields / enumeration
     *          entries, by document ID (a validation finding about a field names the field, not
     *          the type that holds it). Does nothing if nothing has that ID.
     * \param   id      The document ID of the element to reveal.
     * \param   field   The field the caller wants accented once the row is selected.
     **/
    void revealElement(uint32_t id, eIssueField field = eIssueField::None);

    /**
     * \brief   Hands over the description text the page is still holding, for the type or for the
     *          field that is selected. The box applies its text when it loses the focus, which a
     *          save from the keyboard never causes.
     **/
    virtual void commitPendingEdits(void) override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
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
    void onAddFieldClicked(void);
    void onInsertFieldClicked(void);
    void onRemoveFieldClicked(void);
    void onMoveUpClicked(void);
    void onMoveDownClicked(void);

    void onNameCommitted(void);
    void onStructSelected(bool checked);
    void onEnumSelected(bool checked);
    void onImportSelected(bool checked);
    void onContainerSelected(bool checked);
    void onEnumDerivedChanged(int index);
    void onImportLocationCommitted(void);
    void onImportNamespaceCommitted(void);
    void onImportObjectCommitted(void);
    void onImportBrowse(void);
    void onContainerObjectChanged(int index);
    void onContainerKeyChanged(int index);
    void onContainerValueChanged(int index);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted(void);

    void onFieldNameCommitted(void);
    void onFieldTypeChanged(int index);
    void onFieldValueCommitted(void);
    void onFieldDeprecatedToggled(bool checked);
    void onFieldDeprecateHintCommitted(void);

    //!< Applies what was typed into a tree cell. Queued, so the inline editor is fully closed
    //!< before the tree is rebuilt underneath it.
    void onEditorDataChanged(const QModelIndex& index, const QString& newValue);

    //!< Mirrors the name or value being typed in a tree cell into the details panel, leaving the
    //!< inline editor open; the edit itself commits once, through onEditorDataChanged.
    void onEditorTextChanged(const QModelIndex& index, const QString& newText);

    //!< Fills the details panel from the model once an inline editor has closed.
    void onEditorClosed(void);

    //!< Rebuilds the whole tree on any DataType-kind notifier signal.
    void onNotifierChanged(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi(const QString& headline);
    void setupSignals(void);

    //!< Rebuilds the whole tree from the live model and restores the selection by ID.
    void refreshAll(void);
    //!< Selects the data type / field by ID; returns false if not found (and selects nothing).
    bool selectDataType(uint32_t typeId, uint32_t fieldId = 0);
    //!< Selects an imported type by its qualified name; returns false if no group carries it.
    bool selectImportedType(const QString& qualifiedName);
    //!< Clears the details/fields panels and disables the field-only tool buttons.
    void showClean(void);

    QTreeWidgetItem* createNode(DataTypeCustom* dataType) const;
    //!< Builds the heading of one included data type document with its types beneath it, every
    //!< row lock-marked to say it is declared elsewhere.
    QTreeWidgetItem* createImportNode(const DataTypeDataSection::ImportedTypes& group) const;
    //!< Greys the two detail forms out and disables the row tools, for a row this document reads
    //!< but does not own.
    void lockDetails(bool locked);
    void setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const;
    //!< Empty string if a structure field's default is valid for its type (a missing value,
    //!< or a declared enum/structure/container/imported type, is always valid), otherwise a
    //!< short, user-facing reason.
    QString validateFieldValue(const QString& typeName, const QString& value) const;
    //!< Empty string when the type a row declares is known to the document, otherwise the reason
    //!< behind the warning marker the row carries in the type column.
    static QString validateDeclaredType(const FieldEntry& field);
    static QString validateDeclaredType(const DataTypeCustom* dataType);
    static QString unknownTypeHint(const QString& typeName);

    void selectedStruct(DataTypeStructure* dataType);
    void selectedEnum(DataTypeEnum* dataType);
    void selectedImport(DataTypeImported* dataType);
    void selectedContainer(DataTypeContainer* dataType);
    void selectedStructField(DataTypeStructure* parent, uint32_t fieldId);
    void selectedEnumField(DataTypeEnum* parent, uint32_t fieldId);

    void activateFields(bool activate);
    void updateMoveButtons(int row, int rowCount);

    void populateTypeCombo(QComboBox* combo, const DataTypeCustom* exclude) const;
    void populateIntegerCombo(QComboBox* combo) const;
    void populateContainerObjectCombo(QComboBox* combo) const;

    //!< Refreshes the name lists the inline editors of the Data Type column offer.
    void populateInlineTypeNames(void);

    //!< The data type this page may edit, or nullptr. An imported row answers nullptr, which is
    //!< what keeps every editing path off a declaration another document owns.
    DataTypeCustom* currentDataType(void) const;
    //!< The data type of the selected row whether it is editable here or not.
    DataTypeCustom* selectedDataType(void) const;
    uint32_t currentFieldId(void) const;

    QString genTypeName(void);
    QString genFieldName(const DataTypeCustom* dataType) const;

    //!< Puts the caret in the Name field of the selected row, type or field, with its text
    //!< selected.
    void focusNameField(void);

    //!< Creates and selects a new data type of the given category (the Add split button and
    //!< its category drop-down entries all route here).
    void addNewType(DataTypeBase::eCategory category);

    //!< True if the given tree cell may be edited inline for its row's category.
    bool isCellEditable(const QModelIndex& index) const;

    //!< The name list to offer in the given cell's inline editor, or nullptr for a plain text
    //!< editor.
    QAbstractItemModel* editorModelFor(const QModelIndex& index) const;

    //!< The keystroke validation to apply to the given cell's inline text editor.
    TableCell::eCellValidation validationFor(const QModelIndex& index) const;

    //!< Commits an inline edit of the given tree row through the model's undo commands.
    void applyCellEdit(DataTypeCustom* dataType, uint32_t fieldId, int column, const QString& newValue);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeModel&              mModel;         //!< The data types of the document being edited.
    DataTypeListView*           mList;          //!< The list (tree) panel.
    DataTypeDetailsView*        mDetails;       //!< The data type details panel.
    DataTypeFieldDetailsView*   mFields;        //!< The field details panel.
    QStringListModel*           mFieldTypeNames;//!< The type names the inline field type editor offers.
    QStringListModel*           mIntegerNames;  //!< The integer names the inline derived type editor offers.
    TableCell*                  mTableCell;     //!< The inline editors of the tree.
    uint32_t                    mNameCounter;   //!< The counter behind the generated names.
    QString                     mCurUrl;        //!< The directory the import browser opens at.
    QString                     mCurFile;       //!< The file the import browser preselects.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DataTypePage(const DataTypePage& /*src*/) = delete;
    DataTypePage& operator = (const DataTypePage& /*src*/) = delete;
};

#endif  // LUSAN_VIEW_COMMON_DATATYPEPAGE_HPP

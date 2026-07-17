#ifndef LUSAN_VIEW_COMMON_TABLECELL_HPP
#define LUSAN_VIEW_COMMON_TABLECELL_HPP
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
 *  \file        lusan/view/common/TableCell.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ComboBox for the table cell.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QStyledItemDelegate>
#include <QHash>
#include <QString>

#include <functional>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QComboBox;
class QAbstractItemModel;
class QTableWidget;

//////////////////////////////////////////////////////////////////////////
// IETableHelper class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The IETableHelper class is an interface to provide the table helper
 *          to validate the data in the table cell editor.
 **/
class IETableHelper
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    IETableHelper() = default;
    virtual ~IETableHelper() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the number of columns in the table.
     **/
    virtual int getColumnCount() const = 0;

    /**
     * \brief   Returns the text of the cell.
     * \param   cell    The index of the cell.
     **/
    virtual QString getCellText(const QModelIndex & cell) const = 0;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    IETableHelper(const IETableHelper & /*src*/) = delete;
    IETableHelper(IETableHelper && /*src*/) = delete;
    IETableHelper& operator = (const IETableHelper & /*src*/) = delete;
    IETableHelper& operator = (IETableHelper && /*src*/) = delete;
};

//////////////////////////////////////////////////////////////////////////
// TableCell class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The TableCell class is a custom delegate to create a combo box
 *          in the table cell. The class is used to create a combo box in
 *          the table cell and populate the combo box with the data from
 *          the model. The class is used to create a combo box in the table
 *          cell and populate the combo box with the data from the model.
 *          The class is used to create a combo box in the table cell and
 *          populate the combo box with the data from the model.
 **/
class TableCell : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(TableCell)

//////////////////////////////////////////////////////////////////////////
// Public types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The keystroke validation to apply to a text (line-edit) column's inline editor.
     **/
    enum class eCellValidation
    {
          NoValidation      //!< No keystroke filtering (default).
        , Identifier        //!< C++ identifier characters only (names).
        , Path              //!< Include-path characters only (locations).
        , QualifiedName     //!< C++ qualified name (identifiers joined by '::'), e.g. imported type.
        , Value             //!< Enumeration value characters: letters, digits, '_' and '::'.
    };

    /**
     * \brief   Predicate that answers whether the cell at the given index may be edited.
     **/
    using FuncEditable = std::function<bool(const QModelIndex&)>;

    /**
     * \brief   Resolver that returns the combo-box model to use for the cell at the given index,
     *          or nullptr to use a plain line editor. Overrides the per-column model list.
     **/
    using FuncEditorModel = std::function<QAbstractItemModel*(const QModelIndex&)>;

    /**
     * \brief   Resolver that returns the keystroke validation to apply to the line editor of the
     *          cell at the given index. Overrides the per-column validation map.
     **/
    using FuncValidation = std::function<eCellValidation(const QModelIndex&)>;

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor with initialization.
     * \param   parent      The parent table widget.
     * \param   tableHelper The table helper object to validate the data.
     * \param   waitEndEdit If true, waits until editing has been finished then trigger signal.
     *                      If false, the signal is triggered each time the text is changed.
     *                      Valid only for the line edit widget. Ignored for combo-box.
     **/
    explicit TableCell(QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit);

    /**
     * \brief   Constructor with initialization.
     * \param   models      The list of models to populate the combo box.
     * \param   columns     The list of columns to create combo box.
     * \param   parent      The parent table widget.
     * \param   tableHelper The table helper object to validate the data.
     * \param   waitEndEdit If true, waits until editing has been finished then trigger signal.
     *                      If false, the signal is triggered each time the text is changed.
     *                      Valid only for the line edit widget. Ignored for combo-box.
     **/
    explicit TableCell(const QList<QAbstractItemModel*>& models, const QList<int>& columns, QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the keystroke validation applied to the inline editor of a text column.
     *          Only relevant for line-edit columns (combo columns ignore it). Lets a page
     *          forbid invalid characters directly in the table, matching the details panel.
     * \param   column  The column index to validate.
     * \param   kind    The validation kind (identifier, path, or none).
     **/
    void setColumnValidation(int column, eCellValidation kind);

    /**
     * \brief   Sets a predicate that decides, per cell, whether an inline editor may open. A page
     *          with a heterogeneous tree (e.g. Data Types) uses this so only the columns that make
     *          sense for the row's category are editable. When unset, every valid cell is editable.
     * \param   check   The predicate; returning false suppresses the editor for that index.
     **/
    void setEditableCheck(FuncEditable check);

    /**
     * \brief   Sets a resolver that picks the combo-box model per cell (or nullptr for a plain
     *          line editor), overriding the per-column model list. Lets one column show a combo
     *          for some rows and a text editor for others (e.g. struct field type vs imported type).
     * \param   resolver    The resolver; when unset, the per-column model list is used.
     **/
    void setEditorModelResolver(FuncEditorModel resolver);

    /**
     * \brief   Sets a resolver that picks the keystroke validation per cell, overriding the
     *          per-column validation map. Lets the same column validate differently by row
     *          category (e.g. a struct default value vs an enumeration value).
     * \param   resolver    The resolver; when unset, the per-column validation map is used.
     **/
    void setValidationResolver(FuncValidation resolver);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Creates the editor widget to edit the data in the table cell.
     * \param   parent      The parent widget.
     * \param   option      The style option.
     * \param   index       The index of the table cell.
     * \return  Returns the created editor widget.
     **/
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * \brief   Sets the data to the editor widget.
     * \param   editor      The editor widget.
     * \param   index       The index of the table cell.
     **/
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    /**
     * \brief   Sets the data to the model.
     * \param   editor      The editor widget.
     * \param   model       The model to set the data.
     * \param   index       The index of the table cell.
     **/
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal is triggered when the data in the table cell is changed.
     * \param   index       The index of the table cell.
     * \param   newValue    The new value of the table cell.
     **/
    void signalEditorDataChanged(const QModelIndex &index, const QString &newValue);
    
//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   The slot is triggered when the user picks an item in the combo box editor. It emits
     *          signalEditorDataChanged with the chosen text and closes the editor so the drop-down
     *          does not linger. It reacts to user activation only (never to programmatic combo
     *          changes), so re-seeding the editor during a commit cannot revert the selection.
     * \param   index   The index of the activated item (unused).
     **/
    void onComboActivated(int index);

    /**
     * \brief   The slot is triggered when the text in the editor widget is changed.
     * \param   newText     The new text in the editor widget.
     **/
    void onEditorTextChanged(const QString & newText);
    
    /**
     * \brief   The slot is triggered when the text editing in the editor widget is finished.
     *          This slot is used to handle the event when the user has finished editing the text
     *          in the editor widget. It validates the new text and updates the model accordingly.
     **/
    void onEditorTextChangeFinished();

    /**
     * \brief   The slot is triggered when the view closes the editor. On Escape the view reports
     *          RevertModelCache: the edit is cancelled instead of leaving the live per-keystroke
     *          changes applied. In live mode (mWaitEnd == false) the pre-edit value is replayed
     *          through signalEditorDataChanged so the owning controller restores the model, the
     *          cell and the details panel; in wait-for-end mode the pending text is discarded.
     * \param   editor  The editor widget being closed.
     * \param   hint    The end-edit hint reported by the view.
     **/
    void onCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Returns true if the given column index is valid within the parent QTableWidget object.
     * \param   col     The column index.
     **/
    bool isValidColumn(int col) const;

    /**
     * \brief   Returns true if the given column index is a combo box widget.
     * \param   col     The column index.
     **/
    bool isComboWidget(int col) const;

    /**
     * \brief   Returns the model of the given column.
     * \param   col     The column index.
     * \return  Returns the model of the given column.
     **/
    QAbstractItemModel* columnToModel(int col) const;

//////////////////////////////////////////////////////////////////////////
// Hidden variables
//////////////////////////////////////////////////////////////////////////
private:
    QList<QAbstractItemModel*>  mModels;    //!< The list of models to populate the combo box.
    QList<int>                  mColumns;   //!< The list of columns to create combo box.
    QHash<int, eCellValidation> mValidation;//!< Per text-column keystroke validation for inline editors.
    FuncEditable                mEditable;  //!< Optional per-cell editability predicate (heterogeneous trees).
    FuncEditorModel             mModelOf;   //!< Optional per-cell combo-model resolver (overrides mColumns).
    FuncValidation              mValidOf;   //!< Optional per-cell validation resolver (overrides mValidation).
    QWidget *                   mParent;    //!< The parent table widget.
    IETableHelper*              mTable;     //!< The table helper object to validate the data.
    bool                        mWaitEnd;   //!< Wait for end of editing. Valid only for line editor, no relevant to combobox.
    mutable QString             mNewText;   //!< The changed text.
    mutable QModelIndex         mSelIndex;  //!< The index of selected item.
    mutable QString             mEditOriginal;//!< The cell text captured when the editor opened; replayed to cancel (Esc).
};

#endif // LUSAN_VIEW_COMMON_TableCell_HPP

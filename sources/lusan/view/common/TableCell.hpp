#ifndef LUSAN_VIEW_COMMON_TABLECELL_HPP
#define LUSAN_VIEW_COMMON_TABLECELL_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/TableCell.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ComboBox for the table cell.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QStyledItemDelegate>
#include <QString>

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
    IETableHelper(void) = default;
    virtual ~IETableHelper(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the number of columns in the table.
     **/
    virtual int getColumnCount(void) const = 0;

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
    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    /**
     * \brief   Sets the data to the editor widget.
     * \param   editor      The editor widget.
     * \param   index       The index of the table cell.
     **/
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    /**
     * \brief   Sets the data to the model.
     * \param   editor      The editor widget.
     * \param   model       The model to set the data.
     * \param   index       The index of the table cell.
     **/
    virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    
//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal is triggered when the data in the table cell is changed.
     * \param   index       The index of the table cell.
     * \param   newValue    The new value of the table cell.
     **/
    void editorDataChanged(const QModelIndex &index, const QString &newValue) const;
    
//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   The slot is triggered when the current text in the combo box is changed.
     * \param   newText     The new text in the combo box.
     **/
    void onComboTextChanged(const QString & newText);

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
    void onEditorTextChangeFinished(void);
    
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
    QWidget *                   mParent;    //!< The parent table widget.
    IETableHelper*              mTable;     //!< The table helper object to validate the data.
    bool                        mWaitEnd;   //!< Wait for end of editing. Valid only for line editor, no relevant to combobox.
    mutable QString             mNewText;   //!< The changed text.
    mutable QModelIndex         mSelIndex;  //!< The index of selected item.
};

#endif // LUSAN_VIEW_COMMON_TableCell_HPP

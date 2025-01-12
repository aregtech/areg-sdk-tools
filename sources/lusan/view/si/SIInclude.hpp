#ifndef LUSAN_APPLICATION_SI_SIINCLUDE_HPP
#define LUSAN_APPLICATION_SI_SIINCLUDE_HPP
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
 *  \file        lusan/view/si/SIInclude.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QScrollArea>
#include "lusan/view/common/IEDataTypeConsumer.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
namespace Ui {
    class SIInclude;
}
    
class IncludeEntry;
class SIIncludeDetails;
class SIIncludeList;
class SIIncludeModel;
class TableCell;

/**
 * \brief   The widget object for SIInclude page.
 **/
class SIIncludeWidget : public QWidget
{
    friend class SIInclude;

    Q_OBJECT

public:
    explicit SIIncludeWidget(QWidget* parent);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    Ui::SIInclude* ui;
};

/**
 * \brief   The SIInclude class is the view of the Service Interface
 *          Include section. It displays the list of included files
 *          and allows to add, remove, update, and insert new entries.
 **/
class SIInclude : public QScrollArea
                , public IEDataTypeConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor with initialization.
     * \param   model   The model of the SIInclude section.
     * \param   parent  The parent widget of the section.
     **/
    explicit SIInclude(SIIncludeModel & model, QWidget* parent = nullptr);

    virtual ~SIInclude(void);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:

    /**
     * \brief   Triggered when the current cell is changed.
     * \param   currentRow      The current row index.
     * \param   currentColumn   The current column index.
     * \param   previousRow     The previous row index.
     * \param   previousColumn  The previous column index.
     **/
    void onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    /**
     * \brief   Triggered when the add button is clicked.
     **/
    void onAddClicked(void);

    /**
     * \brief   Triggered when the remove button is clicked.
     **/
    void onRemoveClicked(void);

    /**
     * \brief   Triggered when the insert button is clicked.
     **/
    void onInsertClicked(void);

    /**
     * \brief   Triggered when the browse button is clicked.
     **/
    void onBrowseClicked(void);

    /**
     * \brief   Triggered when the include field is changed.
     **/
    void onIncludeChanged(const QString & newText);
    
    /**
     * \brief   Triggered when the description text is changed.
     **/
    void onDescriptionChanged(void);
    
    /**
     * \brief   Triggered when the deprecated check box is clicked.
     **/
    void onDeprectedClicked(Qt::CheckState newState);
    
    /**
     * \brief   Triggered when the deprecation hint field is changed.
     **/
    void onDeprecateHint(const QString & newText);

    /**
     * \brief   Triggered when the cell editor data is changed.
     * \param   index       The index of the cell.
     * \param   newValue    The new value of the cell.
     **/
    void onEditorDataChanged(const QModelIndex &index, const QString &newValue);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    
    /**
     * \brief   Initializes the SIConstant object.
     **/
    void updateWidgets(void);

    /**
     * \brief   Initializes the SIInclude object.
     **/
    void updateData(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

    /**
     * \brief   Returns the list of supported file extensions.
     **/
    static QStringList getSupportedExtensions(void);

    /**
     * \brief   Finds and returns valid pointer to the include object entry in the specified row.
     * \param   row     The row index of the include entry.
     **/
    inline IncludeEntry* _findInclude(int row);    
    inline const IncludeEntry* _findInclude(int row) const;

    /**
     * \brief   Adds new include entry at the specified row.
     * \param   row     The row index to add new include object
     **/
    inline void _addInclude(int row);
    
    /**
     * \brief   Blocks the basic signals.
     * \param   doBlock     If true, blocks the signals, otherwise unblocks.
     **/
    inline void blockBasicSignals(bool doBlock);
    
    /**
     * \brief   Triggered when the cell data is changed to update other controls.
     **/
    inline void cellChanged(int row, int col, const QString& newValue);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeModel&     mModel;     //!< The model of the SIInclude section.
    SIIncludeDetails*   mDetails;   //!< The details page.
    SIIncludeList*      mList;      //!< The list page.
    SIIncludeWidget*    mWidget;    //!< The widget object.
    Ui::SIInclude&      ui;         //!< The UI object.
    TableCell*          mTableCell; //!< The table cell.

    QString             mCurUrl;    //!< The current URL.
    QString             mCurFile;   //!< The current file.
    QString             mCurFilter; //!< The current filter.
    int                 mCurView;   //!< The current view mode.
    
    uint32_t            mCount;     //!< The counter to generate names.
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDE_HPP

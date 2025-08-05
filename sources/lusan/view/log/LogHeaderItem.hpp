#ifndef LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
#define LUSAN_VIEW_LOG_LOGHEADERITEM_HPP
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
 *  \file        lusan/view/log/LogHeaderItem.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/log/LiveLogsModel.hpp"

#include <QFrame>
#include <QList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LogTableHeader;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QLineEdit;
class QListWidget;

/************************************************************************
 * Implemented classes
 ************************************************************************/

class LogComboFilter;
class LogTextFilter;
class LogHeaderItem;

//////////////////////////////////////////////////////////////////////////
// LogHeaderItem class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   Header item, which contains visual elements to visualize by need.
 **/
class LogHeaderItem : public QObject
{
    Q_OBJECT

private:
/************************************************************************
 * Implemented classes
 ************************************************************************/

    //!< The type of visual object to display
    enum eType
    {
          None  //!< Nothing to display
        , Combo //!< Display combo-box
        , Text  //!< Display line-editor
    };

public:
    /**
     * \brief   Initialize the elements
     * \param   header  The header object
     * @param   column  The virtual column, based on LiveLogsModel
     * @param   text    The header name to display
     */
    LogHeaderItem(LogTableHeader& header, int logicalIndex);

    /**
     * \brief   Visualize the filter widgets
     **/
    void showFilters(void);

    /**
     * \brief   Sets the filter string for line edit filter control.
     **/
    void setFilterData(const QString & data);

    /**
     * \brief   Sets the list of strings in the combo-box filter control
     **/
    void setFilterData(const std::vector<String> & data);
    
    /**
     * \brief   Sets the list of integers in the combo-box filter control
     **/
    void setFilterData(const std::vector<ITEM_ID> & data);

    /**
     * \brief   Returns true if header object can be visualized in the pop-up widget.
     **/
    inline bool canPopupFilter(void) const;

    /**
     * \brief   Resets filter data.
     **/
    void resetFilter(void);

private:
/************************************************************************
 * Hidden methods
 ************************************************************************/

    //!< Returns the logical index of the column.
    //!< Returns `-1` if the column is not active.
    inline int fromColumnToIndex(void) const;

    //!< Returns the column from the index.
    //!< Return LogColumnInvalid value if index is invalid.
    inline LoggingModelBase::eColumn fromIndexToColumn(int logicalIndex) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:

    LoggingModelBase::eColumn mColumn;  //!< The index of the header item.
    eType           mType;      //!< Type of the header item.
    LogTableHeader& mHeader;    //!< The header object, which contains this item.
    LogComboFilter* mCombo;     //!< The combo-box filter, if applicable.
    LogTextFilter*  mEdit;      //!< The line editor filter, if applicable.
};

//////////////////////////////////////////////////////////////////////////
// LogComboFilter class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   A class to display as a combo-box entries when click on header.
 **/
class LogComboFilter : public QFrame
{
    Q_OBJECT

private:
/************************************************************************
 * Internal types and constants
 ************************************************************************/
    //!< Structure is used to filter combo-box entries
    struct sComboItem
    {
        QString text{};         //!< The string in combo-box
        bool    checked{false}; //!< Flag, indicating whether the element is selected or not
    };
    
public:
    explicit LogComboFilter(QWidget* parent = nullptr);

    /**
     * \brief   Updates and sets the items of combo-box
     * \param   items   The list of entries to set in combo-box
     **/
    void setItems(const QStringList& items);
    
    /**
     * \brief   Returns list of selected (checked) entries.
     **/
    QStringList getCheckedItems() const;

    /**
     * \brief   Clears filter data.
     **/
    void clearFilter(void);

    /**
     * \brief   Shows the filter widget and set focus on the list widget.
     **/
    void showFilter(void);

/************************************************************************
 * Signals
 ************************************************************************/
signals:
    /**
     * \brief   The signal, which is triggered when an element from combo-box is selected.
     **/
    void signalFiltersChanged(void);

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
private:
    QListWidget*        mListWidget;    //!< The list widget to display as a bombo-box.
};

//////////////////////////////////////////////////////////////////////////
// LogTextFilter class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   A visual element to type a string to search and filter
 **/
class LogTextFilter : public QFrame
{
    Q_OBJECT

public:
    /**
     * \brief   Creates the simple or extended text filter object.
     *          The extended text filter is an instance of SearchLineEdit and contains additional tool-buttons
     * \param   isExtended  If true, the text filter is extended and contains additional tool-buttons for match case, match word, wild card, and backward search.
     * \param   parent  The parent widget of the text filter.
     **/
    explicit LogTextFilter(bool isExtended, QWidget* parent = nullptr);

    /**
     * \brief   Returns types string in the line edit
     **/
    QString getText() const;

    /**
     * \brief   Sets the text in the line edit.
     **/
    void setText(const QString & newText);

    /**
     * \brief   Clears filter data.
     **/
    void clearFilter(void);

    /**
     * \brief   Shows the filter widget and set focus on the line edit widget.
     **/
    void showFilter(void);

signals:
/************************************************************************
 * Signals
 ************************************************************************/

    /**
     * \brief   A signal triggered when user types a text inside editor
     * \param   text    A new text.
     */
    void signalFilterTextChanged(const QString& text, bool isCaseSensitive, bool isWholeWord, bool isWidlCard);
    
private slots:
/************************************************************************
 * Slots
 ************************************************************************/

    /**
     * \brief   Slot triggered when the tool-button is checked or unchecked.
     *          The slot emits signalFilterTextChanged with the current text and flags.
     * \param   checked     Is set true if tool-button is checked, false otherwise.
     **/
    void slotToolbuttonChecked(bool checked);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLineEdit* mLineEdit;   //!< Line edit controller
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline bool LogHeaderItem::canPopupFilter(void) const
{
    return (mType != None);    
}

#endif  // LUSAN_VIEW_LOG_LOGHEADERITEM_HPP

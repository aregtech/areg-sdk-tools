#ifndef LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP
#define LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP
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
 *  \file        lusan/view/common/SearchLineEdit.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, QLineEdit control with tool-buttons for search.
 *
 ************************************************************************/

#include "areg/base/GEGlobal.h"
#include <QLineEdit>
#include <QToolButton>
#include <QSize>

/**
 * \brief   The QLineEdit control for search or filter with integrated tool-buttons
 *          for match case, match word, wild card, backward search and search.
 **/
class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Type of tool-buttons to add to the search line edit.
     */
    enum class eToolButton  : uint32_t
    {
          ToolButtonNothing     = 0     //!< No tool button to add
        , ToolButtonMatchCase   = 1     //!< Add match case checkable tool-button with icon.
        , ToolButtonMatchWord   = 4     //!< Add match word checkable tool-button with icon.
        , ToolButtonWildCard    = 8     //!< Add wild-cart checkable tool-button with icon.
        , ToolButtonBackward    = 16    //!< Add search backward checkable tool-button with icon.
        , ToolButtonSearch      = 32    //!< Add search next tool-button with icon.
    };

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    explicit SearchLineEdit(const QList<SearchLineEdit::eToolButton> & addButtons, QSize buttonSize = QSize(16, 16), QWidget* parent = nullptr);

    /**
     * \brief   Creates search line edit object with integrated tool-buttons.
     *          The type of tool-buttons to add is specified in the \p addButtons parameter.
     *          Each entry of \p addButtons is a flag of type SearchLineEdit::eToolButton indicating the tool-buttons to add
     *          and the order of buttons is the same as in the list.
     * \param   addButtons  Specifies the list of the type and the order of tool-buttons to add.
     * \param   buttonSize  Specifies the size of the tool-buttons to add. By default, it is 20x20 pixels.
     * \param   parent      The parent widget of the search line edit.
     **/
    explicit SearchLineEdit(const QList<SearchLineEdit::eToolButton> & addButtons, QSize buttonSize = QSize(20, 20), QWidget* parent = nullptr);

    /**
     * \brief   Creates search line edit object without tool-butons. Call \p initialize() method to add tool-buttons.
     * \param   parent      The parent widget of the search line edit.
     **/
    explicit SearchLineEdit(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Initializes the search line edit with the specified tool-buttons.
     *          The type of tool-buttons to add is specified in the \p addButtons parameter.
     *          Each entry of \p addButtons is a flag of type SearchLineEdit::eToolButton indicating the tool-buttons to add
     *          and the order of buttons is the same as in the list.
     *          If the search line edit is already initialized and contains tool-buttons, this method does nothing.
     * \param   addButtons  Specifies the list of the type and the order of tool-buttons to add.
     * \param   buttonSize  Specifies the size of the tool-buttons to add. By default, it is 20x20 pixels.
     **/
    void initialize(const QList<SearchLineEdit::eToolButton> & addButtons, QSize buttonSize = QSize(16, 16));

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    //!< Returns the tool button for match case.
    inline QToolButton* buttonMatchCase(void) const;

    //!< Returns the tool button for match word.
    inline QToolButton* buttonMatchWord(void) const;

    //!< Returns the tool button for wild card.
    inline QToolButton* buttonWildCard(void) const;

    //!< Returns the tool button for search backward.
    inline QToolButton* buttonSearchBackward(void) const;

    //!< Returns the tool button for search next.
    inline QToolButton* buttonSearch(void) const;

    //!< Returns true if match case tool-button exists and checked.
    inline bool isMatchCaseChecked(void) const;

    //!< Returns true if match word tool-button exists and checked.
    inline bool isMatchWordChecked(void) const;

    //!< Returns true if wild card tool-button exists and checked.
    inline bool isWildCardChecked(void) const;

    //!< Returns true if search backward tool-button exists and checked.
    inline bool isBackwardChecked(void) const;

//////////////////////////////////////////////////////////////////////////
// SearchLineEdit class signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   Signal emitted when the search match case tool-button is checked or unchecked.
     * \param   checked     Is set true if search match case tool-button is checked, false otherwise.
     **/
    void signalButtonSearchMatchCaseClicked(bool checked);

    /**
     * \brief   Signal emitted when the search match word tool-button is checked or unchecked.
     * \param   checked     Is set true if search match word tool-button is checked, false otherwise.
     **/
    void signalButtonSearchMatchWordClicked(bool checked);

    /**
     * \brief   Signal emitted when the search wild-card tool-button is checked or unchecked.
     * \param   checked     Is set true if search wild-card tool-button is checked, false otherwise.
     **/
    void signalButtonSearchWildCardClicked(bool checked);

    /**
     * \brief   Signal emitted when the search backward tool-button is checked or unchecked.
     * \param   checked     Is set true if search backward tool-button is checked, false otherwise.
     **/
    void signalButtonSearchBackwardClicked(bool checked);

    /**
     * \brief   Signal emitted when the search next button is clicked.
     * \param   checked     Is set true if search button is checked, false otherwise.
     **/
    void signalButtonSearchClicked(bool checked);

    /**
     * \brief   Signal emitted when the search text is changed.
     * \param   newText     The new text of the search line edit.
     **/
    void signalSearchTextChanged(const QString & newText);

    /**
     * \brief   Signal emitted when the search text is requested, i.e. when search next button is clicked.
     * \param   text        The text to search.
     * \param   isMatchCase If true, the search is case sensitive.
     * \param   isWholeWord If true, the search matches whole words only.
     * \param   isWildCard  If true, the search uses wild-card characters.
     * \param   isBackward  If true, the search is backward.
     **/
    void signalSearchText(const QString& text, bool isMatchCase, bool isWholeWord, bool isWildCard, bool isBackward);

    /**
     * \brief   Signal emitted when the filter text is changed, i.e. each time the text is changed.
     * \param   text        The text to filter.
     * \param   isMatchCase If true, the filter is case sensitive.
     * \param   isWholeWord If true, the filter matches whole words only.
     * \param   isWildCard  If true, the filter uses wild-card characters.
     * \param   isBackward  If true, the filter is backward.
     **/
    void signalFilterText(const QString& text, bool isMatchCase, bool isWholeWord, bool isWildCard, bool isBackward);
    
//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   QLineEdit event triggered when the search line edit is resized.
     **/
    virtual void resizeEvent(QResizeEvent *event) override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    bool            mIsInitialized; //!< Indicates if the search line edit is initialized with tool-buttons.
    QWidget*        mToolButtons;   //!< The widget that contains the tool-buttons.
    QToolButton*    mBtnSearch;     //!< The tool button for search next.
    QToolButton*    mBtnMatchCase;  //!< The tool button for match case.
    QToolButton*    mBtnMatchWord;  //!< The tool button for match word.
    QToolButton*    mBtnWildCard;   //!< The tool button for wild card.
    QToolButton*    mBtnBackward;   //!< The tool button for search backward.
    uint32_t        mButtonFlags;   //!< The flags indicating which tool-buttons are added.
    QList<SearchLineEdit::eToolButton> mButtons;    //<!< The list of tool-buttons added to the search line edit.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    DECLARE_NOCOPY_NOMOVE(SearchLineEdit);
};

//////////////////////////////////////////////////////////////////////////
// SearchLineEdit inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline QToolButton* SearchLineEdit::buttonMatchCase(void) const
{
    return mBtnMatchCase;
}

inline QToolButton* SearchLineEdit::buttonMatchWord(void) const
{
    return mBtnMatchWord;
}

inline QToolButton* SearchLineEdit::buttonWildCard(void) const
{
    return mBtnWildCard;
}

inline QToolButton* SearchLineEdit::buttonSearchBackward(void) const
{
    return mBtnBackward;
}

inline QToolButton* SearchLineEdit::buttonSearch(void) const
{
    return mBtnSearch;
}

inline bool SearchLineEdit::isMatchCaseChecked(void) const
{
    return (mBtnMatchCase != nullptr) && mBtnMatchCase->isChecked();
}

inline bool SearchLineEdit::isMatchWordChecked(void) const
{
    return (mBtnMatchWord != nullptr) && mBtnMatchWord->isChecked();
}

inline bool SearchLineEdit::isWildCardChecked(void) const
{
    return (mBtnWildCard != nullptr) && mBtnWildCard->isChecked();
}

inline bool SearchLineEdit::isBackwardChecked(void) const
{
    return (mBtnBackward != nullptr) && mBtnBackward->isChecked();
}

#endif  // LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP

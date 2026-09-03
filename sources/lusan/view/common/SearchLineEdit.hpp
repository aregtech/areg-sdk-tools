#ifndef LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP
#define LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP
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
 *  \file        lusan/view/common/SearchLineEdit.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, QLineEdit control with tool-buttons for search.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "areg/base/areg_global.h"

#include <QLineEdit>

// The inline option accessors ask a button whether it is checked, so the type must be complete.
#include <QToolButton>

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;
class QWidget;

//////////////////////////////////////////////////////////////////////////
// SearchLineEdit class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The search field of the application. It draws a leading mark that names it, the
 *          option toggles the surface asks for, an optional match counter and a clear button,
 *          all inside the field itself.
 *
 *          Every surface that searches or filters uses it, so the log window, the two scope
 *          panels, the log column filters and the state machine canvas offer one control with
 *          one look and one set of gestures.
 *
 * \note    The field owns no keyboard shortcut. Ctrl+F, F3 and Escape belong to the window
 *          that holds it, which is what keeps two surfaces from claiming the same key.
 **/
class SearchLineEdit : public QLineEdit
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The option toggles a surface can ask for, in the order they are drawn.
     **/
    enum class eToolButton  : uint32_t
    {
          ToolButtonNothing     = 0     //!< No toggle at all.
        , ToolButtonMatchCase   = 1     //!< Tell an upper case letter from a lower case one.
        , ToolButtonMatchWord   = 4     //!< Match whole words only.
        , ToolButtonWildCard    = 8     //!< Read the text as a pattern.
        , ToolButtonBackward    = 16    //!< Walk the matches backwards.
    };

    //!< The air the field keeps between the typed text and the marks at its right end.
    static constexpr int    SEARCH_AIR      { 4 };

    //!< The smallest square an option toggle may be drawn in.
    static constexpr int    SEARCH_BUTTON_MIN { 14 };

    //!< How much smaller the icon is than the toggle that holds it.
    static constexpr int    SEARCH_ICON_INSET { 6 };

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Creates the field with the given option toggles.
     * \param   addButtons  The toggles to draw, in the order given.
     * \param   parent      The parent widget.
     **/
    explicit SearchLineEdit(const QList<SearchLineEdit::eToolButton>& addButtons, QWidget* parent = nullptr);

    /**
     * \brief   Creates the field with the given option toggles and leading mark.
     * \param   addButtons  The toggles to draw, in the order given.
     * \param   mark        The mark drawn at the left end, which names what the field does.
     * \param   parent      The parent widget.
     **/
    explicit SearchLineEdit(const QList<SearchLineEdit::eToolButton>& addButtons, const QIcon& mark, QWidget* parent = nullptr);

    /**
     * \brief   Creates the field without toggles. Call initialize() to add them.
     * \param   parent      The parent widget.
     **/
    explicit SearchLineEdit(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Draws the leading mark, the given toggles and the clear button. Does nothing
     *          when the field is already initialized.
     * \param   addButtons  The toggles to draw, in the order given.
     **/
    void initialize(const QList<SearchLineEdit::eToolButton>& addButtons, const QIcon& mark = QIcon());

    /**
     * \brief   Returns the width the field spends on its own marks, at both ends. A caller
     *          that sizes the field adds the room it wants for the text to it.
     **/
    inline int chromeWidth() const;

    /**
     * \brief   Shows the given text between the typed phrase and the toggles, where a surface
     *          reports how many matches it found. An empty text hides it.
     * \param   text    The line to show, for example "3 of 17" or "none".
     **/
    void setCounter(const QString& text);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    //!< Returns the match case toggle, or nullptr when the surface did not ask for it.
    inline QToolButton* buttonMatchCase() const;

    //!< Returns the whole word toggle, or nullptr when the surface did not ask for it.
    inline QToolButton* buttonMatchWord() const;

    //!< Returns the pattern toggle, or nullptr when the surface did not ask for it.
    inline QToolButton* buttonWildCard() const;

    //!< Returns the backwards toggle, or nullptr when the surface did not ask for it.
    inline QToolButton* buttonSearchBackward() const;

    //!< Returns true if the match case toggle exists and is on.
    inline bool isMatchCaseChecked() const;

    //!< Returns true if the whole word toggle exists and is on.
    inline bool isMatchWordChecked() const;

    //!< Returns true if the pattern toggle exists and is on.
    inline bool isWildCardChecked() const;

    //!< Returns true if the backwards toggle exists and is on.
    inline bool isBackwardChecked() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    //!< Emitted when the match case toggle changes.
    void signalButtonSearchMatchCaseClicked(bool checked);

    //!< Emitted when the whole word toggle changes.
    void signalButtonSearchMatchWordClicked(bool checked);

    //!< Emitted when the pattern toggle changes.
    void signalButtonSearchWildCardClicked(bool checked);

    //!< Emitted when the backwards toggle changes.
    void signalButtonSearchBackwardClicked(bool checked);

    //!< Emitted when the field is asked to move to the next match.
    void signalButtonSearchClicked(bool checked);

    /**
     * \brief   Emitted whenever the typed text changes.
     * \param   newText     The new text of the field.
     **/
    void signalSearchTextChanged(const QString& newText);

    /**
     * \brief   Emitted when the field is asked to move to the next match.
     * \param   text        The text to search.
     * \param   isMatchCase True if the search tells the cases apart.
     * \param   isWholeWord True if the search matches whole words only.
     * \param   isWildCard  True if the text is a pattern.
     * \param   isBackward  True if the search walks backwards.
     **/
    void signalSearchText(const QString& text, bool isMatchCase, bool isWholeWord, bool isWildCard, bool isBackward);

    /**
     * \brief   Emitted on every change of the text or of an option, for a surface that
     *          narrows a list while the text is typed.
     * \param   text        The text to filter by.
     * \param   isMatchCase True if the filter tells the cases apart.
     * \param   isWholeWord True if the filter matches whole words only.
     * \param   isWildCard  True if the text is a pattern.
     * \param   isBackward  True if the search walks backwards.
     **/
    void signalFilterText(const QString& text, bool isMatchCase, bool isWholeWord, bool isWildCard, bool isBackward);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    //!< Keeps the marks in place when the field changes width.
    void resizeEvent(QResizeEvent* event) override;

    //!< Answers Enter and F3. Escape is left to the window that holds the field.
    void keyPressEvent(QKeyEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< Builds one option toggle and appends it to the trailing group.
    QToolButton* _addOption(const QIcon& icon, const QString& toolTip, const QString& name);

    //!< Puts the leading mark and the trailing group where the current width wants them.
    void _placeMarks(void);

    //!< Emits the filter signal with the current text and options.
    void _emitFilter(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    bool            mIsInitialized; //!< True once the marks are built.
    QWidget*        mTrailing;      //!< The group drawn at the right end of the field.
    QLabel*         mCounter;       //!< What the surface reports about its matches.
    QToolButton*    mBtnClear;      //!< Empties the field. Hidden while the field is empty.
    QToolButton*    mBtnMatchCase;  //!< The match case toggle.
    QToolButton*    mBtnMatchWord;  //!< The whole word toggle.
    QToolButton*    mBtnWildCard;   //!< The pattern toggle.
    QToolButton*    mBtnBackward;   //!< The backwards toggle.
    int             mTrailWidth;    //!< The width the trailing group had when the margins were set.
    int             mLeadWidth;     //!< The width the leading mark takes.
    int             mBoxExtent;     //!< The square an option toggle is drawn in, cut to the field.
    int             mIconExtent;    //!< The icon edge inside a toggle.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    AREG_NOCOPY_NOMOVE(SearchLineEdit);
};

//////////////////////////////////////////////////////////////////////////
// SearchLineEdit class inline methods
//////////////////////////////////////////////////////////////////////////

inline int SearchLineEdit::chromeWidth() const
{
    return mLeadWidth + textMargins().right();
}

inline QToolButton* SearchLineEdit::buttonMatchCase() const
{
    return mBtnMatchCase;
}

inline QToolButton* SearchLineEdit::buttonMatchWord() const
{
    return mBtnMatchWord;
}

inline QToolButton* SearchLineEdit::buttonWildCard() const
{
    return mBtnWildCard;
}

inline QToolButton* SearchLineEdit::buttonSearchBackward() const
{
    return mBtnBackward;
}

inline bool SearchLineEdit::isMatchCaseChecked() const
{
    return (mBtnMatchCase != nullptr) && mBtnMatchCase->isChecked();
}

inline bool SearchLineEdit::isMatchWordChecked() const
{
    return (mBtnMatchWord != nullptr) && mBtnMatchWord->isChecked();
}

inline bool SearchLineEdit::isWildCardChecked() const
{
    return (mBtnWildCard != nullptr) && mBtnWildCard->isChecked();
}

inline bool SearchLineEdit::isBackwardChecked() const
{
    return (mBtnBackward != nullptr) && mBtnBackward->isChecked();
}

#endif  // LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP

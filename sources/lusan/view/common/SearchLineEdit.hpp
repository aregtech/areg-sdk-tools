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

#include <QLineEdit>
#include <QToolButton>
#include <QSize>

class SearchLineEdit : QLineEdit
{
    Q_OBJECT

public:
    enum class eToolButton  : uint32_t
    {
          ToolButtonNothing     = 0
        , ToolButtonMatchCase   = 1
        , ToolButtonMatchWord   = 4
        , ToolButtonBackward    = 8
        , ToolButtonSearch      = 16
    };

public:
    explicit SearchLineEdit(const QList<SearchLineEdit::eToolButton> & addButtons, QSize buttonSize = QSize(16, 16), QWidget* parent = nullptr);

    inline QToolButton* buttonMatchCase(void);
    inline QToolButton* buttonMatchWord(void);
    inline QToolButton* buttonSearchBackward(void);
    inline QToolbutton* buttonSearch(void);

public signals:

    void signalButtonSearchMatchCaseClicked(bool checked);

    void signalButtonSearchWholeWordClicked(bool checked);

    void signalButtonSearchBackwardClicked(bool checked);

    void signalButtonSearchClicked(bool checked);

private:
    QToolButton*    mBtnSearch;
    QToolButton*    mBtnMatchCase;
    QToolButton*    mBtnMatchWord;
    QToolButton*    mBtnBackward;
    uint32_t        mButtonFlags;
    QList<SearchLineEdit::eToolButton> mButtons;
};

//////////////////////////////////////////////////////////////////////////
// SearchLineEdit inline methods implementation
//////////////////////////////////////////////////////////////////////////

inline QToolButton* SearchLineEdit::buttonMatchCase(void)
{
    return mBtnMatchCase;
}

inline QToolButton* SearchLineEdit::buttonMatchWord(void)
{
    return mBtnMatchWord;
}

inline QToolButton* SearchLineEdit::buttonSearchBackward(void)
{
    return mBtnBackward;
}

inline QToolbutton* SearchLineEdit::buttonSearch(void)
{
    return mBtnSearch;
}

#endif  // LUSAN_VIEW_COMMON_SEARCHLINEEDIT_HPP

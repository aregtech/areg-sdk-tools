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
 *  \file        lusan/view/log/LogHeaderTextFilter.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/
#include "lusan/view/log/LogHeaderTextFilter.hpp"
#include "lusan/model/log/LoggingModelBase.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>

LogHeaderTextFilter::LogHeaderTextFilter(LoggingModelBase* model, bool isExtended, QWidget* parent)
    : QFrame    ( )
    , mModel    (model)
    , mLineEdit (nullptr)
    , mExtended (isExtended)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::Box);
    if (isExtended)
    {
        mLineEdit = new SearchLineEdit(QList<SearchLineEdit::eToolButton>{SearchLineEdit::eToolButton::ToolButtonMatchCase, SearchLineEdit::eToolButton::ToolButtonMatchWord, SearchLineEdit::eToolButton::ToolButtonWildCard}, QSize(20, 20), this);
        connect(mLogSearch, &SearchLineEdit::signalSearchText, this
            , [this](bool enterPressed, const QString& text, bool isMatchCase, bool isWholeWord, bool isWildCard, bool /*isBackward*/) {
                if (enterPressed)
                    emit signalFilterChanged(text, isCaseSensitive, isWholeWord, isWildCard);
            });
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchMatchCaseClicked, this, &LogTextFilter::slotToolbuttonChecked);
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchMatchWordClicked, this, &LogTextFilter::slotToolbuttonChecked);
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchWildCardClicked , this, &LogTextFilter::slotToolbuttonChecked);
    }
    else
    {
        mLineEdit = new QLineEdit(this);
        connect(mLineEdit, &QLineEdit::returnPressed, this, [this]() {
            emit signalFilterChanged(mLineEdit->text(), false, false, false);
            });
    }
}

LogFilterBase::sFilterData LogHeaderTextFilter::getFilterData(void) const
{
    LogFilterBase::sFilterData data;
    if (mLineEdit != nullptr)
    {
        data.text         = mLineEdit->text();
        if (mExtended)
        {
            SearchLineEdit* srch = static_cast<SearchLineEdit*>(mLineEdit);
            data.value.srch.isSensitive = srch->isMatchCaseChecked();
            data.value.srch.isWholeWord = srch->isMatchWordChecked();
            data.value.srch.isRegularEx = srch->isWildCardChecked();
        }
        else
        {
            data.value.srch.isSensitive = false;
            data.value.srch.isWholeWord = false;
            data.value.srch.isRegularEx = false;
        }
    }

    return data;
}

void LogHeaderTextFilter::clearFilter(void)
{
    if (mLineEdit != nullptr)
    {
        mLineEdit->setText(QString());
        if (mExtended)
        {
            SearchLineEdit* srch = static_cast<SearchLineEdit*>(mLineEdit);
            srch->setMatchCaseChecked(false);
            srch->setMatchWordChecked(false);
            srch->setWildCardChecked(false);
        }
    }
}

void LogHeaderTextFilter::showFilter(void)
{
    if (mLineEdit != nullptr)
        mLineEdit->show();
}

void LogHeaderTextFilter::hideFilter(void)
{
    clearFilter();
    if (mLineEdit != nullptr)
        mLineEdit->hide();
}

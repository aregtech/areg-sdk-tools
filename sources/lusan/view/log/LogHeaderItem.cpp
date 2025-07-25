﻿/************************************************************************
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
 *  \file        lusan/view/log/LogHeaderItem.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header item.
 *
 ************************************************************************/
#include "lusan/view/log/LogHeaderItem.hpp"
#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/view/common/SearchLineEdit.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

LogComboFilter::LogComboFilter(QWidget* parent)
    : QFrame        (parent)
    , mListWidget   (nullptr)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::Box);
    mListWidget = new QListWidget(this);
    mListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mListWidget->setFocusPolicy(Qt::NoFocus);
    
    connect(mListWidget, &QListWidget::itemChanged, this, [this](QListWidgetItem* /*item*/){
        emit signalFiltersChanged();
    });

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mListWidget);
}

void LogComboFilter::setItems(const QStringList& items)
{
    QList<sComboItem> list;
    for (const auto & entry : items)
    {
        QList<QListWidgetItem*> f = mListWidget->findItems(entry, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        list.push_back(sComboItem{entry, (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked)});
    }
    
    mListWidget->clear();
    
    for (const auto& entry : list)
    {
        QListWidgetItem* item = new QListWidgetItem(entry.text, mListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry.checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        mListWidget->addItem(item);
    }
}

QStringList LogComboFilter::getCheckedItems() const
{
    QStringList checked;
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        QListWidgetItem* item = mListWidget->item(i);
        if (item->checkState() == Qt::Checked)
            checked << item->text();
    }

    return checked;
}

void LogComboFilter::clearFilter(void)
{
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        QListWidgetItem* item = mListWidget->item(i);
        item->setCheckState(Qt::CheckState::Unchecked);
    }
}

//////////////////////////////////////////////////////////////////////////
// LogTextFilter class implementation.
//////////////////////////////////////////////////////////////////////////

LogTextFilter::LogTextFilter(bool isExtended, QWidget* parent)
    : QFrame(parent)
    , mLineEdit(nullptr)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::Box);
    if (isExtended)
    {
        mLineEdit = new SearchLineEdit(QList<SearchLineEdit::eToolButton>{SearchLineEdit::eToolButton::ToolButtonMatchCase, SearchLineEdit::eToolButton::ToolButtonMatchWord, SearchLineEdit::eToolButton::ToolButtonWildCard}, QSize(20, 20), this);
        connect(static_cast<SearchLineEdit *>(mLineEdit), &SearchLineEdit::signalFilterText, [this](const QString & text, bool isCaseSensitive, bool isWholeWord, bool isWildCard){
            emit signalFilterTextChanged(text, isCaseSensitive, isWholeWord, isWildCard);
        });
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchMatchCaseClicked, this, &LogTextFilter::slotToolbuttonChecked);
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchMatchWordClicked, this, &LogTextFilter::slotToolbuttonChecked);
        connect(static_cast<SearchLineEdit*>(mLineEdit), &SearchLineEdit::signalButtonSearchWildCardClicked , this, &LogTextFilter::slotToolbuttonChecked);
    }
    else
    {
        mLineEdit = new QLineEdit(this);
        connect(mLineEdit, &QLineEdit::textChanged, this, [this](const QString & text){
            emit signalFilterTextChanged(text, false, false, false);
        });
    }
    
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mLineEdit);
    
}

QString LogTextFilter::getText() const
{
    return mLineEdit->text();
}

void LogTextFilter::setText(const QString & newText)
{
    mLineEdit->setText(newText);
}

void LogTextFilter::slotToolbuttonChecked(bool checked)
{
    SearchLineEdit* srch = static_cast<SearchLineEdit*>(mLineEdit);
    QString textFilter{ srch != nullptr ? srch->text() : QString() };
    if (textFilter.isEmpty())
        return;

    emit signalFilterTextChanged(textFilter, srch->isMatchCaseChecked(), srch->isMatchWordChecked(), srch->isWildCardChecked());
}

void LogTextFilter::clearFilter(void)
{
    mLineEdit->setText(QString());
}

/////////////////////////////////////////////////////////////
// LogHeaderItem class implementation
/////////////////////////////////////////////////////////////

LogHeaderItem::LogHeaderItem(LogTableHeader& header, int index)
    : QObject(&header)
    , mColumn(static_cast<LiveLogsModel::eColumn>(index))
    , mType (None)
    , mHeader(header)
    , mCombo(nullptr)
    , mEdit (nullptr)
{
    switch (mColumn)
    {
    case LiveLogsModel::eColumn::LogColumnPriority:
    case LiveLogsModel::eColumn::LogColumnSource:
    case LiveLogsModel::eColumn::LogColumnSourceId:
    case LiveLogsModel::eColumn::LogColumnThread:
    case LiveLogsModel::eColumn::LogColumnThreadId:
        mType = eType::Combo;
        mCombo = new LogComboFilter(&mHeader);
        connect(mCombo, &LogComboFilter::signalFiltersChanged, &mHeader, [this]() {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), mCombo->getCheckedItems());
            });
        break;

    case LiveLogsModel::eColumn::LogColumnScopeId:
        mType = eType::Text;
        mEdit = new LogTextFilter(false, &mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, &mHeader, [this](const QString& newText, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), newText, isCaseSensitive, isWholeWord, isWildCard);
        });
        break;
        
    case LiveLogsModel::eColumn::LogColumnMessage:
        mType = eType::Text;
        mEdit = new LogTextFilter(true, &mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, &mHeader, [this](const QString& newText, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), newText, isCaseSensitive, isWholeWord, isWildCard);
            });
        break;

    case LiveLogsModel::eColumn::LogColumnTimestamp:
    default:
        break;
    }
}

inline int LogHeaderItem::fromColumnToIndex(void) const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LiveLogsModel::eColumn LogHeaderItem::fromIndexToColumn(int logicalIndex) const
{
    return mHeader.mModel->fromIndexToColumn(logicalIndex);
}

void LogHeaderItem::showFilters(void)
{
    if (mType == eType::None)
        return;

    int index   = fromColumnToIndex();
    int pos     = mHeader.sectionViewportPosition(index);
    int height  = mHeader.size().height();
    QPoint pt   = mHeader.mapToGlobal(QPoint(pos, height));

    if (mType == eType::Combo)
    {
        Q_ASSERT(mCombo != nullptr);
        Q_ASSERT(mEdit == nullptr);
        mCombo->move(pt);
        mCombo->show();
    }
    else if (mType == eType::Text)
    {
        Q_ASSERT(mCombo == nullptr);
        Q_ASSERT(mEdit != nullptr);
        QSize sz = mEdit->size();
        sz.setWidth(mHeader.sectionSize(index));
        mEdit->setMinimumSize(sz);
        mEdit->move(pt);
        mEdit->show();
    }
    else
    {
        Q_ASSERT(mCombo == nullptr);
        Q_ASSERT(mEdit == nullptr);
    }
}

void LogHeaderItem::setFilterData(const QString& data)
{
    if (mType == eType::Text && mEdit != nullptr)
    {
        mEdit->setText(data);
    }
}

void LogHeaderItem::setFilterData(const std::vector<String>& data)
{
    if (mType == eType::Combo && mCombo != nullptr)
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const auto& entry : data)
        {
            items << QString::fromStdString(entry.getData());
        }

        mCombo->setItems(items);
    }
}

void LogHeaderItem::setFilterData(const std::vector<ITEM_ID>& data)
{
    if (mType == eType::Combo && mCombo != nullptr)
    {
        QStringList items;
        for (const auto& entry : data)
        {
            items << QString::number(static_cast<uint64_t>(entry));
        }

        mCombo->setItems(items);
    }
}

void LogHeaderItem::resetFilter(void)
{
    if (mType == eType::Text && mEdit != nullptr)
        mEdit->clearFilter();
    else if (mType == eType::Combo && mCombo != nullptr)
        mCombo->clearFilter();
}

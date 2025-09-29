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

void LogComboFilter::setItems(const QStringList& items, const NELusanCommon::AnyList& data)
{
    mComboData.clear();
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString &entry = items[i];
        QList<QListWidgetItem*> f = mListWidget->findItems(entry, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        mComboData.push_back(NELusanCommon::FilterData{entry, data[i], (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked)});
    }
    
    mListWidget->clear();
    
    for (const auto& entry : mComboData)
    {
        QListWidgetItem* item = new QListWidgetItem(entry.text, mListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry.active ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        mListWidget->addItem(item);
    }
}

void LogComboFilter::setItems(const QList<NELusanCommon::FilterData>& items)
{
    mComboData.clear();
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const QString &entry = items[i].text;
        QList<QListWidgetItem*> f = mListWidget->findItems(entry, Qt::MatchFlag::MatchExactly);
        Q_ASSERT(f.size() <= 1);
        mComboData.push_back(NELusanCommon::FilterData{entry, items[i].data, (f.size() == 1) && (f[0]->checkState() == Qt::CheckState::Checked)});
    }
    
    mListWidget->clear();
    
    for (const auto& entry : mComboData)
    {
        QListWidgetItem* item = new QListWidgetItem(entry.text, mListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(entry.active ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        mListWidget->addItem(item);
    }
}

QList<NELusanCommon::FilterData> LogComboFilter::getCheckedItems() const
{
    QList<NELusanCommon::FilterData> checked;
    for (int i = 0; i < mListWidget->count(); ++i)
    {
        QListWidgetItem* item = mListWidget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            checked.push_back(NELusanCommon::FilterData{item->text(), mComboData[i].data, true});
        }
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

void LogComboFilter::showFilter(void)
{
    mListWidget->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
    mListWidget->activateWindow();
    show();
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
        connect(static_cast<SearchLineEdit *>(mLineEdit), &SearchLineEdit::signalFilterText, this, [this](const QString & text, bool isCaseSensitive, bool isWholeWord, bool isWildCard){
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
    if (srch != nullptr)
    {
        QString textFilter{ srch->text()};
        if (textFilter.isEmpty() == false)
        {
            emit signalFilterTextChanged(textFilter, srch->isMatchCaseChecked(), srch->isMatchWordChecked(), srch->isWildCardChecked());
        }
    }
}

void LogTextFilter::clearFilter(void)
{
    mLineEdit->setText(QString());
}

void LogTextFilter::showFilter(void)
{
    if (mLineEdit == nullptr)
        return;

    if (mLineEdit->text().isEmpty() == false)
        mLineEdit->selectAll();
    mLineEdit->activateWindow();
    mLineEdit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
    show();
}

/////////////////////////////////////////////////////////////
// LogHeaderItem class implementation
/////////////////////////////////////////////////////////////

LogHeaderItem::LogHeaderItem(LogTableHeader& header, int index)
    : QObject(&header)
    , mColumn(static_cast<LoggingModelBase::eColumn>(index))
    , mType (None)
    , mHeader(header)
    , mCombo(nullptr)
    , mEdit (nullptr)
{
    switch (mColumn)
    {
    case LoggingModelBase::eColumn::LogColumnPriority:
    case LoggingModelBase::eColumn::LogColumnSource:
    case LoggingModelBase::eColumn::LogColumnSourceId:
    case LoggingModelBase::eColumn::LogColumnThread:
    case LoggingModelBase::eColumn::LogColumnThreadId:
        mType = eType::Combo;
        mCombo = new LogComboFilter(&mHeader);
        connect(mCombo, &LogComboFilter::signalFiltersChanged, &mHeader, [this]() {
            emit mHeader.signalComboFilterChanged(fromColumnToIndex(), mCombo->getCheckedItems());
            });
        break;

    case LoggingModelBase::eColumn::LogColumnScopeId:
        mType = eType::Text;
        mEdit = new LogTextFilter(false, &mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, &mHeader, [this](const QString& newText, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), newText, isCaseSensitive, isWholeWord, isWildCard);
        });
        break;
        
    case LoggingModelBase::eColumn::LogColumnMessage:
        mType = eType::Text;
        mEdit = new LogTextFilter(true, &mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, &mHeader, [this](const QString& newText, bool isCaseSensitive, bool isWholeWord, bool isWildCard) {
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), newText, isCaseSensitive, isWholeWord, isWildCard);
            });
        break;
        
    case LoggingModelBase::eColumn::LogColumnTimeDuration:
        mType = eType::Text;
        mEdit = new LogTextFilter(false, &mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, &mHeader, [this](const QString& newText, bool /* isCaseSensitive */, bool /* isWholeWord */, bool /* isWildCard */) {
            emit mHeader.signalTextFilterChanged(fromColumnToIndex(), newText, false, false, false);
        });
        break;
        
    case LoggingModelBase::eColumn::LogColumnTimestamp:
    case LoggingModelBase::eColumn::LogColumnTimeReceived:
    default:
        break;
    }
}

inline int LogHeaderItem::fromColumnToIndex(void) const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LoggingModelBase::eColumn LogHeaderItem::fromIndexToColumn(int logicalIndex) const
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
        mCombo->showFilter();
        // mCombo->activateWindow();
        // mCombo->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
        // mCombo->show();
    }
    else if (mType == eType::Text)
    {
        Q_ASSERT(mCombo == nullptr);
        Q_ASSERT(mEdit != nullptr);
        QSize sz = mEdit->size();
        sz.setWidth(mHeader.sectionSize(index));
        mEdit->setMinimumSize(sz);
        mEdit->move(pt);
        mEdit->showFilter();
        // mEdit->show();
        // mEdit->activateWindow();
        // mEdit->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
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

void LogHeaderItem::setFilterData(const std::vector<String>& data, const NELusanCommon::AnyList& list)
{
    if (mType == eType::Combo && mCombo != nullptr)
    {
        QStringList items;
        items.reserve(static_cast<int>(data.size()));
        for (const auto& entry : data)
        {
            items << QString::fromStdString(entry.getData());
        }

        mCombo->setItems(items, list);
    }
}

void LogHeaderItem::setFilterData(const std::vector<ITEM_ID>& data, const NELusanCommon::AnyList& list)
{
    if (mType == eType::Combo && mCombo != nullptr)
    {
        QStringList items;
        for (const auto& entry : data)
        {
            items << QString::number(static_cast<uint64_t>(entry));
        }

        mCombo->setItems(items, list);
    }
}

void LogHeaderItem::resetFilter(void)
{
    if (mType == eType::Text && mEdit != nullptr)
        mEdit->clearFilter();
    else if (mType == eType::Combo && mCombo != nullptr)
        mCombo->clearFilter();
}

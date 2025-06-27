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
    : QFrame(parent)
    , mListWidget(nullptr)
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

LogTextFilter::LogTextFilter(QWidget* parent)
    : QFrame(parent)
    , mLineEdit(nullptr)
{
    setWindowFlags(Qt::Popup);
    setFrameShape(QFrame::Box);
    mLineEdit = new QLineEdit(this);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mLineEdit);
    
    connect(mLineEdit, &QLineEdit::textChanged, this, [this](const QString & text){
        emit signalFilterTextChanged(text);
    });
}

QString LogTextFilter::getText() const
{
    return mLineEdit->text();
}

LogHeaderItem::LogHeaderItem(LogTableHeader& header, int logicalIndex, const QString & text)
    : QWidget(&header)
    , mColumn(static_cast<LogViewerModel::eColumn>(logicalIndex))
    , mType (None)
    , mHeader(header)
    , mPush (nullptr)
    , mLabel(new QLabel())
    , mLayout(new QHBoxLayout())
    , mCombo(nullptr)
    , mEdit (nullptr)
{
    switch (mColumn)
    {
    case LogViewerModel::eColumn::LogColumnPriority:
    case LogViewerModel::eColumn::LogColumnSource:
    case LogViewerModel::eColumn::LogColumnSourceId:
    case LogViewerModel::eColumn::LogColumnThread:
    case LogViewerModel::eColumn::LogColumnThreadId:
        mType = eType::Combo;
        mPush = new QPushButton(this);
        mCombo = new LogComboFilter(&mHeader);
        connect(mCombo, &LogComboFilter::signalFiltersChanged, this, [this]() {
                emit signalComboFilterChanged(fromColumnToIndex(), mCombo->getCheckedItems());
            });
        break;

    case LogViewerModel::eColumn::LogColumnScopeId:
    case LogViewerModel::eColumn::LogColumnMessage:
        mType = eType::Text;
        mPush = new QPushButton(this);
        mEdit = new LogTextFilter(&mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, this, [this](const QString& text) {
                emit signalTextFilterChanged(fromColumnToIndex(), text);
            });
        break;

    case LogViewerModel::eColumn::LogColumnTimestamp:
    default:
        break;
    }
    
    if (mPush != nullptr)
    {
        mPush->resize(18, 18);
        mPush->setText("\u25BC"); // down arrow
        mPush->setFlat(true);
        mPush->setCursor(Qt::PointingHandCursor);
        mPush->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        mLayout->addWidget(mPush);
        connect(mPush, &QPushButton::clicked, this, &LogHeaderItem::onButtonClicked);
    }

    mLabel->setText(text);
    mLabel->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    mLayout->addWidget(mLabel);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(2);
    setLayout(mLayout);
    hide();
}

inline int LogHeaderItem::fromColumnToIndex(void) const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LogViewerModel::eColumn LogHeaderItem::fromIndexToColum(int logicalIndex) const
{
    return mHeader.mModel->fromIndexToColum(logicalIndex);
}

void LogHeaderItem::onButtonClicked(void)
{
    Q_ASSERT(mType != eType::None);

    int index = fromColumnToIndex();
    int pos = mHeader.sectionViewportPosition(index);
    int size = mHeader.sectionSize(index);
    QRect rect(pos, 0, size, height());
    QPoint pt = mHeader.mapToGlobal(QPoint(rect.left(), height()));

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

/////////////////////////////////////////////////////////////

LogHeaderObject::LogHeaderObject(LogTableHeader& header, int logicalIndex, const QString& text)
    : QObject(&header)
    , mColumn(static_cast<LogViewerModel::eColumn>(logicalIndex))
    , mType(None)
    , mHeader(header)
    , mCombo(nullptr)
    , mEdit(nullptr)
{
    switch (mColumn)
    {
    case LogViewerModel::eColumn::LogColumnPriority:
    case LogViewerModel::eColumn::LogColumnSource:
    case LogViewerModel::eColumn::LogColumnSourceId:
    case LogViewerModel::eColumn::LogColumnThread:
    case LogViewerModel::eColumn::LogColumnThreadId:
        mType = eType::Combo;
        mCombo = new LogComboFilter(&mHeader);
        connect(mCombo, &LogComboFilter::signalFiltersChanged, this, [this]() {
            emit signalComboFilterChanged(fromColumnToIndex(), mCombo->getCheckedItems());
            });
        break;

    case LogViewerModel::eColumn::LogColumnScopeId:
    case LogViewerModel::eColumn::LogColumnMessage:
        mType = eType::Text;
        mEdit = new LogTextFilter(&mHeader);
        connect(mEdit, &LogTextFilter::signalFilterTextChanged, this, [this](const QString& text) {
            emit signalTextFilterChanged(fromColumnToIndex(), text);
            });
        break;

    case LogViewerModel::eColumn::LogColumnTimestamp:
    default:
        break;
    }
}

inline int LogHeaderObject::fromColumnToIndex(void) const
{
    return mHeader.mModel->fromColumnToIndex(mColumn);
}

inline LogViewerModel::eColumn LogHeaderObject::fromIndexToColum(int logicalIndex) const
{
    return mHeader.mModel->fromIndexToColum(logicalIndex);
}

void LogHeaderObject::showFilters(void)
{
    if (mType == eType::None)
        return;

    int index = fromColumnToIndex();
    int pos = mHeader.sectionViewportPosition(index);
    int size = mHeader.sectionSize(index);
    int height = mHeader.size().height();
    // QRect rect(pos, 0, size, height());
    QPoint pt = mHeader.mapToGlobal(QPoint(pos, height));

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


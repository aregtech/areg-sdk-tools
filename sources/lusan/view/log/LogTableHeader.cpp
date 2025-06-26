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
 *  \file        lusan/view/log/LogTableHeader.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, log view table header.
 *
 ************************************************************************/

#include "lusan/view/log/LogTableHeader.hpp"
#include "lusan/model/log/LogViewerModel.hpp"

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
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

    connect(mListWidget, &QListWidget::itemChanged, this, &LogComboFilter::onItemChanged);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(mListWidget);
}

void LogComboFilter::setItems(const QStringList& items)
{
    mListWidget->clear();
    for (const QString& text : items)
    {
        QListWidgetItem* item = new QListWidgetItem(text, mListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

QStringList LogComboFilter::checkedItems() const
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

void LogComboFilter::onItemChanged(QListWidgetItem*)
{
    emit filtersChanged();
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

    connect(mLineEdit, &QLineEdit::textChanged, this, &LogTextFilter::filterTextChanged);
}

QString LogTextFilter::text() const
{
    return mLineEdit->text();
}

LogTableHeader::LogTableHeader(LogViewer* viewer, QTableView* parent, LogViewerModel* model, Qt::Orientation orientation /*= Qt::Horizontal*/)
    : QHeaderView   (orientation, parent)
    , mModel        (model)
    , mViewer       (viewer)
    , mFilters      ( )
{
    setSectionsMovable(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setHighlightSections(true);

    initializeHeaderTypes();
    updateButtonGeometry();

    connect(this, &QHeaderView::sectionResized, this, &LogTableHeader::updateButtonGeometry);
    connect(this, &QHeaderView::sectionMoved, this, &LogTableHeader::updateButtonGeometry);
}

void LogTableHeader::setFilterType(int column, FilterType type)
{
    if (!mFilters.contains(column))
    {
        if (type != FilterType::None)
        {
            mFilters[column] = sFilterWidget{ new QPushButton(this), type };
            mFilters[column].button->setText("\u25BC"); // down arrow
            mFilters[column].button->setFlat(true);
            mFilters[column].button->setCursor(Qt::PointingHandCursor);

            connect(mFilters[column].button, &QPushButton::clicked, [this, column]() {
                showFilterPopup(column);
            });
        }
    }
    else if ((mFilters[column].type != FilterType::None) && (type == FilterType::None))
    {
        if (mFilters[column].button != nullptr)
        {
            delete mFilters[column].button;
            mFilters[column].button = nullptr;
        }
    }

    mFilters[column].type = type;
}

void LogTableHeader::setComboItems(int column, const QStringList& items)
{
    if (!mFilters.contains(column))
        return;

    if (!mFilters[column].comboPopup)
        mFilters[column].comboPopup = new LogComboFilter(this);

    mFilters[column].comboPopup->setItems(items);

    connect(mFilters[column].comboPopup, &LogComboFilter::filtersChanged, [this, column, items]() {
        emit comboFilterChanged(column, mFilters[column].comboPopup->checkedItems());
        });
}

void LogTableHeader::showFilterPopup(int column)
{
    if (!mFilters.contains(column))
        return;

    QRect rect = sectionRect(column);
    QPoint pos = mapToGlobal(QPoint(rect.left(), height()));

    if (mFilters[column].type == ComboFilter)
    {
        if (!mFilters[column].comboPopup)
            return;
        mFilters[column].comboPopup->move(pos);
        mFilters[column].comboPopup->show();
    }
    else if (mFilters[column].type == TextFilter)
    {
        if (!mFilters[column].textPopup)
        {
            mFilters[column].textPopup = new LogTextFilter(this);
            connect(mFilters[column].textPopup, &LogTextFilter::filterTextChanged,
                this, [this, column](const QString& text) {
                    emit textFilterChanged(column, text);
                });
        }

        mFilters[column].textPopup->move(pos);
        mFilters[column].textPopup->show();
    }
}

void LogTableHeader::resizeEvent(QResizeEvent* event)
{
    QHeaderView::resizeEvent(event);
    updateButtonGeometry();
}

void LogTableHeader::updateButtonGeometry()
{
    for (int col = 0; col < count(); ++col)
    {
        if (mFilters.contains(col))
        {
            QRect r = sectionRect(col);
            QPushButton* btn = mFilters[col].button;
            btn->setGeometry(r.right() - 20, 2, 18, height() - 4);
            btn->show();
        }
    }
}

void LogTableHeader::initializeHeaderTypes(void)
{
    Q_ASSERT(mModel != nullptr);
    int cnt = mModel->columnCount();
    for (int i = 0; i < cnt; ++i)
    {
        int data = mModel->headerData(i, Qt::Orientation::Horizontal, Qt::ItemDataRole::UserRole).toInt();
        switch (static_cast<LogViewerModel::eColumn>(data))
        {
        case LogViewerModel::eColumn::LogColumnPriority:
        case LogViewerModel::eColumn::LogColumnSource:
        case LogViewerModel::eColumn::LogColumnSourceId:
        case LogViewerModel::eColumn::LogColumnThread:
        case LogViewerModel::eColumn::LogColumnThreadId:
            setFilterType(i, FilterType::ComboFilter);
            break;

        case LogViewerModel::eColumn::LogColumnScopeId:
        case LogViewerModel::eColumn::LogColumnMessage:
            setFilterType(i, FilterType::TextFilter);
            break;

        case LogViewerModel::eColumn::LogColumnTimestamp:
        default:
            setFilterType(i, FilterType::None);
            break;
        }
    }
}

QRect LogTableHeader::sectionRect(int logicalIndex) const
{
    int pos = sectionPosition(logicalIndex);
    int size = sectionSize(logicalIndex);
    if (orientation() == Qt::Horizontal)
        return QRect(pos, 0, size, height());
    else
        return QRect(0, pos, width(), size);
}

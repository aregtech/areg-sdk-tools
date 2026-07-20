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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMGuardCatalogView.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog view (spec 8, SM-21-04).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardCatalogView.hpp"

#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/SMGuardCatalog.hpp"
#include "lusan/view/sm/SMGuardCatalogModel.hpp"

#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardCatalogView::SMGuardCatalogView(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mModel    (model)
    , mTransId  (0u)
    , mSearch   (nullptr)
    , mTable    (nullptr)
    , mCatalog  (nullptr)
    , mProxy    (nullptr)
{
    setObjectName(QStringLiteral("smGuardCatalog"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    mSearch = new QLineEdit(this);
    mSearch->setObjectName(QStringLiteral("smGuardCatalogSearch"));
    mSearch->setClearButtonEnabled(true);
    mSearch->setPlaceholderText(tr("filter symbols..."));
    layout->addWidget(mSearch);

    mCatalog = new SMGuardCatalogModel(this);
    mProxy = new QSortFilterProxyModel(this);
    mProxy->setSourceModel(mCatalog);
    mProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxy->setFilterKeyColumn(SMGuardCatalogModel::ColName);
    mProxy->setSortRole(Qt::UserRole);

    mTable = new QTableView(this);
    mTable->setObjectName(QStringLiteral("smGuardCatalogTable"));
    mTable->setModel(mProxy);
    mTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTable->setSortingEnabled(true);
    mTable->setShowGrid(false);
    mTable->setWordWrap(false);
    mTable->setContextMenuPolicy(Qt::CustomContextMenu);
    mTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mTable->verticalHeader()->setVisible(false);
    mTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mTable->verticalHeader()->setDefaultSectionSize(mTable->fontMetrics().height() + 6);

    QHeaderView* header = mTable->horizontalHeader();
    header->setSectionResizeMode(SMGuardCatalogModel::ColHue, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(SMGuardCatalogModel::ColName, QHeaderView::Interactive);
    header->setSectionResizeMode(SMGuardCatalogModel::ColType, QHeaderView::Interactive);
    header->setSectionResizeMode(SMGuardCatalogModel::ColUsed, QHeaderView::ResizeToContents);
    header->setStretchLastSection(false);
    header->setHighlightSections(false);
    layout->addWidget(mTable);

    connect(mSearch, &QLineEdit::textChanged, this, &SMGuardCatalogView::onSearchChanged);
    connect(mTable, &QTableView::doubleClicked, this, &SMGuardCatalogView::onDoubleClicked);
    connect(mTable, &QTableView::customContextMenuRequested, this, &SMGuardCatalogView::onContextMenu);
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

void SMGuardCatalogView::setTransition(uint32_t transitionId)
{
    mTransId = transitionId;
    refresh();
}

void SMGuardCatalogView::refresh(void)
{
    if (mTransId == 0u)
    {
        mCatalog->setSymbols(QList<SMGuardSymbol>());
        return;
    }

    mCatalog->setSymbols(SMGuardCatalog::build(mModel.getData(), mTransId));
    mTable->resizeColumnToContents(SMGuardCatalogModel::ColName);
}

void SMGuardCatalogView::setUseCounts(const QHash<uint32_t, int>& counts)
{
    mCatalog->setUseCounts(counts);
}

void SMGuardCatalogView::focusSearch(void)
{
    mSearch->setFocus();
    mSearch->selectAll();
}

QSize SMGuardCatalogView::minimumSizeHint(void) const
{
    QSize hint = QWidget::minimumSizeHint();
    hint.setWidth(0);
    return hint;
}

//////////////////////////////////////////////////////////////////////////
// Interaction
//////////////////////////////////////////////////////////////////////////

void SMGuardCatalogView::onSearchChanged(const QString& text)
{
    mProxy->setFilterFixedString(text.trimmed());
}

int SMGuardCatalogView::sourceRow(const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return -1;
    }

    return mProxy->mapToSource(index).row();
}

void SMGuardCatalogView::onDoubleClicked(const QModelIndex& index)
{
    const SMGuardSymbol* symbol = mCatalog->symbolAt(sourceRow(index));
    if (symbol != nullptr)
    {
        emit insertRequested(*symbol);
    }
}

void SMGuardCatalogView::onContextMenu(const QPoint& pos)
{
    const QModelIndex index = mTable->indexAt(pos);
    const SMGuardSymbol* symbol = mCatalog->symbolAt(sourceRow(index));
    if ((symbol == nullptr) || (symbol->symbolId == 0u))
    {
        return;
    }

    QMenu menu(this);
    QAction* where = menu.addAction(tr("Where used..."));
    if (menu.exec(mTable->viewport()->mapToGlobal(pos)) == where)
    {
        emit whereUsedRequested(symbol->symbolId);
    }
}

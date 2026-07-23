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
 *  \file        lusan/view/sm/SMGuardCallsOutline.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Calls outline (accordion section 1).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardCallsOutline.hpp"

#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include <QListWidget>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

namespace
{
    // Item data roles carried by each outline row.
    constexpr int RoleType    = Qt::UserRole + 1;   //!< The row kind (only TypeInsert is used now).
    constexpr int RoleSymbol  = Qt::UserRole + 3;   //!< The symbol / method id.
    constexpr int RoleExtra   = Qt::UserRole + 4;   //!< The insertable-method index.

    constexpr int TypeInsert  = 4;  //!< An "available condition method" row (double-click inserts).
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardCallsOutline::SMGuardCallsOutline(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mTransId          (0u)
    , mList             (nullptr)
    , mRebuildPending   (false)
{
    setObjectName(QStringLiteral("smGuardCallsOutline"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mList = new QListWidget(this);
    mList->setObjectName(QStringLiteral("smGuardCallsList"));
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mList->setTextElideMode(Qt::ElideRight);
    mList->setContextMenuPolicy(Qt::CustomContextMenu);
    mList->setUniformItemSizes(false);
    layout->addWidget(mList);

    // Only itemActivated (a double-click on desktop styles, or Return): it fires ONCE per gesture.
    // Wiring itemDoubleClicked as well double-fired the handler on one double-click, which inserted
    // the reference twice and nested it (`@cond:x(@cond:x())`), because the first insert leaves the
    // caret between the parens where the second insert then lands.
    connect(mList, &QListWidget::itemActivated, this, &SMGuardCallsOutline::onItemActivated);
    connect(mList, &QListWidget::customContextMenuRequested, this, &SMGuardCallsOutline::onContextMenu);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMGuardCallsOutline::onElementAdded);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMGuardCallsOutline::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMGuardCallsOutline::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMGuardCallsOutline::onDocumentReloaded);
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

void SMGuardCallsOutline::setTransition(uint32_t transitionId)
{
    mTransId = transitionId;
    rebuild();
}

void SMGuardCallsOutline::refresh(void)
{
    rebuild();
}

int SMGuardCallsOutline::insertRowCount(void) const
{
    int count = 0;
    for (int i = 0; i < mList->count(); ++i)
    {
        if (mList->item(i)->data(RoleType).toInt() == TypeInsert)
        {
            ++count;
        }
    }

    return count;
}

QSize SMGuardCallsOutline::minimumSizeHint(void) const
{
    // Width 0 so the longest signature can never widen the Properties dock.
    QSize hint = QWidget::minimumSizeHint();
    hint.setWidth(0);
    return hint;
}

//////////////////////////////////////////////////////////////////////////
// Notifier slots
//////////////////////////////////////////////////////////////////////////

namespace
{
    //!< True when a document change can alter the set of insertable condition methods. The pickup
    //!< list is a projection of the METHOD declarations only, so a guard edit (a Transition /
    //!< Condition change) must not repopulate it -- rebuilding on every keystroke-commit rebuilt the
    //!< whole catalog and the whole list widget for nothing.
    bool affectsPickupList(eDocElementKind kind)
    {
        return (kind == eDocElementKind::Method);
    }
}

void SMGuardCallsOutline::onElementAdded(uint32_t /*id*/, eDocElementKind kind)
{
    if (affectsPickupList(kind))
    {
        scheduleRebuild();
    }
}

void SMGuardCallsOutline::onElementChanged(uint32_t /*id*/, eDocElementKind kind)
{
    if (affectsPickupList(kind))
    {
        scheduleRebuild();
    }
}

void SMGuardCallsOutline::onElementRemoved(uint32_t /*id*/, eDocElementKind kind)
{
    if (affectsPickupList(kind))
    {
        scheduleRebuild();
    }
}

void SMGuardCallsOutline::onDocumentReloaded(void)
{
    scheduleRebuild();
}

//////////////////////////////////////////////////////////////////////////
// Rebuild
//////////////////////////////////////////////////////////////////////////

void SMGuardCallsOutline::scheduleRebuild(void)
{
    if (mRebuildPending)
    {
        return;
    }

    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        rebuild();
    });
}

void SMGuardCallsOutline::rebuild(void)
{
    mList->clear();
    mInsertable.clear();

    // Pickup list only: the Conditions section is a simple palette of
    // the defined condition methods; double-clicking one inserts its `@cond:name()` reference into
    // the guard. It no longer projects the calls already used in the guard -- the guard field shows
    // those (as chips) and the Arguments section maps the call the caret sits in.
    appendInsertableConditions();

    if (mList->count() == 0)
    {
        QListWidgetItem* empty = new QListWidgetItem(tr("no condition methods defined yet"), mList);
        empty->setFlags(Qt::NoItemFlags);
    }
}

void SMGuardCallsOutline::appendInsertableConditions(void)
{
    if (mTransId == 0u)
    {
        return;
    }

    const QList<SMGuardSymbol> catalog = SMGuardCatalog::build(mModel.getData(), mTransId);
    for (const SMGuardSymbol& symbol : catalog)
    {
        if (symbol.refkind == SMGuardSymbol::eRefKind::Cond)
        {
            mInsertable.append(symbol);
        }
    }

    for (int i = 0; i < mInsertable.size(); ++i)
    {
        const SMGuardSymbol& symbol = mInsertable.at(i);
        QListWidgetItem* item = new QListWidgetItem(symbol.display(), mList);
        item->setData(RoleType, TypeInsert);
        item->setData(RoleSymbol, symbol.symbolId);
        item->setData(RoleExtra, i);
        item->setToolTip(tr("Double-click to insert %1 into the guard").arg(symbol.mention()));
    }
}

//////////////////////////////////////////////////////////////////////////
// Interaction
//////////////////////////////////////////////////////////////////////////

void SMGuardCallsOutline::onItemActivated(QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }

    if (item->data(RoleType).toInt() == TypeInsert)
    {
        const int index = item->data(RoleExtra).toInt();
        if ((index >= 0) && (index < mInsertable.size()))
        {
            emit insertRequested(mInsertable.at(index));
        }
    }
}

void SMGuardCallsOutline::onContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = mList->itemAt(pos);
    if (item == nullptr)
    {
        return;
    }

    const uint32_t symbolId = item->data(RoleSymbol).toUInt();
    if ((item->data(RoleType).toInt() == TypeInsert) && (symbolId != 0u))
    {
        QMenu menu(this);
        QAction* where = menu.addAction(tr("Where used..."));
        if (menu.exec(mList->viewport()->mapToGlobal(pos)) == where)
        {
            emit whereUsedRequested(symbolId);
        }
    }
}

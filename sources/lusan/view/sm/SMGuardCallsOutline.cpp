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
 *  \brief       Lusan application, FSM guard Calls outline (accordion section 1, spec 10 / 8.1).
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
    using eKind = SMGuardNode::eKind;

    // Item data roles carried by each outline row.
    constexpr int RoleType    = Qt::UserRole + 1;   //!< 0 call, 1 reference, 2 island, 3 raw, 4 insert.
    constexpr int RolePath    = Qt::UserRole + 2;   //!< QVariantList of child indices.
    constexpr int RoleSymbol  = Qt::UserRole + 3;   //!< The symbol / method id.
    constexpr int RoleExtra   = Qt::UserRole + 4;   //!< Island index (islands) / unmapped (calls) / insert index.

    constexpr int TypeCall    = 0;
    constexpr int TypeRef     = 1;
    constexpr int TypeIsland  = 2;
    constexpr int TypeRaw     = 3;
    constexpr int TypeInsert  = 4;  //!< An "available condition method" row (double-click inserts).

    //!< Packs a child-index path into a QVariant list.
    QVariant packPath(const QList<int>& path)
    {
        QVariantList list;
        for (int index : path)
        {
            list.append(index);
        }

        return list;
    }

    //!< Unpacks a child-index path from a QVariant list.
    QList<int> unpackPath(const QVariant& value)
    {
        QList<int> path;
        const QVariantList list = value.toList();
        for (const QVariant& entry : list)
        {
            path.append(entry.toInt());
        }

        return path;
    }
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
    , mEmitting         (false)
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

    connect(mList, &QListWidget::itemSelectionChanged, this, &SMGuardCallsOutline::onCurrentRowChanged);
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

QList<int> SMGuardCallsOutline::selectedCallPath(void) const
{
    const QListWidgetItem* item = mList->currentItem();
    if ((item != nullptr) && (item->data(RoleType).toInt() == TypeCall))
    {
        return unpackPath(item->data(RolePath));
    }

    return QList<int>();
}

void SMGuardCallsOutline::selectCallPath(const QList<int>& callPath)
{
    for (int i = 0; i < mList->count(); ++i)
    {
        QListWidgetItem* item = mList->item(i);
        if ((item->data(RoleType).toInt() == TypeCall) && (unpackPath(item->data(RolePath)) == callPath))
        {
            mList->setCurrentItem(item);
            return;
        }
    }
}

int SMGuardCallsOutline::callRowCount(void) const
{
    int count = 0;
    for (int i = 0; i < mList->count(); ++i)
    {
        if (mList->item(i)->data(RoleType).toInt() == TypeCall)
        {
            ++count;
        }
    }

    return count;
}

QSize SMGuardCallsOutline::minimumSizeHint(void) const
{
    // Width 0 so the longest signature can never widen the Properties dock (hazard 12.4).
    QSize hint = QWidget::minimumSizeHint();
    hint.setWidth(0);
    return hint;
}

//////////////////////////////////////////////////////////////////////////
// Notifier slots
//////////////////////////////////////////////////////////////////////////

void SMGuardCallsOutline::onElementAdded(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMGuardCallsOutline::onElementChanged(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMGuardCallsOutline::onElementRemoved(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
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

const SMGuardNode* SMGuardCallsOutline::guardTree(void) const
{
    const SMTransitionEntry* transition = (mTransId != 0u) ? mModel.getData().findTransitionById(mTransId) : nullptr;
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return nullptr;
    }

    return transition->getGuard().getTree();
}

void SMGuardCallsOutline::rebuild(void)
{
    // Remember the selected call so re-projection keeps the developer on the same call (12.1).
    const QList<int> keepPath = selectedCallPath();

    mList->clear();
    mInsertable.clear();

    const SMGuardNode* tree = guardTree();
    if (tree != nullptr)
    {
        int islandCounter = 0;
        walk(*tree, QList<int>(), false, islandCounter);
    }

    // The defined condition methods, listed as double-click-to-insert rows below whatever is
    // already in the guard. This is what makes the section useful when the guard is still empty:
    // the developer sees their condition methods and picks one instead of recalling its name.
    appendInsertableConditions();

    if (mList->count() == 0)
    {
        QListWidgetItem* empty = new QListWidgetItem(tr("no condition methods defined yet"), mList);
        empty->setFlags(Qt::NoItemFlags);
    }

    if (keepPath.isEmpty() == false)
    {
        selectCallPath(keepPath);
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

    if (mInsertable.isEmpty())
    {
        return;
    }

    // A non-selectable header separates the "in the guard" rows above from the insertable methods.
    QListWidgetItem* header = new QListWidgetItem(tr("Insert a condition:"), mList);
    header->setFlags(Qt::NoItemFlags);

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

void SMGuardCallsOutline::walk(const SMGuardNode& node, const QList<int>& path, bool parentIsCall, int& islandCounter)
{
    const StateMachineData& data = mModel.getData();

    switch (node.getKind())
    {
    case eKind::Call:
        {
            const SMMethodEntry* method = SMGuardSymbols::method(data, node.getSymbolId());
            const QString name = (method != nullptr) ? method->getName() : tr("(unknown)");
            QString sig;
            if (method != nullptr)
            {
                QStringList types;
                for (const MethodParameter& formal : method->getElements())
                {
                    types.append(formal.getType());
                }

                sig = name + QLatin1Char('(') + types.join(QStringLiteral(", ")) + QLatin1Char(')');
                const QString ret = method->getReturn();
                if (ret.isEmpty() == false)
                {
                    sig += QStringLiteral(" : ") + ret;
                }
            }
            else
            {
                sig = name + QStringLiteral("()");
            }

            const int unmapped = unmappedCount(node);
            if (unmapped > 0)
            {
                sig += QStringLiteral("   [%1 unmapped]").arg(unmapped);
            }

            QListWidgetItem* item = new QListWidgetItem(sig, mList);
            item->setData(RoleType, TypeCall);
            item->setData(RolePath, packPath(path));
            item->setData(RoleSymbol, node.getSymbolId());
            item->setData(RoleExtra, unmapped);
            item->setToolTip(sig);
        }
        break;

    case eKind::Attr:
    case eKind::Const:
    case eKind::Param:
        if (parentIsCall == false)
        {
            // A top-level reference operand shows as a read-only row (design 8.1: `enabled -- attribute (read)`).
            QString name;
            QString kindWord;
            if (node.getKind() == eKind::Attr)
            {
                name = SMGuardSymbols::attributeName(data, node.getSymbolId());
                kindWord = tr("attribute");
            }
            else if (node.getKind() == eKind::Const)
            {
                name = SMGuardSymbols::constantName(data, node.getSymbolId());
                kindWord = tr("constant");
            }
            else
            {
                name = SMGuardSymbols::paramName(data, mTransId, node.getSymbolId());
                kindWord = tr("parameter");
            }

            const QString text = QStringLiteral("%1  --  %2 (read)").arg(name, kindWord);
            QListWidgetItem* item = new QListWidgetItem(text, mList);
            item->setData(RoleType, TypeRef);
            item->setData(RolePath, packPath(path));
            item->setData(RoleSymbol, node.getSymbolId());
            item->setToolTip(text);
        }
        break;

    case eKind::Lambda:
        {
            const int islandIndex = islandCounter;
            const QString text = tr("raw C++  --  lambda [%1]").arg(islandIndex + 1);
            QListWidgetItem* item = new QListWidgetItem(text, mList);
            item->setData(RoleType, TypeIsland);
            item->setData(RolePath, packPath(path));
            item->setData(RoleExtra, islandIndex);
            item->setToolTip(node.getText());
        }
        ++islandCounter;
        break;

    case eKind::Raw:
        {
            QListWidgetItem* item = new QListWidgetItem(tr("raw C++  --  %1").arg(node.getText()), mList);
            item->setData(RoleType, TypeRaw);
            item->setData(RolePath, packPath(path));
            item->setToolTip(node.getText());
        }
        break;

    default:
        break;
    }

    // Lambda bodies carry no child guard nodes to list; a Call's arg children are shown as
    // ghost/orphan rows in the Arguments table, not here (so they are not double-listed).
    const bool descendIntoCall = (node.getKind() == eKind::Call);
    const QList<SMGuardNode*>& kids = node.getChildren();
    for (int i = 0; i < kids.size(); ++i)
    {
        QList<int> childPath = path;
        childPath.append(i);
        walk(*kids.at(i), childPath, descendIntoCall, islandCounter);
    }
}

int SMGuardCallsOutline::unmappedCount(const SMGuardNode& call) const
{
    const SMMethodEntry* method = SMGuardSymbols::method(mModel.getData(), call.getSymbolId());
    if (method == nullptr)
    {
        return 0;
    }

    const QList<MethodParameter>& formals = method->getElements();
    int unmapped = 0;
    for (int i = 0; i < formals.size(); ++i)
    {
        const uint32_t formalId = formals.at(i).getId();
        bool mapped = false;
        for (int c = 0; c < call.getCount(); ++c)
        {
            const SMGuardNode* arg = call.childAt(c);
            if ((formalId != 0u) && (arg->getArgFormalId() == formalId))
            {
                mapped = true;
                break;
            }
        }

        if (mapped == false)
        {
            // Legacy positional fallback: an id-less arg child at this slot counts as mapped.
            const SMGuardNode* positional = call.childAt(i);
            if ((positional != nullptr) && (positional->getArgFormalId() == 0u))
            {
                mapped = true;
            }
        }

        if (mapped == false)
        {
            ++unmapped;
        }
    }

    return unmapped;
}

//////////////////////////////////////////////////////////////////////////
// Interaction
//////////////////////////////////////////////////////////////////////////

void SMGuardCallsOutline::onCurrentRowChanged(void)
{
    QListWidgetItem* item = mList->currentItem();
    if ((item == nullptr) || (item->data(RoleType).toInt() != TypeCall) || mEmitting)
    {
        return;
    }

    mEmitting = true;
    emit callSelected(unpackPath(item->data(RolePath)), item->data(RoleSymbol).toUInt(), item->data(RoleExtra).toInt());
    mEmitting = false;
}

void SMGuardCallsOutline::onItemActivated(QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }

    const int type = item->data(RoleType).toInt();
    if (type == TypeIsland)
    {
        emit islandActivated(item->data(RoleExtra).toInt());
    }
    else if (type == TypeInsert)
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

    const int type = item->data(RoleType).toInt();
    const uint32_t symbolId = item->data(RoleSymbol).toUInt();
    if (((type == TypeCall) || (type == TypeRef)) && (symbolId != 0u))
    {
        QMenu menu(this);
        QAction* where = menu.addAction(tr("Where used..."));
        if (menu.exec(mList->viewport()->mapToGlobal(pos)) == where)
        {
            emit whereUsedRequested(symbolId);
        }
    }
}

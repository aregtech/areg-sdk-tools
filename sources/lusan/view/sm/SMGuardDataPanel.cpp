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
 *  \file        lusan/view/sm/SMGuardDataPanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog (accordion section).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardDataPanel.hpp"

#include "lusan/data/sm/SMGuardTree.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QFont>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>

namespace
{
    constexpr int RoleSymbol { Qt::UserRole + 1 };  //!< The index into the projected symbol list.

    //!< The group heading a symbol belongs under, in display order.
    int groupOf(SMGuardSymbol::eRefKind kind)
    {
        switch (kind)
        {
        case SMGuardSymbol::eRefKind::Param:    return 0;
        case SMGuardSymbol::eRefKind::Attr:     return 1;
        case SMGuardSymbol::eRefKind::Const:    return 2;
        case SMGuardSymbol::eRefKind::Cond:
        default:                                return 3;
        }
    }

    QString groupTitle(int group)
    {
        switch (group)
        {
        case 0:     return QObject::tr("Stimulus parameters");
        case 1:     return QObject::tr("Attributes");
        case 2:     return QObject::tr("Constants");
        default:    return QObject::tr("Conditions");
        }
    }

    //!< Counts, over the whole sub-tree, the BOUND references to \p symbolId (never raw text).
    void countRefs(const SMGuardNode* node, uint32_t symbolId, int& count)
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->getSymbolId() == symbolId)
        {
            const SMGuardNode::eKind kind = node->getKind();
            if ((kind == SMGuardNode::eKind::Call) || node->isReference())
            {
                ++count;
            }
        }

        for (const SMGuardNode* child : node->getChildren())
        {
            countRefs(child, symbolId, count);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardDataPanel::SMGuardDataPanel(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mTransId          (0u)
    , mSearch           (nullptr)
    , mList             (nullptr)
    , mSymbols          ( )
    , mRebuildPending   (false)
{
    setObjectName(QStringLiteral("smGuardDataPanel"));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 0);
    layout->setSpacing(2);

    mSearch = new QLineEdit(this);
    mSearch->setObjectName(QStringLiteral("smGuardDataSearch"));
    mSearch->setPlaceholderText(tr("filter by name or type"));
    mSearch->setClearButtonEnabled(true);
    layout->addWidget(mSearch);

    mList = new QListWidget(this);
    mList->setObjectName(QStringLiteral("smGuardDataList"));
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mList->setTextElideMode(Qt::ElideRight);
    mList->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(mList);

    // itemActivated only (one signal per double-click / Return): wiring itemDoubleClicked as well
    // fires the handler twice for one gesture and inserts the reference twice.
    connect(mList, &QListWidget::itemActivated, this, &SMGuardDataPanel::onItemActivated);
    connect(mList, &QListWidget::customContextMenuRequested, this, &SMGuardDataPanel::onContextMenu);
    connect(mSearch, &QLineEdit::textChanged, this, [this](const QString&) { rebuild(); });

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMGuardDataPanel::onElementAdded);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMGuardDataPanel::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMGuardDataPanel::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMGuardDataPanel::onDocumentReloaded);
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

void SMGuardDataPanel::setTransition(uint32_t transitionId)
{
    mTransId = transitionId;
    rebuild();
}

void SMGuardDataPanel::refresh(void)
{
    rebuild();
}

int SMGuardDataPanel::symbolRowCount(void) const
{
    int count = 0;
    for (int i = 0; i < mList->count(); ++i)
    {
        if (mList->item(i)->data(RoleSymbol).isValid())
        {
            ++count;
        }
    }

    return count;
}

void SMGuardDataPanel::setFilter(const QString& text)
{
    mSearch->setText(text);     // textChanged rebuilds
}

void SMGuardDataPanel::focusSearch(void)
{
    mSearch->setFocus();
    mSearch->selectAll();
}

QSize SMGuardDataPanel::minimumSizeHint(void) const
{
    QSize hint = QWidget::minimumSizeHint();
    hint.setWidth(0);
    return hint;
}

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////

void SMGuardDataPanel::onElementAdded(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMGuardDataPanel::onElementChanged(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    // Any change can move the catalog: a rename, a retype, or a guard edit that changes `used N`.
    scheduleRebuild();
}

void SMGuardDataPanel::onElementRemoved(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMGuardDataPanel::onDocumentReloaded(void)
{
    mTransId = 0u;
    scheduleRebuild();
}

void SMGuardDataPanel::onItemActivated(QListWidgetItem* item)
{
    if (item == nullptr)
    {
        return;
    }

    const QVariant index = item->data(RoleSymbol);
    if (index.isValid() && (index.toInt() >= 0) && (index.toInt() < mSymbols.size()))
    {
        emit insertRequested(mSymbols.at(index.toInt()));
    }
}

void SMGuardDataPanel::onContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = mList->itemAt(pos);
    if (item == nullptr)
    {
        return;
    }

    const QVariant index = item->data(RoleSymbol);
    if ((index.isValid() == false) || (index.toInt() < 0) || (index.toInt() >= mSymbols.size()))
    {
        return;
    }

    const SMGuardSymbol& symbol = mSymbols.at(index.toInt());
    QMenu menu(this);
    QAction* insert = menu.addAction(tr("Insert %1").arg(symbol.mention()));
    QAction* used   = menu.addAction(tr("Where used..."));
    QAction* chosen = menu.exec(mList->viewport()->mapToGlobal(pos));
    if (chosen == insert)
    {
        emit insertRequested(symbol);
    }
    else if ((chosen == used) && (symbol.symbolId != 0u))
    {
        emit whereUsedRequested(symbol.symbolId);
    }
}

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////

void SMGuardDataPanel::scheduleRebuild(void)
{
    // Never rebuild inside a notifier slot: the emitting command is still on the stack, and the
    // rows carry the widgets the gesture may still be unwinding through.
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

int SMGuardDataPanel::useCount(uint32_t symbolId) const
{
    if (symbolId == 0u)
    {
        return 0;
    }

    const SMTransitionEntry* transition = (mTransId != 0u) ? mModel.getData().findTransitionById(mTransId) : nullptr;
    if ((transition == nullptr) || (transition->getGuard().isOk() == false))
    {
        return 0;
    }

    int count = 0;
    countRefs(transition->getGuard().getTree(), symbolId, count);
    return count;
}

void SMGuardDataPanel::rebuild(void)
{
    mList->clear();
    mSymbols.clear();
    if (mTransId == 0u)
    {
        return;
    }

    // Recognition over recall: the whole universe, grouped, is what makes this panel worth its
    // vertical space. The filter narrows by name OR type, so "uint16" finds every uint16 symbol.
    const QString filter = mSearch->text().trimmed();
    const QList<SMGuardSymbol> universe = SMGuardCatalog::build(mModel.getData(), mTransId);

    for (int group = 0; group < 4; ++group)
    {
        QList<SMGuardSymbol> members;
        for (const SMGuardSymbol& symbol : universe)
        {
            if (groupOf(symbol.refkind) != group)
            {
                continue;
            }

            if ((filter.isEmpty() == false)
                && (symbol.name.contains(filter, Qt::CaseInsensitive) == false)
                && (symbol.typeText.contains(filter, Qt::CaseInsensitive) == false))
            {
                continue;
            }

            members.append(symbol);
        }

        if (members.isEmpty())
        {
            continue;       // an empty group shows no heading: no dead rows in a short dock
        }

        QListWidgetItem* heading = new QListWidgetItem(groupTitle(group), mList);
        QFont headingFont = heading->font();
        headingFont.setBold(true);
        heading->setFont(headingFont);
        heading->setFlags(Qt::NoItemFlags);         // a label, never selectable or activatable

        for (const SMGuardSymbol& symbol : members)
        {
            const int used = useCount(symbol.symbolId);
            QString label = QStringLiteral("  %1  %2").arg(symbol.glyph, symbol.display());
            if (used > 0)
            {
                label += tr("      used %1").arg(used);
            }

            QListWidgetItem* row = new QListWidgetItem(label, mList);
            row->setForeground(NEGuardStyle::ownerColor(symbol.owner));
            row->setData(RoleSymbol, mSymbols.size());
            row->setToolTip(tr("%1 -- double-click to insert %2")
                            .arg(symbol.kindNoun(), symbol.mention()));
            mSymbols.append(symbol);
        }
    }
}

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
 *  \file        lusan/view/sm/SMOutlinePanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Design page outline panel.
 *
 ************************************************************************/

#include "lusan/view/sm/SMOutlinePanel.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"

#include <QHeaderView>
#include <QMenu>
#include <QSet>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace
{
    //!< Item data roles carrying the element identity and navigation target of each row.
    constexpr int RoleElementId { Qt::UserRole + 0 };
    constexpr int RoleLevelId   { Qt::UserRole + 1 };
    constexpr int RoleKind      { Qt::UserRole + 2 };
    constexpr int RoleComposite { Qt::UserRole + 3 };

    //!< The row kinds, distinguishing selectable canvas elements from group headers.
    constexpr int KindGroup     { 0 };
    constexpr int KindState     { 1 };
    constexpr int KindTransition{ 2 };
    constexpr int KindRegistry  { 3 };

    //!< A short label for a transition: its stimulus and target (or an internal marker).
    QString transitionLabel(const SMTransitionEntry& transition)
    {
        const QString stimulus = transition.getStimulus().isEmpty()
                ? QObject::tr("<stimulus>") : transition.getStimulus();
        return transition.isExternal()
                ? (stimulus + QStringLiteral(" -> ") + transition.getTo())
                : (stimulus + QObject::tr(" (internal)"));
    }
}

SMOutlinePanel::SMOutlinePanel(StateMachineModel& model, SMSceneManager& sceneManager, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mSceneManager (sceneManager)
    , mTree         (new QTreeWidget(this))
    , mRebuildTimer (new QTimer(this))
    , mUpdating     (false)
{
    mTree->setHeaderHidden(true);
    mTree->setColumnCount(1);
    mTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mTree->setContextMenuPolicy(Qt::CustomContextMenu);
    mTree->setExpandsOnDoubleClick(false);
    mTree->header()->setStretchLastSection(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mTree);

    mRebuildTimer->setSingleShot(true);
    mRebuildTimer->setInterval(0);
    connect(mRebuildTimer, &QTimer::timeout, this, &SMOutlinePanel::rebuild);

    connect(mTree, &QTreeWidget::itemSelectionChanged, this, &SMOutlinePanel::onOutlineSelectionChanged);
    connect(mTree, &QTreeWidget::itemDoubleClicked, this, &SMOutlinePanel::onItemDoubleClicked);
    connect(mTree, &QTreeWidget::customContextMenuRequested, this, &SMOutlinePanel::onContextMenuRequested);

    connect(&mModel.getSelectionModel(), &SMSelectionModel::signalSelectionChanged, this, &SMOutlinePanel::onModelSelectionChanged);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMOutlinePanel::scheduleRebuild);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMOutlinePanel::scheduleRebuild);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMOutlinePanel::scheduleRebuild);
    connect(&notifier, &DocModelNotifier::nameChanged, this, &SMOutlinePanel::scheduleRebuild);
    connect(&notifier, &DocModelNotifier::listReordered, this, &SMOutlinePanel::scheduleRebuild);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMOutlinePanel::scheduleRebuild);

    rebuild();
}

SMOutlinePanel::~SMOutlinePanel()
{
    mModel.getNotifier().disconnect(this);
    mModel.getSelectionModel().disconnect(this);
}

void SMOutlinePanel::scheduleRebuild()
{
    mRebuildTimer->start();
}

QTreeWidgetItem* SMOutlinePanel::makeItem(const QString& label, uint32_t elementId, uint32_t levelId, int role, bool composite)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList{ label });
    item->setData(0, RoleElementId, elementId);
    item->setData(0, RoleLevelId, levelId);
    item->setData(0, RoleKind, role);
    item->setData(0, RoleComposite, composite);
    if ((elementId != 0) && ((role == KindState) || (role == KindTransition)))
    {
        mItemById.insert(elementId, item);
    }

    return item;
}

void SMOutlinePanel::addLevelStates(QTreeWidgetItem* parent, const SMStateData& level, uint32_t levelId)
{
    for (SMStateEntry* state : level.getElements())
    {
        if (state == nullptr)
        {
            continue;
        }

        const bool composite = state->hasNestedStates();
        QString label = state->getName();
        if (composite)
        {
            label += QStringLiteral("  [+]");
        }
        else if (state->isImportedSubmachine())
        {
            label += QStringLiteral("  [") + state->getSubmachine() + QStringLiteral("]");
        }

        QTreeWidgetItem* item = makeItem(label, state->getId(), levelId, KindState, composite);
        parent->addChild(item);

        for (SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            if (transition != nullptr)
            {
                item->addChild(makeItem(transitionLabel(*transition), transition->getId(), levelId, KindTransition, false));
            }
        }

        if (composite)
        {
            addLevelStates(item, *state->getNestedStates(), state->getId());
        }
    }
}

void SMOutlinePanel::rebuild()
{
    mRebuildTimer->stop();

    // Preserve which levels were expanded so a rebuild after any edit does not collapse the
    // user's view; group headers re-expand unconditionally.
    QSet<uint32_t> expanded;
    for (QHash<uint32_t, QTreeWidgetItem*>::const_iterator it = mItemById.constBegin(); it != mItemById.constEnd(); ++it)
    {
        if (it.value()->isExpanded())
        {
            expanded.insert(it.key());
        }
    }

    mUpdating = true;
    mTree->clear();
    mItemById.clear();

    StateMachineData& data = mModel.getData();
    const uint32_t rootLevel = data.getOverview().getId();

    QTreeWidgetItem* states = new QTreeWidgetItem(QStringList{ tr("States") });
    states->setData(0, RoleKind, KindGroup);
    mTree->addTopLevelItem(states);
    addLevelStates(states, data.getStates(), rootLevel);
    states->setExpanded(true);

    const auto addGroup = [this](const QString& title) -> QTreeWidgetItem*
    {
        QTreeWidgetItem* group = new QTreeWidgetItem(QStringList{ title });
        group->setData(0, RoleKind, KindGroup);
        mTree->addTopLevelItem(group);
        return group;
    };

    const auto addRegistry = [this](QTreeWidgetItem* group, uint32_t id, const QString& name)
    {
        group->addChild(makeItem(name, id, 0u, KindRegistry, false));
    };

    QTreeWidgetItem* types = addGroup(tr("Data Types"));
    for (const DataTypeCustom* type : data.getDataTypes().getElements())
    {
        if (type != nullptr) addRegistry(types, type->getId(), type->getName());
    }

    QTreeWidgetItem* attributes = addGroup(tr("Attributes"));
    for (const SMAttributeEntry& attribute : data.getAttributes().getElements())
    {
        addRegistry(attributes, attribute.getId(), attribute.getName());
    }

    QTreeWidgetItem* events = addGroup(tr("Events"));
    for (const SMEventEntry* event : data.getEvents().getElements())
    {
        if (event != nullptr) addRegistry(events, event->getId(), event->getName());
    }

    QTreeWidgetItem* timers = addGroup(tr("Timers"));
    for (const SMTimerEntry& timer : data.getTimers().getElements())
    {
        addRegistry(timers, timer.getId(), timer.getName());
    }

    QTreeWidgetItem* methods = addGroup(tr("Methods"));
    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if (method != nullptr)
        {
            const QString label = method->getName() + QStringLiteral(" (")
                    + QString::fromLatin1(SMMethodEntry::toString(method->getMethodType())) + QStringLiteral(")");
            addRegistry(methods, method->getId(), label);
        }
    }

    QTreeWidgetItem* constants = addGroup(tr("Constants"));
    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        addRegistry(constants, constant.getId(), constant.getName());
    }

    QTreeWidgetItem* includes = addGroup(tr("Includes"));
    for (const IncludeEntry& include : data.getIncludes().getElements())
    {
        addRegistry(includes, include.getId(), include.getName());
    }

    QTreeWidgetItem* imports = addGroup(tr("Imports"));
    for (const SMImportEntry& import : data.getImports().getElements())
    {
        addRegistry(imports, import.getId(), import.getName());
    }

    for (QHash<uint32_t, QTreeWidgetItem*>::const_iterator it = mItemById.constBegin(); it != mItemById.constEnd(); ++it)
    {
        if (expanded.contains(it.key()))
        {
            it.value()->setExpanded(true);
        }
    }

    mUpdating = false;
    onModelSelectionChanged();
}

void SMOutlinePanel::onModelSelectionChanged()
{
    if (mUpdating)
    {
        return;
    }

    mUpdating = true;
    mTree->clearSelection();

    QTreeWidgetItem* last = nullptr;
    for (uint32_t id : mModel.getSelectionModel().getSelection())
    {
        QTreeWidgetItem* item = mItemById.value(id, nullptr);
        if (item != nullptr)
        {
            item->setSelected(true);
            last = item;
        }
    }

    if (last != nullptr)
    {
        mTree->setCurrentItem(last);
        mTree->scrollToItem(last);
    }

    mUpdating = false;
}

void SMOutlinePanel::onOutlineSelectionChanged()
{
    if (mUpdating)
    {
        return;
    }

    mUpdating = true;

    const QList<QTreeWidgetItem*> selected = mTree->selectedItems();
    QTreeWidgetItem* current = mTree->currentItem();
    if ((current == nullptr) && (selected.isEmpty() == false))
    {
        current = selected.first();
    }

    QList<uint32_t> ids;
    if (current != nullptr)
    {
        const int kind = current->data(0, RoleKind).toInt();
        const uint32_t levelId = current->data(0, RoleLevelId).toUInt();

        if (((kind == KindState) || (kind == KindTransition)) && (levelId != 0))
        {
            if (levelId != mModel.getSelectionModel().getActiveLevel())
            {
                mSceneManager.navigateTo(levelId);
            }

            // Mirror only the sibling selection that shares the navigated level.
            for (QTreeWidgetItem* item : selected)
            {
                const int itemKind = item->data(0, RoleKind).toInt();
                if (((itemKind == KindState) || (itemKind == KindTransition))
                    && (item->data(0, RoleLevelId).toUInt() == levelId))
                {
                    ids.append(item->data(0, RoleElementId).toUInt());
                }
            }
        }
        else if (kind == KindRegistry)
        {
            ids.append(current->data(0, RoleElementId).toUInt());
        }
    }

    mModel.getSelectionModel().setSelection(ids);
    mUpdating = false;
}

void SMOutlinePanel::onItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
    if (item == nullptr)
    {
        return;
    }

    const int kind = item->data(0, RoleKind).toInt();
    const uint32_t elementId = item->data(0, RoleElementId).toUInt();
    const uint32_t levelId = item->data(0, RoleLevelId).toUInt();

    if ((kind == KindState) && item->data(0, RoleComposite).toBool())
    {
        // A composite: descend into its submachine (its own level is the parent's).
        if (levelId != mModel.getSelectionModel().getActiveLevel())
        {
            mSceneManager.navigateTo(levelId);
        }

        mSceneManager.enterSubmachine(elementId);
    }
    else if (((kind == KindState) || (kind == KindTransition)) && (levelId != 0))
    {
        mSceneManager.navigateTo(levelId);
        mModel.getSelectionModel().setSelection(QList<uint32_t>{ elementId });
    }
}

void SMOutlinePanel::onContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = mTree->itemAt(pos);
    if (item == nullptr)
    {
        return;
    }

    const int kind = item->data(0, RoleKind).toInt();
    if ((kind != KindState) && (kind != KindTransition))
    {
        return;
    }

    // Bring the clicked row into the selection so the delegated actions target it.
    if (item->isSelected() == false)
    {
        mTree->setCurrentItem(item);
    }

    QMenu menu(this);
    if (kind == KindState)
    {
        QAction* rename = menu.addAction(tr("Rename"));
        connect(rename, &QAction::triggered, this, [this, item]() {
            emit signalRenameRequested(item->data(0, RoleElementId).toUInt());
        });
    }

    QAction* remove = menu.addAction(tr("Delete"));
    connect(remove, &QAction::triggered, this, [this]() { emit signalDeleteRequested(); });

    menu.exec(mTree->viewport()->mapToGlobal(pos));
}

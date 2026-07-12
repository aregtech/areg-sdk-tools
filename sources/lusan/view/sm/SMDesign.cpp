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
 *  \file        lusan/view/sm/SMDesign.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor Design page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDesign.hpp"

#include "lusan/data/sm/SMClipboard.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMPasteCommand.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMAutoPlacer.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMNoteItem.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMStateItem.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"

#include <QAction>
#include <QClipboard>
#include <QColorDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPair>
#include <QScrollBar>
#include <QSettings>
#include <QStringList>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace
{
    //!< Synthetic stress-test element IDs start here, far above any document ID.
    constexpr uint32_t StressIdBase{ 0x40000000u };

    /**
     * \brief   A plain synthetic node used only by the stress populate: a rounded,
     *          movable, selectable box exercising selection, snapping, and repaints.
     **/
    class StressNodeItem : public SMCanvasItem
    {
    public:
        StressNodeItem(uint32_t elementId, const QString& name)
            : SMCanvasItem  (elementId)
            , mName         (name)
        {
            setFlag(QGraphicsItem::ItemIsSelectable, true);
            setFlag(QGraphicsItem::ItemIsMovable, true);
            setFlag(QGraphicsItem::ItemIsFocusable, true);
        }

        QRectF boundingRect() const override
        {
            return QRectF(-2.0, -2.0, 124.0, 64.0);
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* widget) override
        {
            const QPalette palette{ (widget != nullptr) ? widget->palette() : QPalette() };
            const QRectF   body{ 0.0, 0.0, 120.0, 60.0 };

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(QPen(palette.color(QPalette::WindowText), 1.0));
            painter->setBrush(palette.color(QPalette::Window));
            painter->drawRoundedRect(body, 6.0, 6.0);
            painter->setPen(palette.color(QPalette::WindowText));
            painter->drawText(body, Qt::AlignCenter, mName);

            if (isSelected())
            {
                NESMDesign::paintSelectionFrame(painter, body.adjusted(-2.0, -2.0, 2.0, 2.0), palette, hasFocus());
            }
        }

    private:
        QString mName;  //!< The displayed node name.
    };

    /**
     * \brief   One movable box (state or note) participating in align/distribute.
     **/
    struct SelectionBox
    {
        bool        isNote; //!< True for a note, false for a state.
        uint32_t    id;     //!< The element ID.
        QRectF      rect;   //!< The current box geometry (scene coordinates).
    };

    //!< Collects the selected state and note boxes eligible for align/distribute.
    QList<SelectionBox> collectSelectionBoxes(SMScene& scene)
    {
        QList<SelectionBox> boxes;
        for (SMStateItem* item : scene.selectedStateItems())
        {
            boxes.append(SelectionBox{ false, item->getElementId(), item->getBoxGeometry() });
        }

        for (SMNoteItem* item : scene.selectedNoteItems())
        {
            boxes.append(SelectionBox{ true, item->getElementId(), item->getBoxGeometry() });
        }

        return boxes;
    }

    //!< Pushes the moved boxes (state/note) as one undo step (single command or composite).
    void pushMoveChanges(  StateMachineModel& model, const QList<QPair<SelectionBox, QRectF>>& changed
                         , const QString& text)
    {
        if (changed.isEmpty())
        {
            return;
        }

        StateMachineData& data    = model.getData();
        DocModelNotifier& notifier= model.getNotifier();
        const uint32_t    gesture = SMMoveNodeCommand::takeNextGesture();
        const bool        single  = (changed.size() == 1);
        QUndoCommand*     parent  = single ? nullptr : new SMCompositeCommand(data, notifier, text);

        for (const QPair<SelectionBox, QRectF>& entry : changed)
        {
            const SelectionBox& box  = entry.first;
            const QRectF&       rect = entry.second;
            QUndoCommand* command = box.isNote
                    ? static_cast<QUndoCommand*>(new SMMoveNoteCommand(  data, notifier, box.id, gesture
                                                                       , rect.x(), rect.y(), rect.width(), rect.height(), text, parent))
                    : static_cast<QUndoCommand*>(new SMMoveNodeCommand(  data, notifier, box.id, gesture
                                                                       , rect.x(), rect.y(), rect.width(), rect.height(), text, parent));
            if (single)
            {
                model.getUndoStack().push(command);
            }
        }

        if (single == false)
        {
            model.getUndoStack().push(parent);
        }
    }
}

SMDesign::SMDesign(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mView         (new SMGraphicsView(this))
    , mSceneManager (nullptr)
    , mScene        (nullptr)
    , mBreadcrumb   (nullptr)
    , mBreadcrumbLayout(nullptr)
    , mActZoomIn    (nullptr)
    , mActZoomOut   (nullptr)
    , mActZoomReset (nullptr)
    , mActZoomFit   (nullptr)
    , mActToggleGrid(nullptr)
    , mActGridDots  (nullptr)
    , mActGridDotSize(nullptr)
    , mActToggleSnap(nullptr)
    , mActGridSize  (nullptr)
    , mActSelectAll (nullptr)
    , mActUndo      (nullptr)
    , mActRedo      (nullptr)
    , mActAddState  (nullptr)
    , mActAddFinal  (nullptr)
    , mActAddTransition(nullptr)
    , mActAddNote   (nullptr)
    , mActDelete    (nullptr)
    , mActRename    (nullptr)
    , mActCut       (nullptr)
    , mActCopy      (nullptr)
    , mActPaste     (nullptr)
    , mActDuplicate (nullptr)
    , mActStateColor(nullptr)
    , mActEdgeColor (nullptr)
    , mActNoteColor (nullptr)
    , mActSetColor  (nullptr)
    , mActAlignLeft (nullptr)
    , mActAlignRight(nullptr)
    , mActAlignTop  (nullptr)
    , mActAlignBottom(nullptr)
    , mActDistributeH(nullptr)
    , mActDistributeV(nullptr)
    , mActAddInternal(nullptr)
    , mActSetStimulus(nullptr)
    , mActRaisePriority(nullptr)
    , mActLowerPriority(nullptr)
    , mActAddSubstate(nullptr)
    , mActEnterSubmachine(nullptr)
    , mActGoToParent(nullptr)
    , mActCenterMachine(nullptr)
    , mActNewTrigger(nullptr)
    , mActNewAction (nullptr)
    , mActNewCondition(nullptr)
    , mActNewEvent  (nullptr)
    , mActNewTimer  (nullptr)
    , mActNewAttribute(nullptr)
    , mActNewConstant(nullptr)
    , mActNewDataType(nullptr)
    , mToolbarVisible(true)
    , mShownLevel   (0u)
    , mViewGesture  (0u)
    , mRestoringView(false)
    , mSyncingGrid  (false)
{
    mSceneManager = new SMSceneManager(model, this);

    mBreadcrumb = new QWidget(this);
    mBreadcrumbLayout = new QHBoxLayout(mBreadcrumb);
    mBreadcrumbLayout->setContentsMargins(8, 4, 8, 4);
    mBreadcrumbLayout->setSpacing(4);

    // The drawing toolbar lives in the navigation dock's FSM Toolbar tab (bound to the active
    // State Machine), not above the canvas; the Design page only owns the actions.
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mBreadcrumb);
    layout->addWidget(mView);

    connect(mSceneManager, &SMSceneManager::signalLevelChanged, this, &SMDesign::onLevelChanged);
    connect(mView->horizontalScrollBar(), &QScrollBar::valueChanged, this, &SMDesign::onViewportChanged);
    connect(mView->verticalScrollBar(), &QScrollBar::valueChanged, this, &SMDesign::onViewportChanged);
    connect(mView, &SMGraphicsView::signalZoomChanged, this, &SMDesign::onViewportChanged);

    rebuildScene();
    setupActions();
    mView->installEventFilter(this);

    // Context-sensitive canvas/state/transition/note menus (spec 9.3 rule 2), built on
    // demand from the same action set the toolbar and Design menu reuse. A QGraphicsView
    // routes context-menu events through contextMenuEvent(), so a Qt::CustomContextMenu
    // policy never fires customContextMenuRequested; the view's own signal (emitted from its
    // contextMenuEvent override, viewport coordinates) is the reliable hook.
    connect(mView, &SMGraphicsView::signalContextMenuRequested, this, &SMDesign::onViewContextMenuRequested);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMDesign::onDocumentReloaded);
    connect(&notifier, &DocModelNotifier::layoutChanged, this, &SMDesign::onModelLayoutChanged);
    connect(&notifier, &DocModelNotifier::nameChanged, this, [this](uint32_t, const QString&, const QString&) {
        rebuildBreadcrumb();
    });
    connect(&notifier, &DocModelNotifier::elementChanged, this, [this](uint32_t, eDocElementKind kind) {
        if (kind == eDocElementKind::Overview)
        {
            rebuildBreadcrumb();
        }
        else if (kind == eDocElementKind::State)
        {
            updateNavActions();
        }
    });
    connect(&mModel.getSelectionModel(), &SMSelectionModel::signalSelectionChanged, this, [this](const QList<uint32_t>&) {
        updateNavActions();
    });

    updateNavActions();
}

bool SMDesign::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == mView)
    {
        if (event->type() == QEvent::Resize)
        {
            // The resize's scrollbar churn is not a viewport edit: suppress persisting
            // and re-pin the stored center once the view has finished the resize.
            mRestoringView = true;
            QMetaObject::invokeMethod(this, [this]() {
                mRestoringView = false;
                // Re-anchor the default (top-left) view when the level has no stored viewport
                // yet: the first onLevelChanged ran before the page was shown at full size, so
                // the initial anchor used a placeholder viewport rect. Once a View entry exists
                // (the user scrolled/zoomed), the resize just re-pins that stored center.
                const bool hasEntry = (mModel.getData().getLayout().findView(mShownLevel) != nullptr);
                restoreViewport(mShownLevel, hasEntry == false);
            }, Qt::QueuedConnection);
        }
        else if (event->type() == QEvent::ShortcutOverride)
        {
            if (matchAction(*static_cast<QKeyEvent*>(event)) != nullptr)
            {
                event->accept();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QAction* action = matchAction(*static_cast<QKeyEvent*>(event));
            if (action != nullptr)
            {
                action->trigger();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

QAction* SMDesign::matchAction(const QKeyEvent& event) const
{
    if (event.key() == 0 || event.key() == Qt::Key_unknown)
    {
        return nullptr;
    }

    QList<QKeySequence> pressed;
    pressed.append(QKeySequence(event.keyCombination()));

    // Shift+digit arrives as the shifted symbol (e.g. ')'); also match the digit itself.
    const quint32 nativeKey = event.nativeVirtualKey();
    if ((nativeKey >= '0') && (nativeKey <= '9'))
    {
        const Qt::Key digit = static_cast<Qt::Key>(Qt::Key_0 + static_cast<int>(nativeKey - '0'));
        pressed.append(QKeySequence(QKeyCombination(event.modifiers(), digit)));
    }

    for (QAction* action : actions())
    {
        for (const QKeySequence& shortcut : action->shortcuts())
        {
            if (pressed.contains(shortcut))
            {
                return action;
            }
        }
    }

    return nullptr;
}

void SMDesign::onDocumentReloaded()
{
    rebuildScene();
}

void SMDesign::setupActions()
{
    mActZoomIn = new QAction(tr("Zoom In"), this);
    mActZoomIn->setShortcuts({ QKeySequence::ZoomIn, QKeySequence(Qt::CTRL | Qt::Key_Equal) });
    connect(mActZoomIn, &QAction::triggered, mView, &SMGraphicsView::zoomIn);

    mActZoomOut = new QAction(tr("Zoom Out"), this);
    mActZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(mActZoomOut, &QAction::triggered, mView, &SMGraphicsView::zoomOut);

    mActZoomReset = new QAction(tr("Zoom 100%"), this);
    mActZoomReset->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(mActZoomReset, &QAction::triggered, mView, &SMGraphicsView::zoomReset);

    mActZoomFit = new QAction(tr("Zoom to Fit"), this);
    mActZoomFit->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_0));
    connect(mActZoomFit, &QAction::triggered, mView, &SMGraphicsView::zoomToFit);

    mActToggleGrid = new QAction(tr("Show Grid"), this);
    mActToggleGrid->setCheckable(true);
    connect(mActToggleGrid, &QAction::toggled, this, [this](bool checked) {
        if (mSyncingGrid)
        {
            return;
        }

        // The scene/action are re-synced from the command's notification (onModelLayoutChanged),
        // so undo/redo of this toggle keeps the canvas and the checked action consistent.
        mModel.getUndoStack().push(new SMSetGridVisibleCommand(  mModel.getData(), mModel.getNotifier()
                                                               , checked, tr("Toggle grid visibility")));
    });

    mActGridSize = new QAction(tr("Grid Size..."), this);
    connect(mActGridSize, &QAction::triggered, this, [this]() {
        const int current = mModel.getData().getLayout().getGridSize();
        bool ok = false;
        const int value = QInputDialog::getInt(  this, tr("Grid Size"), tr("Grid size (px):")
                                               , current, NESMDesign::GridSizeMin, 200, 1, &ok);
        if (ok && (value != current))
        {
            mModel.getUndoStack().push(new SMSetGridSizeCommand(  mModel.getData(), mModel.getNotifier()
                                                                , value, tr("Change grid size")));
        }
    });

    // The grid style is an application-level display preference (the document persists
    // only the grid size and visibility, spec 7.6).
    mActGridDots = new QAction(tr("Dotted Grid"), this);
    mActGridDots->setCheckable(true);
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        mActGridDots->setChecked(settings.value(QStringLiteral("smDesign/gridDots"), false).toBool());
    }
    connect(mActGridDots, &QAction::toggled, this, [this](bool checked) {
        getScene().setGridStyle(checked ? NESMDesign::eGridStyle::Dots : NESMDesign::eGridStyle::Lines);
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        settings.setValue(QStringLiteral("smDesign/gridDots"), checked);
    });

    // The stored checked state was seeded before the connect (no toggled signal fired), so
    // push it to the scene explicitly - the canvas and the checked button must agree from
    // the very first paint (issue #514).
    getScene().setGridStyle(mActGridDots->isChecked() ? NESMDesign::eGridStyle::Dots : NESMDesign::eGridStyle::Lines);

    // The dot diameter is likewise an application-level display preference (spec 7.6 keeps
    // only grid size/visibility in the document). Seed the scene from the stored value.
    {
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        const int dotSize = settings.value(QStringLiteral("smDesign/gridDotSize"), NESMDesign::GridDotSizeDefault).toInt();
        getScene().setGridDotSize(dotSize);
    }

    mActGridDotSize = new QAction(tr("Dot Size..."), this);
    connect(mActGridDotSize, &QAction::triggered, this, [this]() {
        const int current = getScene().getGridDotSize();
        bool ok = false;
        const int value = QInputDialog::getInt(  this, tr("Dot Size"), tr("Dotted grid dot size (px):")
                                               , current, NESMDesign::GridDotSizeMin, NESMDesign::GridDotSizeMax, 1, &ok);
        if (ok && (value != current))
        {
            getScene().setGridDotSize(value);
            QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
            settings.setValue(QStringLiteral("smDesign/gridDotSize"), value);
        }
    });

    mActToggleSnap = new QAction(tr("Snap to Grid"), this);
    mActToggleSnap->setCheckable(true);
    mActToggleSnap->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
    connect(mActToggleSnap, &QAction::toggled, this, [this](bool checked) {
        getScene().setSnapToGrid(checked);
    });

    mActSelectAll = new QAction(tr("Select All"), this);
    mActSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(mActSelectAll, &QAction::triggered, this, [this]() {
        getScene().selectAll();
    });

    // No shortcut of their own: Ctrl+Z/Ctrl+Y are the global Edit-menu shortcut
    // (MdiMainWindow), forwarded to the active document's undo stack.
    mActUndo = new QAction(tr("Undo"), this);
    connect(mActUndo, &QAction::triggered, this, [this]() { mModel.getUndoStack().undo(); });

    mActRedo = new QAction(tr("Redo"), this);
    connect(mActRedo, &QAction::triggered, this, [this]() { mModel.getUndoStack().redo(); });

    mActAddState = new QAction(tr("Add State"), this);
    mActAddState->setShortcut(QKeySequence(Qt::Key_S));
    connect(mActAddState, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddState);
    });

    mActAddFinal = new QAction(tr("Add Final State"), this);
    mActAddFinal->setShortcut(QKeySequence(Qt::Key_F));
    connect(mActAddFinal, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddFinalState);
    });

    mActAddTransition = new QAction(tr("Add Transition"), this);
    mActAddTransition->setShortcut(QKeySequence(Qt::Key_T));
    connect(mActAddTransition, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddTransition);
    });

    mActAddNote = new QAction(tr("Add Note"), this);
    mActAddNote->setShortcut(QKeySequence(Qt::Key_N));
    connect(mActAddNote, &QAction::triggered, this, [this]() {
        getScene().setActiveTool(NESMDesign::eCanvasTool::AddNote);
    });

    mActStateColor = new QAction(tr("State Color..."), this);
    connect(mActStateColor, &QAction::triggered, this, [this]() { applyColorToSelection(eColorTarget::State); });

    mActEdgeColor = new QAction(tr("Transition Color..."), this);
    connect(mActEdgeColor, &QAction::triggered, this, [this]() { applyColorToSelection(eColorTarget::Edge); });

    mActNoteColor = new QAction(tr("Note Color..."), this);
    connect(mActNoteColor, &QAction::triggered, this, [this]() { applyColorToSelection(eColorTarget::Note); });

    // A single color action for the toolbar: colors whatever is selected, of any kind.
    mActSetColor = new QAction(tr("Set Color..."), this);
    connect(mActSetColor, &QAction::triggered, this, [this]() { applyColorToCurrentSelection(); });

    mActAlignLeft = new QAction(tr("Align Left"), this);
    connect(mActAlignLeft, &QAction::triggered, this, [this]() { alignSelection(eAlign::Left); });

    mActAlignRight = new QAction(tr("Align Right"), this);
    connect(mActAlignRight, &QAction::triggered, this, [this]() { alignSelection(eAlign::Right); });

    mActAlignTop = new QAction(tr("Align Top"), this);
    connect(mActAlignTop, &QAction::triggered, this, [this]() { alignSelection(eAlign::Top); });

    mActAlignBottom = new QAction(tr("Align Bottom"), this);
    connect(mActAlignBottom, &QAction::triggered, this, [this]() { alignSelection(eAlign::Bottom); });

    mActDistributeH = new QAction(tr("Distribute Horizontally"), this);
    connect(mActDistributeH, &QAction::triggered, this, [this]() { distributeSelection(eDistribute::Horizontal); });

    mActDistributeV = new QAction(tr("Distribute Vertically"), this);
    connect(mActDistributeV, &QAction::triggered, this, [this]() { distributeSelection(eDistribute::Vertical); });

    // An internal transition runs its operations on the stimulus without exit/entry
    // and no state change; it is shown as a row in the state body, not as an edge.
    mActAddInternal = new QAction(tr("Add Internal Transition"), this);
    connect(mActAddInternal, &QAction::triggered, this, &SMDesign::addInternalToSelection);

    mActSetStimulus = new QAction(tr("Set Stimulus..."), this);
    connect(mActSetStimulus, &QAction::triggered, this, &SMDesign::setStimulusOfSelection);

    mActRaisePriority = new QAction(tr("Raise Priority"), this);
    connect(mActRaisePriority, &QAction::triggered, this, [this]() { reorderSelectedTransition(true); });

    mActLowerPriority = new QAction(tr("Lower Priority"), this);
    connect(mActLowerPriority, &QAction::triggered, this, [this]() { reorderSelectedTransition(false); });

    mActDelete = new QAction(tr("Delete"), this);
    mActDelete->setShortcut(QKeySequence::Delete);
    connect(mActDelete, &QAction::triggered, this, &SMDesign::deleteSelection);

    mActRename = new QAction(tr("Rename"), this);
    mActRename->setShortcut(QKeySequence(Qt::Key_F2));
    connect(mActRename, &QAction::triggered, this, [this]() {
        getScene().startRenameOfSelection();
    });

    // Cut/Copy/Paste keep Qt::WidgetShortcut: the page itself never has focus, so the
    // registration stays inert and cannot turn the main window's Edit actions (which
    // carry the same key sequences and call back into this page) ambiguous. While the
    // canvas has focus the eventFilter dispatches the keys to these actions directly.
    mActCut = new QAction(tr("Cut"), this);
    mActCut->setShortcut(QKeySequence::Cut);
    mActCut->setShortcutContext(Qt::WidgetShortcut);
    connect(mActCut, &QAction::triggered, this, &SMDesign::cutSelection);

    mActCopy = new QAction(tr("Copy"), this);
    mActCopy->setShortcut(QKeySequence::Copy);
    mActCopy->setShortcutContext(Qt::WidgetShortcut);
    connect(mActCopy, &QAction::triggered, this, &SMDesign::copySelection);

    mActPaste = new QAction(tr("Paste"), this);
    mActPaste->setShortcut(QKeySequence::Paste);
    mActPaste->setShortcutContext(Qt::WidgetShortcut);
    connect(mActPaste, &QAction::triggered, this, &SMDesign::pasteClipboard);

    mActDuplicate = new QAction(tr("Duplicate"), this);
    mActDuplicate->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(mActDuplicate, &QAction::triggered, this, &SMDesign::duplicateSelection);

    mActAddSubstate = new QAction(tr("Add Substate (Painted)"), this);
    connect(mActAddSubstate, &QAction::triggered, this, &SMDesign::addSubstateToSelection);

    // Enter also descends: the scene handles the key itself, after the active tool.
    mActEnterSubmachine = new QAction(tr("Enter Submachine"), this);
    connect(mActEnterSubmachine, &QAction::triggered, this, &SMDesign::enterSelectedSubmachine);

    mActGoToParent = new QAction(tr("Go to Parent"), this);
    mActGoToParent->setShortcut(QKeySequence(Qt::Key_Backspace));
    connect(mActGoToParent, &QAction::triggered, this, [this]() {
        mSceneManager->goToParent();
    });

    // Scrolling far from the diagram easily "loses" it; this brings it back into view
    // without changing the zoom (issue #514).
    mActCenterMachine = new QAction(tr("Center Machine"), this);
    mActCenterMachine->setShortcut(QKeySequence(Qt::Key_Home));
    connect(mActCenterMachine, &QAction::triggered, this, &SMDesign::centerMachine);

    // The Declare dropdown: each entry asks the owning MDI window to switch to the right
    // page and start a new entry there (SMDesign does not know about sibling pages).
    const auto declare = [this](eDeclareKind kind) { emit signalDeclareRequested(kind); };
    mActNewTrigger = new QAction(tr("New Trigger"), this);
    connect(mActNewTrigger, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Trigger); });
    mActNewAction = new QAction(tr("New Action"), this);
    connect(mActNewAction, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Action); });
    mActNewCondition = new QAction(tr("New Condition"), this);
    connect(mActNewCondition, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Condition); });
    mActNewEvent = new QAction(tr("New Event"), this);
    connect(mActNewEvent, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Event); });
    mActNewTimer = new QAction(tr("New Timer"), this);
    connect(mActNewTimer, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Timer); });
    mActNewAttribute = new QAction(tr("New Attribute"), this);
    connect(mActNewAttribute, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Attribute); });
    mActNewConstant = new QAction(tr("New Constant"), this);
    connect(mActNewConstant, &QAction::triggered, this, [declare]() { declare(eDeclareKind::Constant); });
    mActNewDataType = new QAction(tr("New Data Type"), this);
    connect(mActNewDataType, &QAction::triggered, this, [declare]() { declare(eDeclareKind::DataType); });

    // Vector glyph icons so the (relocated, icon-only by default) toolbar is usable; the
    // icons also show in the Design menu and context menus alongside the labels.
    using SMToolIcons::eIcon;
    mActAddState->setIcon(SMToolIcons::icon(eIcon::AddState));
    mActAddFinal->setIcon(SMToolIcons::icon(eIcon::AddFinalState));
    mActAddTransition->setIcon(SMToolIcons::icon(eIcon::AddTransition));
    mActAddNote->setIcon(SMToolIcons::icon(eIcon::AddNote));
    mActStateColor->setIcon(SMToolIcons::icon(eIcon::StateColor));
    mActEdgeColor->setIcon(SMToolIcons::icon(eIcon::EdgeColor));
    mActNoteColor->setIcon(SMToolIcons::icon(eIcon::NoteColor));
    mActSetColor->setIcon(SMToolIcons::icon(eIcon::StateColor));
    mActAlignLeft->setIcon(SMToolIcons::icon(eIcon::AlignLeft));
    mActAlignRight->setIcon(SMToolIcons::icon(eIcon::AlignRight));
    mActAlignTop->setIcon(SMToolIcons::icon(eIcon::AlignTop));
    mActAlignBottom->setIcon(SMToolIcons::icon(eIcon::AlignBottom));
    mActDistributeH->setIcon(SMToolIcons::icon(eIcon::DistributeHorizontal));
    mActDistributeV->setIcon(SMToolIcons::icon(eIcon::DistributeVertical));
    mActToggleSnap->setIcon(SMToolIcons::icon(eIcon::ToggleSnap));
    mActToggleGrid->setIcon(SMToolIcons::icon(eIcon::ToggleGrid));
    mActGridDots->setIcon(SMToolIcons::icon(eIcon::GridDots));
    mActGridDotSize->setIcon(SMToolIcons::icon(eIcon::GridDotSize));
    mActGridSize->setIcon(SMToolIcons::icon(eIcon::GridSize));
    mActEnterSubmachine->setIcon(SMToolIcons::icon(eIcon::EnterSubmachine));
    mActGoToParent->setIcon(SMToolIcons::icon(eIcon::GoToParent));
    mActCenterMachine->setIcon(SMToolIcons::icon(eIcon::CenterMachine));
    mActZoomIn->setIcon(SMToolIcons::icon(eIcon::ZoomIn));
    mActZoomOut->setIcon(SMToolIcons::icon(eIcon::ZoomOut));
    mActZoomReset->setIcon(SMToolIcons::icon(eIcon::ZoomReset));
    mActZoomFit->setIcon(SMToolIcons::icon(eIcon::ZoomFit));
    mActUndo->setIcon(SMToolIcons::icon(eIcon::Undo));
    mActRedo->setIcon(SMToolIcons::icon(eIcon::Redo));
    mActSelectAll->setIcon(SMToolIcons::icon(eIcon::SelectAll));
    mActCut->setIcon(SMToolIcons::icon(eIcon::Cut));
    mActCopy->setIcon(SMToolIcons::icon(eIcon::Copy));
    mActPaste->setIcon(SMToolIcons::icon(eIcon::Paste));
    mActDuplicate->setIcon(SMToolIcons::icon(eIcon::Duplicate));
    mActNewTrigger->setIcon(SMToolIcons::icon(eIcon::NewTrigger));
    mActNewAction->setIcon(SMToolIcons::icon(eIcon::NewAction));
    mActNewCondition->setIcon(SMToolIcons::icon(eIcon::NewCondition));
    mActNewEvent->setIcon(SMToolIcons::icon(eIcon::NewEvent));
    mActNewTimer->setIcon(SMToolIcons::icon(eIcon::NewTimer));
    mActNewAttribute->setIcon(SMToolIcons::icon(eIcon::NewAttribute));
    mActNewConstant->setIcon(SMToolIcons::icon(eIcon::NewConstant));
    mActNewDataType->setIcon(SMToolIcons::icon(eIcon::NewDataType));

    const QList<QAction*> actions{ mActZoomIn, mActZoomOut, mActZoomReset, mActZoomFit
                                 , mActToggleGrid, mActGridDots, mActGridDotSize, mActToggleSnap, mActGridSize, mActSelectAll
                                 , mActUndo, mActRedo
                                 , mActAddState, mActAddFinal, mActAddTransition, mActAddNote, mActAddInternal
                                 , mActDelete, mActRename, mActDuplicate
                                 , mActStateColor, mActEdgeColor, mActNoteColor, mActSetColor
                                 , mActAlignLeft, mActAlignRight, mActAlignTop, mActAlignBottom
                                 , mActDistributeH, mActDistributeV
                                 , mActSetStimulus, mActRaisePriority, mActLowerPriority
                                 , mActAddSubstate, mActEnterSubmachine, mActGoToParent, mActCenterMachine
                                 , mActNewTrigger, mActNewAction, mActNewCondition, mActNewEvent, mActNewTimer
                                 , mActNewAttribute, mActNewConstant, mActNewDataType };
    for (QAction* action : actions)
    {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(action);
    }

    // Registered for matchAction() dispatch; their WidgetShortcut context stays as set.
    addAction(mActCut);
    addAction(mActCopy);
    addAction(mActPaste);

    // Seeding the checked state must not push a grid command for simply opening the page.
    mSyncingGrid = true;
    mActToggleGrid->setChecked(getScene().isGridVisible());
    mSyncingGrid = false;
    mActToggleSnap->setChecked(getScene().isSnapToGrid());
}

QList<QAction*> SMDesign::declareActions() const
{
    return { mActNewTrigger, mActNewAction, mActNewCondition, mActNewEvent, mActNewTimer
           , mActNewAttribute, mActNewConstant, mActNewDataType };
}

void SMDesign::setToolbarVisible(bool visible)
{
    // The toolbar now lives in the navigation dock (global), shown/raised from the View menu;
    // this per-child flag is kept only for the MdiChild contract.
    mToolbarVisible = visible;
}

bool SMDesign::isToolbarVisible() const
{
    return mToolbarVisible;
}

QList<SMDesign::ToolGroup> SMDesign::toolGroups() const
{
    // Every tool's tooltip shows its shortcut alongside the label (spec 9.2). Refresh here so
    // the tooltip is present whichever way the toolbar is (re)built.
    for (QAction* action : actions())
    {
        if (action->shortcut().isEmpty() == false)
        {
            action->setToolTip(QStringLiteral("%1 (%2)").arg(action->text(), action->shortcut().toString(QKeySequence::NativeText)));
        }
    }

    // Ordered by importance (spec 9.2 / issue #514): the design/placement operations first,
    // the declarations right after them, then alignment, level navigation, color, grid,
    // edit, and zoom. The Design group order matches the canvas context menu.
    QList<ToolGroup> groups;
    groups.append(ToolGroup{ tr("Design"),    { mActAddState, mActAddTransition, mActAddNote, mActAddFinal } });
    groups.append(ToolGroup{ tr("Declare"),   declareActions() });
    groups.append(ToolGroup{ tr("Alignment"), { mActAlignLeft, mActAlignRight, mActAlignTop, mActAlignBottom
                                               , mActDistributeH, mActDistributeV } });
    groups.append(ToolGroup{ tr("Navigate"),  { mActEnterSubmachine, mActGoToParent, mActCenterMachine } });
    groups.append(ToolGroup{ tr("Color"),     { mActSetColor } });
    groups.append(ToolGroup{ tr("Grid"),      { mActToggleGrid, mActGridDots, mActGridDotSize, mActGridSize, mActToggleSnap } });
    groups.append(ToolGroup{ tr("Edit"),      { mActUndo, mActRedo, mActCut, mActCopy, mActPaste, mActSelectAll } });
    groups.append(ToolGroup{ tr("Zoom"),      { mActZoomIn, mActZoomOut, mActZoomReset, mActZoomFit } });
    return groups;
}

QList<SMDesign::ToolGroup> SMDesign::placeholderToolGroups(QObject& owner)
{
    using SMToolIcons::eIcon;

    // Display-only stand-ins: same titles, order, icons, and labels as toolGroups(), but
    // disabled - the Design Toolbar tab always shows its buttons, they just do not act
    // until a State Machine's Design page is open (issue #514). Keep in sync with
    // toolGroups() above.
    const auto make = [&owner](eIcon glyph, const QString& text) -> QAction*
    {
        QAction* action = new QAction(SMToolIcons::icon(glyph), text, &owner);
        action->setEnabled(false);
        return action;
    };

    QList<ToolGroup> groups;
    groups.append(ToolGroup{ tr("Design"),    { make(eIcon::AddState, tr("Add State"))
                                              , make(eIcon::AddTransition, tr("Add Transition"))
                                              , make(eIcon::AddNote, tr("Add Note"))
                                              , make(eIcon::AddFinalState, tr("Add Final State")) } });
    groups.append(ToolGroup{ tr("Declare"),   { make(eIcon::NewTrigger, tr("New Trigger"))
                                              , make(eIcon::NewAction, tr("New Action"))
                                              , make(eIcon::NewCondition, tr("New Condition"))
                                              , make(eIcon::NewEvent, tr("New Event"))
                                              , make(eIcon::NewTimer, tr("New Timer"))
                                              , make(eIcon::NewAttribute, tr("New Attribute"))
                                              , make(eIcon::NewConstant, tr("New Constant"))
                                              , make(eIcon::NewDataType, tr("New Data Type")) } });
    groups.append(ToolGroup{ tr("Alignment"), { make(eIcon::AlignLeft, tr("Align Left"))
                                              , make(eIcon::AlignRight, tr("Align Right"))
                                              , make(eIcon::AlignTop, tr("Align Top"))
                                              , make(eIcon::AlignBottom, tr("Align Bottom"))
                                              , make(eIcon::DistributeHorizontal, tr("Distribute Horizontally"))
                                              , make(eIcon::DistributeVertical, tr("Distribute Vertically")) } });
    groups.append(ToolGroup{ tr("Navigate"),  { make(eIcon::EnterSubmachine, tr("Enter Submachine"))
                                              , make(eIcon::GoToParent, tr("Go to Parent"))
                                              , make(eIcon::CenterMachine, tr("Center Machine")) } });
    groups.append(ToolGroup{ tr("Color"),     { make(eIcon::StateColor, tr("Set Color...")) } });
    groups.append(ToolGroup{ tr("Grid"),      { make(eIcon::ToggleGrid, tr("Show Grid"))
                                              , make(eIcon::GridDots, tr("Dotted Grid"))
                                              , make(eIcon::GridDotSize, tr("Dot Size..."))
                                              , make(eIcon::GridSize, tr("Grid Size..."))
                                              , make(eIcon::ToggleSnap, tr("Snap to Grid")) } });
    groups.append(ToolGroup{ tr("Edit"),      { make(eIcon::Undo, tr("Undo"))
                                              , make(eIcon::Redo, tr("Redo"))
                                              , make(eIcon::Cut, tr("Cut"))
                                              , make(eIcon::Copy, tr("Copy"))
                                              , make(eIcon::Paste, tr("Paste"))
                                              , make(eIcon::SelectAll, tr("Select All")) } });
    groups.append(ToolGroup{ tr("Zoom"),      { make(eIcon::ZoomIn, tr("Zoom In"))
                                              , make(eIcon::ZoomOut, tr("Zoom Out"))
                                              , make(eIcon::ZoomReset, tr("Zoom 100%"))
                                              , make(eIcon::ZoomFit, tr("Zoom to Fit")) } });
    return groups;
}

bool SMDesign::placementToolFor(QAction* action, NESMDesign::eCanvasTool& toolOut) const
{
    if (action == mActAddState)           { toolOut = NESMDesign::eCanvasTool::AddState;      return true; }
    if (action == mActAddFinal)           { toolOut = NESMDesign::eCanvasTool::AddFinalState; return true; }
    if (action == mActAddTransition)      { toolOut = NESMDesign::eCanvasTool::AddTransition; return true; }
    if (action == mActAddNote)            { toolOut = NESMDesign::eCanvasTool::AddNote;       return true; }
    return false;
}

void SMDesign::armStickyTool(NESMDesign::eCanvasTool tool)
{
    // The single click already activated the tool single-shot; re-activating it sticky just
    // flips the flag (SMScene::setActiveTool early-exits when the kind is unchanged).
    getScene().setActiveTool(tool, true);
}

void SMDesign::onViewContextMenuRequested(const QPoint& pos)
{
    SMScene& scene = getScene();
    const QPointF scenePos = mView->mapToScene(pos);

    SMStateItem* state = scene.stateAt(scenePos);
    SMEdgeItem*  edge   = nullptr;
    SMNoteItem*  note   = nullptr;
    if (state == nullptr)
    {
        for (QGraphicsItem* item : scene.items(scenePos))
        {
            if (edge == nullptr)
            {
                edge = dynamic_cast<SMEdgeItem*>(item);
            }

            if (note == nullptr)
            {
                note = dynamic_cast<SMNoteItem*>(item);
            }
        }
    }

    // Undo/Redo carry no shortcut of their own; reflect the document stack's state each
    // time the menu opens so the shared group shows them enabled only when they act.
    mActUndo->setEnabled(mModel.getUndoStack().canUndo());
    mActRedo->setEnabled(mModel.getUndoStack().canRedo());

    QMenu menu(this);
    if (state != nullptr)
    {
        if (state->isSelected() == false)
        {
            scene.clearSelection();
            state->setSelected(true);
        }

        menu.addAction(mActAddInternal);
        menu.addAction(mActAddSubstate);
        menu.addAction(mActEnterSubmachine);
        menu.addSeparator();
        menu.addAction(mActCut);
        menu.addAction(mActCopy);
        menu.addAction(mActDuplicate);
        menu.addSeparator();
        menu.addAction(mActStateColor);
        addNoteMenuEntries(menu, state->getElementId(), true);
        menu.addSeparator();
        menu.addAction(mActRename);
        menu.addAction(mActDelete);
    }
    else if (edge != nullptr)
    {
        if (edge->isSelected() == false)
        {
            scene.clearSelection();
            edge->setSelected(true);
        }

        menu.addAction(mActSetStimulus);
        menu.addAction(mActRaisePriority);
        menu.addAction(mActLowerPriority);
        menu.addSeparator();
        menu.addAction(mActEdgeColor);
        addNoteMenuEntries(menu, edge->getElementId(), false);
        menu.addSeparator();
        menu.addAction(mActDelete);
    }
    else if (note != nullptr)
    {
        if (note->isSelected() == false)
        {
            scene.clearSelection();
            note->setSelected(true);
        }

        menu.addAction(mActNoteColor);
        menu.addSeparator();
        menu.addAction(mActCut);
        menu.addAction(mActCopy);
        menu.addAction(mActDuplicate);
        menu.addSeparator();
        menu.addAction(mActDelete);
    }
    else
    {
        // The empty-canvas menu mirrors the Design Toolbar tab's group order (issue #514):
        // the Design group on top (same action order as the toolbar), then alignment,
        // then navigation; the grid/view helpers follow, "Show Design Toolbar" closes.
        menu.addAction(mActAddState);
        menu.addAction(mActAddTransition);
        menu.addAction(mActAddNote);
        menu.addAction(mActAddFinal);
        menu.addSeparator();
        menu.addAction(mActPaste);
        menu.addSeparator();
        menu.addAction(mActAlignLeft);
        menu.addAction(mActAlignRight);
        menu.addAction(mActAlignTop);
        menu.addAction(mActAlignBottom);
        menu.addAction(mActDistributeH);
        menu.addAction(mActDistributeV);
        menu.addSeparator();
        menu.addAction(mActEnterSubmachine);
        menu.addAction(mActGoToParent);
        menu.addAction(mActCenterMachine);
        menu.addSeparator();
        menu.addAction(mActSelectAll);
        menu.addAction(mActZoomFit);
        menu.addAction(mActToggleGrid);
        menu.addAction(mActGridDots);
        menu.addAction(mActGridSize);
        menu.addSeparator();
        menu.addAction(mActUndo);
        menu.addAction(mActRedo);
    }

    if ((state != nullptr) || (edge != nullptr) || (note != nullptr))
    {
        // The shared command group (spec 9.3): present in every element context so the four
        // core actions stay reachable (the canvas menu above already leads with them).
        menu.addSeparator();
        menu.addAction(mActAddState);
        menu.addAction(mActAddTransition);
        menu.addAction(mActUndo);
        menu.addAction(mActRedo);
    }

    // The user is on the Design page (this menu only opens over the canvas): offer a quick
    // way to bring the drawing toolbar tab to the front (spec issue #514).
    menu.addSeparator();
    QAction* showTools = menu.addAction(tr("Show Design Toolbar"));
    showTools->setIcon(NELusanCommon::iconViewFsmDesign(NELusanCommon::SizeSmall));
    connect(showTools, &QAction::triggered, this, [this]() { emit signalShowDesignTools(); });

    menu.exec(mView->viewport()->mapToGlobal(pos));
}

void SMDesign::deleteSelection()
{
    SMScene& scene = getScene();
    const QList<SMStateItem*> selection{ scene.selectedStateItems() };
    if (selection.isEmpty())
    {
        if (scene.selectedNoteItems().isEmpty() == false)
        {
            deleteSelectedNotes();
        }
        else
        {
            deleteSelectedEdges();
        }

        return;
    }

    StateMachineModel& model = mModel;
    StateMachineData&  data  = model.getData();
    SMStateData* level = data.findLevel(scene.getLevelId());
    if (level == nullptr)
    {
        return;
    }

    // The Start state is auto-created with its level and cannot be recreated by a tool.
    QStringList names;
    QList<uint32_t> deletable;
    bool skippedStart{ false };
    int  substates{ 0 };
    for (const SMStateItem* item : selection)
    {
        const SMStateEntry* state = data.findStateById(item->getElementId());
        if (state == nullptr)
        {
            continue;
        }

        if (state->getKind() == SMStateEntry::eStateKind::Start)
        {
            skippedStart = true;
            continue;
        }

        names.append(state->getName());
        deletable.append(state->getId());
        if (state->hasNestedStates())
        {
            substates += state->getNestedStates()->countStatesRecursive();
        }
    }

    if (deletable.isEmpty())
    {
        QMessageBox::information(this, tr("Delete States")
                                 , tr("The Start state cannot be deleted - every machine level needs exactly one."));
        return;
    }

    QString message = (deletable.size() == 1)
            ? tr("Delete state '%1'?").arg(names.first())
            : tr("Delete %1 states (%2)?").arg(deletable.size()).arg(names.join(QStringLiteral(", ")));
    if (substates > 0)
    {
        message += QStringLiteral("\n") + tr("%1 painted substate(s) are deleted with it.").arg(substates);
    }

    message += QStringLiteral("\n") + tr("Transitions from and to the deleted state(s) are deleted too.");
    if (skippedStart)
    {
        message += QStringLiteral("\n") + tr("The selected Start state is kept - every level needs one.");
    }

    const QMessageBox::StandardButton answer =
            QMessageBox::question(this, tr("Delete States"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
    {
        return;
    }

    const QString text = (deletable.size() == 1)
            ? tr("Delete state %1").arg(names.first())
            : tr("Delete %1 states").arg(deletable.size());
    if (deletable.size() == 1)
    {
        model.getUndoStack().push(new SMRemoveStateCommand(data, model.getNotifier(), *level, deletable.first(), text));
    }
    else
    {
        SMCompositeCommand* composite = new SMCompositeCommand(data, model.getNotifier(), text);
        for (uint32_t stateId : deletable)
        {
            new SMRemoveStateCommand(data, model.getNotifier(), *level, stateId, text, composite);
        }

        model.getUndoStack().push(composite);
    }
}

void SMDesign::deleteSelectedEdges()
{
    SMScene& scene = getScene();
    const QList<SMEdgeItem*> edges{ scene.selectedEdgeItems() };
    if (edges.isEmpty())
    {
        return;
    }

    StateMachineData& data = mModel.getData();

    // Pair each edge's transition with its owning state.
    QList<QPair<SMStateEntry*, uint32_t>> targets;
    for (const SMEdgeItem* edge : edges)
    {
        SMStateEntry* owner = data.findTransitionOwner(edge->getElementId());
        if (owner != nullptr)
        {
            targets.append(qMakePair(owner, edge->getElementId()));
        }
    }

    if (targets.isEmpty())
    {
        return;
    }

    const QString question = (targets.size() == 1)
            ? tr("Delete the selected transition?")
            : tr("Delete %1 selected transitions?").arg(targets.size());
    if (QMessageBox::question(this, tr("Delete Transitions"), question
                              , QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    const QString text = (targets.size() == 1) ? tr("Delete transition") : tr("Delete %1 transitions").arg(targets.size());
    if (targets.size() == 1)
    {
        mModel.getUndoStack().push(new SMRemoveTransitionCommand(data, mModel.getNotifier(), *targets.first().first, targets.first().second, text));
    }
    else
    {
        SMCompositeCommand* composite = new SMCompositeCommand(data, mModel.getNotifier(), text);
        for (const QPair<SMStateEntry*, uint32_t>& target : targets)
        {
            new SMRemoveTransitionCommand(data, mModel.getNotifier(), *target.first, target.second, text, composite);
        }

        mModel.getUndoStack().push(composite);
    }
}

void SMDesign::deleteSelectedNotes()
{
    const QList<SMNoteItem*> notes{ getScene().selectedNoteItems() };
    if (notes.isEmpty())
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const QString text = (notes.size() == 1) ? tr("Delete note") : tr("Delete %1 notes").arg(notes.size());
    if (notes.size() == 1)
    {
        mModel.getUndoStack().push(new SMRemoveNoteCommand(data, mModel.getNotifier(), notes.first()->getElementId(), text));
    }
    else
    {
        SMCompositeCommand* composite = new SMCompositeCommand(data, mModel.getNotifier(), text);
        for (SMNoteItem* note : notes)
        {
            new SMRemoveNoteCommand(data, mModel.getNotifier(), note->getElementId(), text, composite);
        }

        mModel.getUndoStack().push(composite);
    }
}

void SMDesign::copySelection()
{
    const QString xml = SMClipboard::serialize(mModel.getData(), mModel.getSelectionModel().getSelection());
    if (xml.isEmpty())
    {
        return;
    }

    QMimeData* mime = new QMimeData();
    mime->setData(QString::fromLatin1(SMClipboard::MIME_TYPE), xml.toUtf8());
    mime->setText(xml);
    QGuiApplication::clipboard()->setMimeData(mime);
}

void SMDesign::cutSelection()
{
    StateMachineData& data = mModel.getData();
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    const QString xml = SMClipboard::serialize(data, selection);
    if (xml.isEmpty())
    {
        return;
    }

    QMimeData* mime = new QMimeData();
    mime->setData(QString::fromLatin1(SMClipboard::MIME_TYPE), xml.toUtf8());
    mime->setText(xml);
    QGuiApplication::clipboard()->setMimeData(mime);

    // Cut deletes only what the clipboard carries as canvas content: the copied states
    // and free notes. Registry entries are copied but stay; their pages own deletion.
    QList<uint32_t> stateIds;
    QList<uint32_t> noteIds;
    for (uint32_t id : selection)
    {
        const SMStateEntry* state = data.findStateById(id);
        if (state != nullptr)
        {
            if (state->getKind() != SMStateEntry::eStateKind::Start)
            {
                stateIds.append(id);
            }
        }
        else if (data.getLayout().findNote(id) != nullptr)
        {
            noteIds.append(id);
        }
    }

    if (stateIds.isEmpty() && noteIds.isEmpty())
    {
        return;
    }

    SMStateData* level = data.findLevel(getScene().getLevelId());
    if ((level == nullptr) && (stateIds.isEmpty() == false))
    {
        return;
    }

    const QString text = tr("Cut selection");
    SMCompositeCommand* composite = new SMCompositeCommand(data, mModel.getNotifier(), text);
    for (uint32_t stateId : stateIds)
    {
        new SMRemoveStateCommand(data, mModel.getNotifier(), *level, stateId, text, composite);
    }

    for (uint32_t noteId : noteIds)
    {
        new SMRemoveNoteCommand(data, mModel.getNotifier(), noteId, text, composite);
    }

    mModel.getUndoStack().push(composite);
}

void SMDesign::pasteClipboard()
{
    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if ((mime == nullptr) || (mime->hasFormat(QString::fromLatin1(SMClipboard::MIME_TYPE)) == false))
    {
        return;
    }

    std::unique_ptr<SMClipboardContent> content =
            SMClipboard::parse(QString::fromUtf8(mime->data(QString::fromLatin1(SMClipboard::MIME_TYPE))));
    if (content != nullptr)
    {
        pushPaste(std::move(content), tr("Paste"));
    }
}

void SMDesign::duplicateSelection()
{
    const QString xml = SMClipboard::serialize(mModel.getData(), mModel.getSelectionModel().getSelection());
    if (xml.isEmpty())
    {
        return;
    }

    std::unique_ptr<SMClipboardContent> content = SMClipboard::parse(xml);
    if (content != nullptr)
    {
        pushPaste(std::move(content), tr("Duplicate"));
    }
}

void SMDesign::pushPaste(std::unique_ptr<SMClipboardContent> content, const QString& text)
{
    const double offset = static_cast<double>(getScene().getGridSize());
    SMPasteCommand* command = new SMPasteCommand(  mModel.getData(), mModel.getNotifier(), std::move(content)
                                                 , getScene().getLevelId(), QPointF(offset, offset), text);
    if (command->isEffective() == false)
    {
        delete command;
        return;
    }

    // No ensureVisible here: scrolling would push a viewport command on top of the
    // paste and Ctrl+Z would undo the scroll instead of the paste. The offset keeps
    // the copy next to its (visible) original anyway.
    mModel.getUndoStack().push(command);
    mModel.getSelectionModel().setSelection(command->getPastedIds());
}

void SMDesign::reorderSelectedTransition(bool raise)
{
    const QList<SMEdgeItem*> edges{ getScene().selectedEdgeItems() };
    if (edges.size() != 1)
    {
        return;
    }

    const uint32_t transitionId = edges.first()->getElementId();
    StateMachineData& data = mModel.getData();
    SMStateEntry* owner = data.findTransitionOwner(transitionId);
    if (owner == nullptr)
    {
        return;
    }

    SMTransitionData& list = owner->getTransitions();
    const int index = list.findIndex(transitionId);
    const int other = (raise ? index - 1 : index + 1);
    if ((index < 0) || (other < 0) || (other >= list.getElementCount()))
    {
        return;
    }

    // The swap keeps IDs position-keyed; the moved content ends up under the other slot's
    // ID, so re-select that ID to keep the selection on the transition the user moved.
    const uint32_t followId = list.getElements().at(other)->getId();
    const QString text = raise ? tr("Raise transition priority") : tr("Lower transition priority");
    mModel.getUndoStack().push(new TDocReorderCommand<SMTransitionEntry*, DocumentElem>(  mModel.getNotifier(), list
                                                                                        , index, other, owner->getId()
                                                                                        , eDocElementKind::Transition, text));
    mModel.getSelectionModel().setSelection(QList<uint32_t>{ followId });
}

void SMDesign::setStimulusOfSelection()
{
    const QList<SMEdgeItem*> edges{ getScene().selectedEdgeItems() };
    if (edges.size() != 1)
    {
        return;
    }

    const uint32_t transitionId = edges.first()->getElementId();
    StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return;
    }

    // Gather the shared stimulus name space: triggers, events, timers.
    QStringList labels;
    QList<QPair<SMTransitionEntry::eStimulusKind, QString>> options;
    const auto add = [&labels, &options](SMTransitionEntry::eStimulusKind kind, const QString& name)
    {
        labels.append(QString::fromLatin1(SMTransitionEntry::toString(kind)) + QStringLiteral(" - ") + name);
        options.append(qMakePair(kind, name));
    };

    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if (method->getMethodType() == SMMethodEntry::eMethodType::Trigger)
        {
            add(SMTransitionEntry::eStimulusKind::Trigger, method->getName());
        }
    }

    for (const SMEventEntry* event : data.getEvents().getElements())
    {
        add(SMTransitionEntry::eStimulusKind::Event, event->getName());
    }

    for (const SMTimerEntry& timer : data.getTimers().getElements())
    {
        add(SMTransitionEntry::eStimulusKind::Timer, timer.getName());
    }

    if (options.isEmpty())
    {
        QMessageBox::information(this, tr("Set Stimulus")
                                 , tr("Declare a trigger, event, or timer first - the stimulus is picked from those registries."));
        return;
    }

    int current = 0;
    for (int i = 0; i < options.size(); ++i)
    {
        if ((options.at(i).first == transition->getStimulusKind()) && (options.at(i).second == transition->getStimulus()))
        {
            current = i;
            break;
        }
    }

    bool accepted = false;
    const QString chosen = QInputDialog::getItem(this, tr("Set Stimulus"), tr("Stimulus:"), labels, current, false, &accepted);
    if (accepted == false)
    {
        return;
    }

    const int index = labels.indexOf(chosen);
    if (index >= 0)
    {
        mModel.getUndoStack().push(new SMSetStimulusCommand(  data, mModel.getNotifier(), transitionId
                                                           , options.at(index).first, options.at(index).second
                                                           , tr("Set stimulus")));
    }
}

void SMDesign::addInternalToSelection()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    SMStateEntry* state = data.findStateById(selection.first());
    if (state == nullptr)
    {
        return;
    }

    // No target and no edge geometry: internal transitions render as a state-body row.
    mModel.getUndoStack().push(new SMCreateTransitionCommand(  data, mModel.getNotifier(), *state
                                                             , SMTransitionEntry::eStimulusKind::Trigger, QString()
                                                             , QString(), QList<QPointF>()
                                                             , tr("Add internal transition to %1").arg(state->getName())));
}

void SMDesign::addNoteMenuEntries(QMenu& menu, uint32_t ownerId, bool isState)
{
    const bool hasNote = (mModel.getData().getLayout().findNoteByOwner(ownerId) != nullptr);
    if (hasNote)
    {
        QAction* edit = menu.addAction(tr("Edit Note"));
        connect(edit, &QAction::triggered, this, [this, ownerId]() { editOwnedNote(ownerId); });
        QAction* remove = menu.addAction(tr("Remove Note"));
        connect(remove, &QAction::triggered, this, [this, ownerId]() { removeOwnedNote(ownerId); });
    }
    else
    {
        QAction* add = menu.addAction(tr("Add Note"));
        connect(add, &QAction::triggered, this, [this, ownerId, isState]() { addOwnedNote(ownerId, isState); });
    }
}

void SMDesign::addOwnedNote(uint32_t ownerId, bool isState)
{
    StateMachineData& data = mModel.getData();
    if (data.getLayout().findNoteByOwner(ownerId) != nullptr)
    {
        editOwnedNote(ownerId);
        return;
    }

    // The note geometry is a reference box over the owner: the owner's box for a state, a
    // small box near the transition for an edge. Owned notes render as a badge, not a box.
    QRectF box{ 0.0, 0.0, NESMDesign::NoteDefaultWidth, NESMDesign::NoteDefaultHeight };
    SMStateItem* state = getScene().stateItem(ownerId);
    if (isState && (state != nullptr))
    {
        box = state->getBoxGeometry();
    }

    mModel.getUndoStack().push(new SMAddNoteCommand(  data, mModel.getNotifier()
                                                    , getScene().getLevelId(), ownerId, box, QString()
                                                    , tr("Add note")));
    editOwnedNote(ownerId);
}

void SMDesign::editOwnedNote(uint32_t ownerId)
{
    if (SMStateItem* state = getScene().stateItem(ownerId))
    {
        state->startNoteEdit();
        return;
    }

    if (SMEdgeItem* edge = dynamic_cast<SMEdgeItem*>(getScene().findCanvasItem(ownerId)))
    {
        edge->startNoteEdit();
    }
}

void SMDesign::removeOwnedNote(uint32_t ownerId)
{
    const SMLayoutNote* note = mModel.getData().getLayout().findNoteByOwner(ownerId);
    if (note != nullptr)
    {
        mModel.getUndoStack().push(new SMRemoveNoteCommand(  mModel.getData(), mModel.getNotifier()
                                                           , note->id, tr("Remove note")));
    }
}

void SMDesign::applyColorToSelection(eColorTarget target)
{
    SMScene& scene = getScene();
    QList<uint32_t> ids;
    switch (target)
    {
    case eColorTarget::State:
        for (SMStateItem* item : scene.selectedStateItems())
        {
            ids.append(item->getElementId());
        }
        break;

    case eColorTarget::Edge:
        for (SMEdgeItem* item : scene.selectedEdgeItems())
        {
            ids.append(item->getElementId());
        }
        break;

    case eColorTarget::Note:
        for (SMNoteItem* item : scene.selectedNoteItems())
        {
            ids.append(item->getElementId());
        }
        break;
    }

    if (ids.isEmpty())
    {
        return;
    }

    const QColor chosen = QColorDialog::getColor(Qt::white, this, tr("Choose Color"));
    if (chosen.isValid() == false)
    {
        return;
    }

    const QString      colorName = chosen.name(QColor::HexRgb);
    StateMachineData&  data      = mModel.getData();
    DocModelNotifier&  notifier  = mModel.getNotifier();
    const QString       text     = tr("Change color");
    const bool           single  = (ids.size() == 1);
    QUndoCommand*        parent  = single ? nullptr : new SMCompositeCommand(data, notifier, text);

    for (uint32_t id : ids)
    {
        QUndoCommand* command = nullptr;
        switch (target)
        {
        case eColorTarget::State: command = new SMSetNodeColorCommand(data, notifier, id, colorName, text, parent); break;
        case eColorTarget::Edge:  command = new SMSetEdgeColorCommand(data, notifier, id, colorName, text, parent); break;
        case eColorTarget::Note:  command = new SMSetNoteColorCommand(data, notifier, id, colorName, text, parent); break;
        }

        if (single)
        {
            mModel.getUndoStack().push(command);
        }
    }

    if (single == false)
    {
        mModel.getUndoStack().push(parent);
    }
}

void SMDesign::applyColorToCurrentSelection()
{
    SMScene& scene = getScene();

    // Every selected item, tagged with the command that colors its kind.
    QList<QPair<uint32_t, eColorTarget>> targets;
    for (SMStateItem* item : scene.selectedStateItems())
    {
        targets.append(qMakePair(item->getElementId(), eColorTarget::State));
    }
    for (SMEdgeItem* item : scene.selectedEdgeItems())
    {
        targets.append(qMakePair(item->getElementId(), eColorTarget::Edge));
    }
    for (SMNoteItem* item : scene.selectedNoteItems())
    {
        targets.append(qMakePair(item->getElementId(), eColorTarget::Note));
    }

    if (targets.isEmpty())
    {
        return;
    }

    const QColor chosen = QColorDialog::getColor(Qt::white, this, tr("Choose Color"));
    if (chosen.isValid() == false)
    {
        return;
    }

    const QString      colorName = chosen.name(QColor::HexRgb);
    StateMachineData&  data      = mModel.getData();
    DocModelNotifier&  notifier  = mModel.getNotifier();
    const QString      text      = tr("Change color");
    const bool         single    = (targets.size() == 1);
    QUndoCommand*      parent    = single ? nullptr : new SMCompositeCommand(data, notifier, text);

    for (const QPair<uint32_t, eColorTarget>& target : targets)
    {
        QUndoCommand* command = nullptr;
        switch (target.second)
        {
        case eColorTarget::State: command = new SMSetNodeColorCommand(data, notifier, target.first, colorName, text, parent); break;
        case eColorTarget::Edge:  command = new SMSetEdgeColorCommand(data, notifier, target.first, colorName, text, parent); break;
        case eColorTarget::Note:  command = new SMSetNoteColorCommand(data, notifier, target.first, colorName, text, parent); break;
        }

        if (single)
        {
            mModel.getUndoStack().push(command);
        }
    }

    if (single == false)
    {
        mModel.getUndoStack().push(parent);
    }
}

void SMDesign::alignSelection(eAlign align)
{
    const QList<SelectionBox> boxes{ collectSelectionBoxes(getScene()) };
    if (boxes.size() < 2)
    {
        return;
    }

    double target = boxes.first().rect.left();
    for (const SelectionBox& box : boxes)
    {
        switch (align)
        {
        case eAlign::Left:   target = std::min(target, box.rect.left());   break;
        case eAlign::Right:  target = std::max(target, box.rect.right());  break;
        case eAlign::Top:    target = std::min(target, box.rect.top());    break;
        case eAlign::Bottom: target = std::max(target, box.rect.bottom()); break;
        }
    }

    QList<QPair<SelectionBox, QRectF>> changed;
    for (const SelectionBox& box : boxes)
    {
        QRectF rect{ box.rect };
        switch (align)
        {
        case eAlign::Left:   rect.moveLeft(target);   break;
        case eAlign::Right:  rect.moveRight(target);  break;
        case eAlign::Top:    rect.moveTop(target);    break;
        case eAlign::Bottom: rect.moveBottom(target); break;
        }

        if (rect.topLeft() != box.rect.topLeft())
        {
            changed.append(qMakePair(box, rect));
        }
    }

    pushMoveChanges(mModel, changed, tr("Align selection"));
}

void SMDesign::distributeSelection(eDistribute axis)
{
    QList<SelectionBox> boxes{ collectSelectionBoxes(getScene()) };
    if (boxes.size() < 3)
    {
        return;
    }

    const bool horizontal = (axis == eDistribute::Horizontal);
    std::sort(boxes.begin(), boxes.end(), [horizontal](const SelectionBox& a, const SelectionBox& b) {
        return horizontal ? (a.rect.center().x() < b.rect.center().x()) : (a.rect.center().y() < b.rect.center().y());
    });

    const double firstCenter = horizontal ? boxes.first().rect.center().x() : boxes.first().rect.center().y();
    const double lastCenter  = horizontal ? boxes.last().rect.center().x()  : boxes.last().rect.center().y();
    const double step        = (lastCenter - firstCenter) / static_cast<double>(boxes.size() - 1);

    QList<QPair<SelectionBox, QRectF>> changed;
    for (int i = 1; i + 1 < boxes.size(); ++i)
    {
        const SelectionBox& box          = boxes.at(i);
        const double        targetCenter = firstCenter + step * static_cast<double>(i);
        QRectF               rect{ box.rect };
        if (horizontal)
        {
            rect.moveCenter(QPointF(targetCenter, box.rect.center().y()));
        }
        else
        {
            rect.moveCenter(QPointF(box.rect.center().x(), targetCenter));
        }

        if (rect.topLeft() != box.rect.topLeft())
        {
            changed.append(qMakePair(box, rect));
        }
    }

    pushMoveChanges(mModel, changed, tr("Distribute selection"));
}

void SMDesign::rebuildScene()
{
    // No level is shown until the manager settles on one; the auto-placement below must
    // not be mistaken for a viewport edit of the previously shown level.
    mShownLevel = 0u;

    if (mActToggleGrid != nullptr)
    {
        // A (re)loaded document brings its own grid settings.
        mActToggleGrid->setChecked(mModel.getData().getLayout().isGridVisible());
    }

    autoPlaceMissingNodes();
    mSceneManager->reset();
    populateStressContent();
}

void SMDesign::autoPlaceMissingNodes()
{
    const QList<SMLayoutNode> nodes{ SMAutoPlacer::missingNodes(mModel.getData()) };
    if (nodes.isEmpty() == false)
    {
        mModel.getUndoStack().push(new SMAutoPlaceNodesCommand(  mModel.getData(), mModel.getNotifier()
                                                               , nodes, tr("Auto-place elements")));
    }
}

void SMDesign::onLevelChanged(uint32_t levelId)
{
    mScene = mSceneManager->getCurrentScene();

    mRestoringView = true;
    mView->setScene(mScene);

    // The grid settings are per document; the checked actions are the session's source
    // of truth once they exist, the persisted layout before that (page construction).
    const SMLayoutData& layout = mModel.getData().getLayout();
    mScene->setGridSize(layout.getGridSize());
    mScene->setGridVisible(mActToggleGrid != nullptr ? mActToggleGrid->isChecked() : layout.isGridVisible());
    if (mActGridDots != nullptr)
    {
        mScene->setGridStyle(mActGridDots->isChecked() ? NESMDesign::eGridStyle::Dots : NESMDesign::eGridStyle::Lines);
    }

    {
        // The dot size is an app-level preference; a freshly created scene starts at the
        // default, so re-apply the stored value on every level switch.
        QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
        mScene->setGridDotSize(settings.value(QStringLiteral("smDesign/gridDotSize"), NESMDesign::GridDotSizeDefault).toInt());
    }

    if (mActToggleSnap != nullptr)
    {
        mScene->setSnapToGrid(mActToggleSnap->isChecked());
    }

    restoreViewport(levelId, true);
    mRestoringView = false;

    mShownLevel  = levelId;
    mViewGesture = SMMoveNodeCommand::takeNextGesture();
    rebuildBreadcrumb();
    updateNavActions();
}

void SMDesign::restoreViewport(uint32_t levelId, bool applyDefault)
{
    if (levelId == 0u)
    {
        return;
    }

    const bool guard = mRestoringView;
    mRestoringView = true;

    SMLayoutView* entry = mModel.getData().getLayout().findView(levelId);
    if (entry != nullptr)
    {
        mView->setZoom(entry->zoom);
        mView->centerOn(entry->x, entry->y);
    }
    else if (applyDefault)
    {
        mView->zoomReset();
        const QRectF bounds = (mScene != nullptr ? mScene->contentBounds() : QRectF());
        if (bounds.isValid() && (bounds.isEmpty() == false))
        {
            // Anchor the content's top-left near the viewport's top-left corner (a small
            // margin in) rather than centering it: a level with a single Start state must
            // show that state as the top-left entry point, not floating in the middle.
            const QRectF viewRect = mView->mapToScene(mView->viewport()->rect()).boundingRect();
            const double margin   = 48.0;
            mView->centerOn(  bounds.left() - margin + viewRect.width()  / 2.0
                            , bounds.top()  - margin + viewRect.height() / 2.0);
        }
        else
        {
            mView->centerOn(0.0, 0.0);
        }
    }

    mRestoringView = guard;
}

void SMDesign::onViewportChanged()
{
    if (mRestoringView || (mShownLevel == 0u) || (mScene == nullptr))
    {
        return;
    }

    const QPointF center = mView->mapToScene(mView->viewport()->rect().center());
    SMLayoutView value;
    value.owner = mShownLevel;
    value.zoom  = mView->getZoom();
    value.x     = center.x();
    value.y     = center.y();

    SMLayoutView* entry = mModel.getData().getLayout().findView(mShownLevel);
    if ((entry != nullptr) && (entry->zoom == value.zoom)
        && (std::abs(entry->x - value.x) < 1.0) && (std::abs(entry->y - value.y) < 1.0))
    {
        return;
    }

    mModel.getUndoStack().push(new SMSetViewCommand(  mModel.getData(), mModel.getNotifier()
                                                    , mShownLevel, mViewGesture, value
                                                    , tr("Change level view")));
}

void SMDesign::onModelLayoutChanged(const QList<uint32_t>& ownerIds)
{
    // Grid settings are document-wide (not owned by any element), so re-sync on every
    // layout change regardless of ownerIds - this is what makes undo/redo of a grid
    // change (and reopening the document) keep the canvas and the checked action correct.
    const SMLayoutData& layout = mModel.getData().getLayout();
    if ((mScene != nullptr) && (mScene->isGridVisible() != layout.isGridVisible()))
    {
        mScene->setGridVisible(layout.isGridVisible());
    }

    if ((mScene != nullptr) && (mScene->getGridSize() != layout.getGridSize()))
    {
        mScene->setGridSize(layout.getGridSize());
    }

    if ((mActToggleGrid != nullptr) && (mActToggleGrid->isChecked() != layout.isGridVisible()))
    {
        mSyncingGrid = true;
        mActToggleGrid->setChecked(layout.isGridVisible());
        mSyncingGrid = false;
    }

    if (mRestoringView || (mShownLevel == 0u) || (ownerIds.contains(mShownLevel) == false))
    {
        return;
    }

    // An undo/redo rewrote the displayed level's View entry: bring the viewport along.
    SMLayoutView* entry = mModel.getData().getLayout().findView(mShownLevel);
    if (entry == nullptr)
    {
        return;
    }

    const QPointF center = mView->mapToScene(mView->viewport()->rect().center());
    if ((entry->zoom != mView->getZoom())
        || (std::abs(entry->x - center.x()) >= 1.0) || (std::abs(entry->y - center.y()) >= 1.0))
    {
        restoreViewport(mShownLevel, false);
    }
}

void SMDesign::rebuildBreadcrumb()
{
    if (mBreadcrumbLayout == nullptr)
    {
        return;
    }

    while (QLayoutItem* item = mBreadcrumbLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    const QList<uint32_t> path{ mSceneManager->getCurrentPath() };
    for (int i = 0; i < path.size(); ++i)
    {
        const uint32_t levelId = path.at(i);
        const QString  title   = mSceneManager->levelTitle(levelId);
        if (i + 1 < path.size())
        {
            QToolButton* crumb = new QToolButton(mBreadcrumb);
            crumb->setText(title);
            crumb->setAutoRaise(true);
            crumb->setCursor(Qt::PointingHandCursor);
            crumb->setToolTip(tr("Go to level '%1'").arg(title));
            connect(crumb, &QToolButton::clicked, this, [this, levelId]() {
                mSceneManager->navigateTo(levelId);
            });

            mBreadcrumbLayout->addWidget(crumb);
            mBreadcrumbLayout->addWidget(new QLabel(QStringLiteral(">"), mBreadcrumb));
        }
        else
        {
            // The bold tail is the current-level indicator.
            QLabel* current = new QLabel(title, mBreadcrumb);
            QFont font{ current->font() };
            font.setBold(true);
            current->setFont(font);
            current->setToolTip(tr("Current level"));
            mBreadcrumbLayout->addWidget(current);
        }
    }

    mBreadcrumbLayout->addStretch(1);
}

void SMDesign::updateNavActions()
{
    if (mActGoToParent == nullptr)
    {
        return;
    }

    mActGoToParent->setEnabled(mSceneManager->getCurrentPath().size() > 1);

    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    const SMStateEntry* single = (selection.size() == 1 ? mModel.getData().findStateById(selection.first()) : nullptr);
    // Enter Submachine descends into an existing submachine, or (for a plain normal state)
    // creates one on the fly, so it is enabled for any non-imported normal/composite state.
    mActEnterSubmachine->setEnabled((single != nullptr)
                                    && (single->isImportedSubmachine() == false)
                                    && (single->hasNestedStates() || (single->getKind() == SMStateEntry::eStateKind::Normal)));
    mActAddSubstate->setEnabled((single != nullptr)
                                && (single->getKind() == SMStateEntry::eStateKind::Normal)
                                && (single->isImportedSubmachine() == false)
                                && (single->hasNestedStates() == false));

    // Internal transitions run operations without leaving the state; Final states have
    // no outgoing transitions at all, so they cannot carry one either.
    mActAddInternal->setEnabled((single != nullptr) && (single->getKind() != SMStateEntry::eStateKind::Final));

    // The transition actions apply to a single selected edge; priority moves need a
    // neighbour in the owner's document order to swap with.
    const SMTransitionEntry* transition = (selection.size() == 1 ? mModel.getData().findTransitionById(selection.first()) : nullptr);
    const SMStateEntry* owner = (transition != nullptr ? mModel.getData().findTransitionOwner(transition->getId()) : nullptr);
    const int index = (owner != nullptr ? owner->getTransitions().findIndex(transition->getId()) : -1);
    const int count = (owner != nullptr ? owner->getTransitions().getElementCount() : 0);
    mActSetStimulus->setEnabled(transition != nullptr);
    mActRaisePriority->setEnabled((index > 0));
    mActLowerPriority->setEnabled((index >= 0) && (index < count - 1));

    // Appearance: color swatches enable per selected kind; align/distribute need a
    // multi-selection of movable boxes (states and/or notes).
    const bool hasStates = (getScene().selectedStateItems().isEmpty() == false);
    const bool hasEdges  = (getScene().selectedEdgeItems().isEmpty() == false);
    const bool hasNotes  = (getScene().selectedNoteItems().isEmpty() == false);
    mActStateColor->setEnabled(hasStates);
    mActEdgeColor->setEnabled(hasEdges);
    mActNoteColor->setEnabled(hasNotes);
    // The single toolbar "Set Color" acts on any selected element; disabled when nothing is selected.
    mActSetColor->setEnabled(hasStates || hasEdges || hasNotes);

    const int boxCount = getScene().selectedStateItems().size() + getScene().selectedNoteItems().size();
    const bool canAlign = (boxCount > 1);
    const bool canDistribute = (boxCount > 2);
    mActAlignLeft->setEnabled(canAlign);
    mActAlignRight->setEnabled(canAlign);
    mActAlignTop->setEnabled(canAlign);
    mActAlignBottom->setEnabled(canAlign);
    mActDistributeH->setEnabled(canDistribute);
    mActDistributeV->setEnabled(canDistribute);
}

void SMDesign::addSubstateToSelection()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        return;
    }

    StateMachineData& data = mModel.getData();
    const uint32_t stateId = selection.first();
    const SMStateEntry* state = data.findStateById(stateId);
    if ((state == nullptr) || (state->getKind() != SMStateEntry::eStateKind::Normal)
        || state->isImportedSubmachine() || state->hasNestedStates())
    {
        return;
    }

    const QString base{ QStringLiteral("Start") };
    QString name{ base };
    for (int i = 2; data.findState(name) != nullptr; ++i)
    {
        name = base + QString::number(i);
    }

    const QRectF geometry{ 80.0, 140.0, NESMDesign::StateDefaultWidth, NESMDesign::StateDefaultHeight };
    SMConvertToCompositeCommand* command =
            new SMConvertToCompositeCommand(  data, mModel.getNotifier(), stateId, name, geometry
                                            , tr("Add substate to %1").arg(state->getName()));
    if (command->isEffective() == false)
    {
        delete command;
        return;
    }

    mModel.getUndoStack().push(command);
    mSceneManager->enterSubmachine(stateId);
}

void SMDesign::enterSelectedSubmachine()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        return;
    }

    const uint32_t id = selection.first();
    const SMStateEntry* state = mModel.getData().findStateById(id);
    if (state == nullptr)
    {
        return;
    }

    if (state->hasNestedStates())
    {
        mSceneManager->enterSubmachine(id);
    }
    else if ((state->getKind() == SMStateEntry::eStateKind::Normal) && (state->isImportedSubmachine() == false))
    {
        // The state has no submachine yet: create a painted composite (with its Start state)
        // and descend into it, so "Enter Submachine" doubles as "start designing one here".
        // (Follow-up: an auto-created submachine left with only its Start state should revert
        // to a plain state when the user leaves it; that needs an undoable revert command.)
        addSubstateToSelection();
    }
}

void SMDesign::centerMachine()
{
    if (mScene == nullptr)
    {
        return;
    }

    const QRectF bounds = mScene->contentBounds();
    if ((bounds.isValid() == false) || bounds.isEmpty())
    {
        mView->centerOn(0.0, 0.0);
        return;
    }

    const QRect   viewport = mView->viewport()->rect();
    const QRectF  viewRect = mView->mapToScene(viewport).boundingRect();
    if ((viewRect.width() >= bounds.width()) && (viewRect.height() >= bounds.height()))
    {
        // The whole diagram fits at the current zoom: center it.
        mView->centerOn(bounds.center());
        return;
    }

    // Too large to fit: anchor the diagram's top-left (the Start state region) about
    // 64 device pixels in from the viewport's top-left corner, keeping the zoom.
    const double marginX = 64.0 * viewRect.width()  / static_cast<double>(std::max(viewport.width(), 1));
    const double marginY = 64.0 * viewRect.height() / static_cast<double>(std::max(viewport.height(), 1));
    mView->centerOn(  bounds.left() - marginX + viewRect.width()  / 2.0
                    , bounds.top()  - marginY + viewRect.height() / 2.0);
}

void SMDesign::populateStressContent()
{
    bool valid{ false };
    int  count = qEnvironmentVariableIntValue("LUSAN_SM_CANVAS_STRESS", &valid);
    if (valid == false)
    {
        return;
    }

    count = (count > 0 ? count : 200);
    constexpr int    columns { 20 };
    constexpr double spacingX{ 180.0 };
    constexpr double spacingY{ 110.0 };

    for (int i = 0; i < count; ++i)
    {
        StressNodeItem* node = new StressNodeItem(StressIdBase + static_cast<uint32_t>(i), QString("S%1").arg(i + 1));
        node->setPos((i % columns) * spacingX, (i / columns) * spacingY);
        mScene->addItem(node);
    }
}

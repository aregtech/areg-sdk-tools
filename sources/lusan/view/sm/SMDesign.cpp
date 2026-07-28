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

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/sm/SMClipboard.hpp"
#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/DocElementCommands.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"
#include "lusan/model/sm/SMLayoutCommands.hpp"
#include "lusan/model/sm/SMPasteCommand.hpp"
#include "lusan/model/sm/SMStateCommands.hpp"
#include "lusan/model/sm/SMTransitionCommands.hpp"
#include "lusan/model/sm/SMGoToDef.hpp"
#include "lusan/model/sm/SMWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMAutoPlacer.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMEdgeItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMNoteItem.hpp"
#include "lusan/view/sm/SMScene.hpp"
#include "lusan/view/sm/SMSceneManager.hpp"
#include "lusan/view/sm/SMOperationsDialog.hpp"
#include "lusan/view/sm/SMOutlinePanel.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"
#include "lusan/view/sm/SMStateItem.hpp"
#include "lusan/view/sm/SMToolIcons.hpp"
#include "lusan/view/sm/SMWhereUsedMenu.hpp"

#include <QAction>
#include <QActionGroup>
#include <QClipboard>
#include <QColorDialog>
#include <QCursor>
#include <QGuiApplication>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPair>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QSize>
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
    : QMainWindow   (parent)
    , mModel        (model)
    , mView         (new SMGraphicsView(this))
    , mSceneManager (nullptr)
    , mScene        (nullptr)
    , mBreadcrumb   (nullptr)
    , mBreadcrumbLayout(nullptr)
    , mSearchEdit   (nullptr)
    , mSearchStatus (nullptr)
    , mSearchIndex  (-1)
    , mToolBar      (nullptr)
    , mPropertiesDock(nullptr)
    , mOutlineDock  (nullptr)
    , mProperties   (nullptr)
    , mOutline      (nullptr)
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
    , mActEdgeShape (nullptr)
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
    , mActAddSubmachine(nullptr)
    , mActRemoveSubmachine(nullptr)
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
    , mPlaceToolbar (1)     // eDesignPlace::InDesign
    , mPlaceProperties(1)   // eDesignPlace::InDesign
    , mPlaceOutline (1)     // eDesignPlace::InDesign
    , mSeedActive   (false)
    , mSeedTarget   (SMReferences::eTarget::State)
    , mSeedId       (0u)
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

    // The top bar carries the breadcrumb (left, cleared/rebuilt on level change) and the
    // canvas search box (right, persistent) so navigating large machines stays cheap.
    QWidget* topBar = new QWidget(this);
    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(0, 0, 8, 0);
    topLayout->setSpacing(4);
    topLayout->addWidget(mBreadcrumb, 1);

    mSearchEdit = new QLineEdit(topBar);
    mSearchEdit->setObjectName(QStringLiteral("smCanvasSearch"));
    mSearchEdit->setPlaceholderText(tr("Find state / transition (Ctrl+F)"));
    mSearchEdit->setClearButtonEnabled(true);
    mSearchEdit->setMaximumWidth(240);
    mSearchEdit->installEventFilter(this);
    topLayout->addWidget(mSearchEdit);

    // Search option toggles (match case, whole word, regular expression). Re-running the
    // search on toggle keeps the result live as the user tunes the query.
    auto makeSearchOption = [this, topBar, topLayout](const QIcon& icon, const QString& tip, const QString& name) -> QToolButton*
    {
        QToolButton* button = new QToolButton(topBar);
        button->setObjectName(name);
        button->setIcon(icon);
        button->setToolTip(tip);
        button->setCheckable(true);
        button->setAutoRaise(true);
        connect(button, &QToolButton::toggled, this, [this](bool) { onSearchTextChanged(); });
        topLayout->addWidget(button);
        return button;
    };
    mSearchCase  = makeSearchOption(NELusanCommon::iconSearchMatchCase(), tr("Match case"), QStringLiteral("smCanvasSearchCase"));
    mSearchWord  = makeSearchOption(NELusanCommon::iconSearchMatchWord(), tr("Match whole word"), QStringLiteral("smCanvasSearchWord"));
    mSearchRegex = makeSearchOption(NELusanCommon::iconSearchWildCard(), tr("Regular expression"), QStringLiteral("smCanvasSearchRegex"));

    mSearchStatus = new QLabel(topBar);
    mSearchStatus->setObjectName(QStringLiteral("smCanvasSearchStatus"));
    mSearchStatus->setMinimumWidth(72);
    topLayout->addWidget(mSearchStatus);

    // The Design page is a QMainWindow: its central widget is the canvas (breadcrumb + viewport),
    // and its own drawing toolbar, Properties, and Outline panels dock to the page's edges
    // (issue #516). They live inside this page and can be moved to the Navigation Window; they are
    // only present while this Design page is the shown tab of the active document.
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(topBar);
    layout->addWidget(mView);
    setCentralWidget(central);

    connect(mSearchEdit, &QLineEdit::textChanged, this, &SMDesign::onSearchTextChanged);
    connect(mSearchEdit, &QLineEdit::returnPressed, this, &SMDesign::advanceSearch);
    // No local Ctrl+F shortcut here: the main window's Edit > Find owns Ctrl+F and calls
    // beginSearch(), so the two never collide into an ambiguous-shortcut no-op (issue #538).

    connect(mSceneManager, &SMSceneManager::signalLevelChanged, this, &SMDesign::onLevelChanged);
    connect(mSceneManager, &SMSceneManager::signalToolChanged, this, &SMDesign::onToolChanged);
    connect(mSceneManager, &SMSceneManager::signalRequestSubstate, this, [this](uint32_t stateId)
    {
        // Body double-click on a state: descend into its submachine, creating one on the fly for a
        // plain normal state (the same create-or-enter path as the Enter Submachine action). Make
        // the double-clicked state the selection so the shared logic acts on it.
        mModel.getSelectionModel().setSelection(QList<uint32_t>{ stateId });
        enterSelectedSubmachine();
    });
    connect(mSceneManager, &SMSceneManager::signalGuardEditRequested, this, [this](uint32_t transitionId)
    {
        // Edge-label double-click: surface the Properties panel and focus the guard field.
        if (mProperties == nullptr)
        {
            return;
        }

        if ((mPropertiesDock != nullptr) && (mPropertiesDock->widget() == mProperties) && (mPropertiesDock->isVisible() == false))
        {
            mPropertiesDock->show();
            mPropertiesDock->raise();
        }

        mProperties->focusConditions(transitionId);
    });
    connect(mSceneManager, &SMSceneManager::signalGotoDefinitionRequested, this, [this](uint32_t elementId, bool isState, int scope)
    {
        // Ctrl+Shift link on a state/transition: go to the declaration(s) the clicked part
        // references. The picker (when several) opens at the pointer, right where the user clicked.
        gotoDefinitionFor(elementId, isState, QCursor::pos(), scope);
    });
    connect(mSceneManager, &SMSceneManager::signalGotoRefsRequested, this, [this](const QList<SMReferences::Ref>& refs)
    {
        // Ctrl+Shift link on one state-body operation row: navigate to what that row references.
        gotoDefinitionForRefs(refs, QCursor::pos());
    });
    connect(mView->horizontalScrollBar(), &QScrollBar::valueChanged, this, &SMDesign::onViewportChanged);
    connect(mView->verticalScrollBar(), &QScrollBar::valueChanged, this, &SMDesign::onViewportChanged);
    connect(mView, &SMGraphicsView::signalZoomChanged, this, &SMDesign::onViewportChanged);

    rebuildScene();
    setupActions();
    buildDesignToolbar();   // the toolbar reads the actions created by setupActions()
    buildDesignPanels();
    mView->installEventFilter(this);

    // Context-sensitive canvas/state/transition/note menus, built on
    // demand from the same action set the toolbar and Design menu reuse. A QGraphicsView
    // routes context-menu events through contextMenuEvent(), so a Qt::CustomContextMenu
    // policy never fires customContextMenuRequested; the view's own signal (emitted from its
    // contextMenuEvent override, viewport coordinates) is the reliable hook.
    connect(mView, &SMGraphicsView::signalContextMenuRequested, this, &SMDesign::onViewContextMenuRequested);

    // A read-only view still navigates, zooms and reads; it just cannot author. The undo stack
    // is the guarantee -- this only stops the drawing tools from pretending otherwise.
    if (mToolBar != nullptr)
    {
        mToolBar->setEnabled(mModel.isReadOnly() == false);
    }

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
    // Adding or removing a state changes whether the current level has a transition target, so
    // the Add Transition tool must re-evaluate its enabled state on add/remove too - not only on
    // selection change (issue #516 bug 2; also covers undo/redo of a state create/delete).
    connect(&notifier, &DocModelNotifier::elementAdded, this, [this](uint32_t, eDocElementKind kind) {
        if (kind == eDocElementKind::State)
        {
            updateNavActions();
        }
    });
    connect(&notifier, &DocModelNotifier::elementRemoved, this, [this](uint32_t, eDocElementKind kind) {
        if (kind == eDocElementKind::State)
        {
            updateNavActions();
        }
    });
    connect(&mModel.getSelectionModel(), &SMSelectionModel::signalSelectionChanged, this, [this](const QList<uint32_t>&) {
        updateNavActions();
    });

    updateNavActions();
}

SMDesign::~SMDesign()
{
    // Child destruction (the view detaching its scene changes the scrollbars) can emit signals
    // wired to this page's slots; stop receiving them before the derived object is gone.
    mModel.getNotifier().disconnect(this);
    mModel.getSelectionModel().disconnect(this);
    mSceneManager->disconnect(this);
    mView->disconnect(this);
    mView->horizontalScrollBar()->disconnect(this);
    mView->verticalScrollBar()->disconnect(this);
}

bool SMDesign::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::Close) && ((watched == mPropertiesDock) || (watched == mOutlineDock)))
    {
        // The dock's own close button hides the widget but knows nothing about the placement the
        // main window persists and re-applies on every activation -- without this the panel came
        // back on the next repaint. Route the close through the same channel as the View menus.
        const int widget = (watched == mPropertiesDock) ? 1 : 2;
        emit signalPlaceDesignWidget(widget, 0);
    }

    if (watched == mSearchEdit)
    {
        // Esc abandons the search and hands focus back to the canvas.
        if ((event->type() == QEvent::KeyPress) && (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape))
        {
            mSearchEdit->clear();   // clears the match cache and status via textChanged
            mView->setFocus();
            return true;
        }

        return QMainWindow::eventFilter(watched, event);
    }

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
            // Accept the override for any key that maps to a tool action so Qt's own
            // WidgetWithChildrenShortcut actions never fire on their own; the matching
            // KeyPress below is what actually dispatches (or, while editing, is left alone).
            if (matchAction(*static_cast<QKeyEvent*>(event)) != nullptr)
            {
                event->accept();
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            // A proxy-backed inline editor (state rename / note edit) owns the whole key
            // stream while it is open. The single-key tool shortcuts (S, F, T, N, Backspace,
            // Delete, ...) must not steal a keystroke destined for that editor -- doing so
            // both blocked editing keys (Backspace/Delete) and spawned stray items (S/F/T/N).
            // While an inline editor is active, never dispatch a tool action: let the key fall
            // through the view to the scene's focused proxy editor.
            if (getScene().isInlineEditorActive() == false)
            {
                QAction* action = matchAction(*static_cast<QKeyEvent*>(event));
                if (action != nullptr)
                {
                    action->trigger();
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

QMenu* SMDesign::createPopupMenu(void)
{
    return nullptr;
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
    // only the grid size and visibility).
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

    // The dot diameter is likewise an application-level display preference (keeps
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

    // The placement actions are checkable: the checked one is the armed tool, which is what
    // makes the picked tool visible on the toolbar, in the Design menu, and in the context
    // menu at once (they all share these action objects, issue #541). Unchecking disarms.
    // onToolChanged() is the only place that writes the checked state back.
    auto placementAction = [this](const QString& text, const QKeySequence& shortcut, NESMDesign::eCanvasTool tool) -> QAction*
    {
        QAction* action = new QAction(text, this);
        action->setShortcut(shortcut);
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [this, tool](bool checked) {
            getScene().setActiveTool(checked ? tool : NESMDesign::eCanvasTool::Select);
        });

        return action;
    };

    mActAddState = placementAction(tr("Add State"), QKeySequence(Qt::Key_S), NESMDesign::eCanvasTool::AddState);

    mActAddFinal = placementAction(tr("Add Final State"), QKeySequence(Qt::Key_F)
                                   , NESMDesign::eCanvasTool::AddFinalState);

    mActAddTransition = placementAction(tr("Add Transition"), QKeySequence(Qt::Key_T)
                                        , NESMDesign::eCanvasTool::AddTransition);

    mActAddNote = placementAction(tr("Add Note"), QKeySequence(Qt::Key_N), NESMDesign::eCanvasTool::AddNote);

    mActStateColor = new QAction(tr("State Color..."), this);
    connect(mActStateColor, &QAction::triggered, this, [this]() { applyColorToSelection(eColorTarget::State); });

    mActEdgeColor = new QAction(tr("Transition Color..."), this);
    connect(mActEdgeColor, &QAction::triggered, this, [this]() { applyColorToSelection(eColorTarget::Edge); });
    mActEdgeShape = new QAction(tr("Make Arc"), this);
    connect(mActEdgeShape, &QAction::triggered, this, &SMDesign::onToggleEdgeShape);

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

    mActAddSubmachine = new QAction(tr("Add Submachine..."), this);
    connect(mActAddSubmachine, &QAction::triggered, this, &SMDesign::addSubmachineToSelection);

    mActRemoveSubmachine = new QAction(tr("Remove Submachine"), this);
    connect(mActRemoveSubmachine, &QAction::triggered, this, &SMDesign::removeSubmachineFromSelection);

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
    mActAddSubmachine->setIcon(QIcon(QStringLiteral(":/icons/entry add")));
    mActRemoveSubmachine->setIcon(QIcon(QStringLiteral(":/icons/entry delete")));
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
                                 , mActAddSubmachine, mActRemoveSubmachine
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
    mToolbarVisible = visible;
    if (mToolBar != nullptr)
    {
        mToolBar->setVisible(visible);
    }
}

bool SMDesign::isToolbarVisible() const
{
    return (mToolBar != nullptr) ? mToolBar->isVisible() : mToolbarVisible;
}

void SMDesign::setToolbarStyle(Qt::ToolButtonStyle style)
{
    if (mToolBar != nullptr)
    {
        mToolBar->setToolButtonStyle(style);
    }
}

void SMDesign::setPropertiesVisible(bool visible)
{
    if (mPropertiesDock != nullptr)
    {
        mPropertiesDock->setVisible(visible);
    }
}

bool SMDesign::isPropertiesVisible() const
{
    return (mPropertiesDock != nullptr) && mPropertiesDock->isVisible();
}

void SMDesign::setOutlineVisible(bool visible)
{
    if (mOutlineDock != nullptr)
    {
        mOutlineDock->setVisible(visible);
    }
}

bool SMDesign::isOutlineVisible() const
{
    return (mOutlineDock != nullptr) && mOutlineDock->isVisible();
}

void SMDesign::setPlacementState(int toolbar, int properties, int outline)
{
    mPlaceToolbar    = toolbar;
    mPlaceProperties = properties;
    mPlaceOutline    = outline;
}

void SMDesign::buildDesignToolbar()
{
    mToolBar = new QToolBar(tr("Design Tools"), this);
    mToolBar->setObjectName(QStringLiteral("SMDesignToolBar"));
    mToolBar->setMovable(true);
    mToolBar->setFloatable(false);
    mToolBar->setAllowedAreas(Qt::AllToolBarAreas);
    mToolBar->setIconSize(QSize(16, 16));

    // Icon-only by default (spec issue #516); the View menu's Toolbutton Mode submenu can switch
    // this and the choice is persisted, so seed the toolbar from the stored style.
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const Qt::ToolButtonStyle style = static_cast<Qt::ToolButtonStyle>(
            settings.value(QStringLiteral("smDesign/toolbarStyle"), static_cast<int>(Qt::ToolButtonIconOnly)).toInt());
    mToolBar->setToolButtonStyle(style);

    // One button per action of toolGroups(), a separator between groups. The actions are this
    // page's own (added in setupActions()), so they act on this canvas and enable/disable with it.
    bool firstGroup = true;
    for (const ToolGroup& group : toolGroups())
    {
        if (group.actions.isEmpty())
        {
            continue;
        }

        if (firstGroup == false)
        {
            mToolBar->addSeparator();
        }

        firstGroup = false;
        for (QAction* action : group.actions)
        {
            mToolBar->addAction(action);
        }
    }

    addToolBar(Qt::TopToolBarArea, mToolBar);
    mToolBar->setVisible(mToolbarVisible);
}

void SMDesign::buildDesignPanels()
{
    // Properties on the right, top; Outline on the right, below it. Both bound to this page's
    // model/scene manager (issue #516) and dockable to any of the page's four edges.
    mProperties = new SMPropertiesPanel(mModel);
    mProperties->bindSubmachineActions(mActEnterSubmachine, mActGoToParent, mActAddSubmachine, mActRemoveSubmachine);
    // A Ctrl+Shift click on a referenced symbol in the Conditions guard field navigates to its
    // declaration page, the same channel the canvas links use (StateMachine switches pages).
    connect(mProperties, &SMPropertiesPanel::signalNavigateToDefinition, this, &SMDesign::signalNavigateToDefinition);
    mPropertiesDock = new QDockWidget(tr("Properties"), this);
    mPropertiesDock->setObjectName(QStringLiteral("SMPropertiesDock"));
    mPropertiesDock->setWidget(mProperties);
    // Left or right only: both panels are tall lists of fields, and a top/bottom strip of the
    // canvas cannot hold one at a usable height.
    mPropertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mPropertiesDock->installEventFilter(this);
    addDockWidget(Qt::RightDockWidgetArea, mPropertiesDock);

    mOutline = new SMOutlinePanel(mModel, *mSceneManager);
    // The outline's context-menu Rename/Delete delegate to this page's own actions.
    connect(mOutline, &SMOutlinePanel::signalRenameRequested, this, [this](uint32_t) {
        if (mActRename != nullptr) mActRename->trigger();
    });
    connect(mOutline, &SMOutlinePanel::signalDeleteRequested, this, [this]() {
        deleteSelection();
    });
    mOutlineDock = new QDockWidget(tr("Outline"), this);
    mOutlineDock->setObjectName(QStringLiteral("SMOutlineDock"));
    mOutlineDock->setWidget(mOutline);
    mOutlineDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mOutlineDock->installEventFilter(this);
    addDockWidget(Qt::RightDockWidgetArea, mOutlineDock);

    splitDockWidget(mPropertiesDock, mOutlineDock, Qt::Vertical);
    // Settle the width now, once. Until a dock is given one it keeps following what its widget
    // asks for, so the first state or transition put into the panel would move the canvas edge --
    // the selection deciding how much room the drawing gets. After this, only a drag does.
    resizeDocks(QList<QDockWidget*>{ mPropertiesDock }, QList<int>{ NESMDesign::PanelDefaultWidth }, Qt::Horizontal);

    // F8 / Shift+F8 step through the findings (spec 9.1). The findings themselves live in the
    // output window's Validation tab, so the page asks for it and the window brings it forward
    // -- otherwise the canvas jumps to a finding with nothing on screen saying why.
    QShortcut* nextIssue = new QShortcut(QKeySequence(Qt::Key_F8), this);
    QShortcut* prevIssue = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F8), this);
    connect(nextIssue, &QShortcut::activated, this, [this]() { emit signalShowValidation(1); });
    connect(prevIssue, &QShortcut::activated, this, [this]() { emit signalShowValidation(-1); });
}

QList<SMDesign::ToolGroup> SMDesign::toolGroups() const
{
    // Every tool's tooltip shows its shortcut alongside the label. Refresh here so
    // the tooltip is present whichever way the toolbar is (re)built.
    for (QAction* action : actions())
    {
        if (action->shortcut().isEmpty() == false)
        {
            action->setToolTip(QStringLiteral("%1 (%2)").arg(action->text(), action->shortcut().toString(QKeySequence::NativeText)));
        }
    }

    // Ordered by importance: the design/placement operations first,
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

void SMDesign::populateDesignMenu(QMenu& menu)
{
    menu.addAction(mActAddState);
    menu.addAction(mActAddFinal);
    menu.addAction(mActAddTransition);
    menu.addAction(mActAddNote);
    menu.addSeparator();
    menu.addAction(mActStateColor);
    menu.addAction(mActEdgeColor);
    menu.addAction(mActNoteColor);
    // Reachable without right-clicking the transition; disabled unless exactly one is selected.
    menu.addAction(shapeToggleAction(selectedEdge()));
    menu.addSeparator();
    menu.addAction(mActAlignLeft);
    menu.addAction(mActAlignRight);
    menu.addAction(mActAlignTop);
    menu.addAction(mActAlignBottom);
    menu.addAction(mActDistributeH);
    menu.addAction(mActDistributeV);
    menu.addSeparator();
    menu.addAction(mActToggleGrid);
    menu.addAction(mActGridDots);
    menu.addAction(mActGridDotSize);
    menu.addAction(mActToggleSnap);
    menu.addAction(mActGridSize);
    menu.addSeparator();
    QMenu* declareMenu = menu.addMenu(tr("Add &Declaration"));
    declareMenu->addActions(declareActions());
    menu.addSeparator();
    menu.addAction(mActAddSubstate);
    menu.addAction(mActEnterSubmachine);
    // Reachable without right-clicking the state; dead unless exactly one composite is selected.
    // The shared selection model, not the scene's item selection: a selection made from the
    // outline, a search hit or the validation panel never touches the scene of a level that is
    // not on screen, and the menu must agree with the Properties panel about what is selected.
    const QList<uint32_t>& stateSelection = mModel.getSelectionModel().getSelection();
    addHistoryMenu(menu, stateSelection.size() == 1 ? stateSelection.first() : 0u);
    menu.addAction(mActGoToParent);
    menu.addAction(mActCenterMachine);
    menu.addSeparator();
    menu.addAction(mActZoomIn);
    menu.addAction(mActZoomOut);
    menu.addAction(mActZoomReset);
    menu.addAction(mActZoomFit);
    menu.addSeparator();
    menu.addAction(mActSelectAll);
    menu.addAction(mActRename);
    menu.addAction(mActDelete);
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
        menu.addAction(mActAddSubmachine);
        menu.addAction(mActRemoveSubmachine);
        addHistoryMenu(menu, state->getElementId());
        menu.addSeparator();
        menu.addAction(mActCut);
        menu.addAction(mActCopy);
        menu.addAction(mActDuplicate);
        menu.addSeparator();
        menu.addAction(mActStateColor);
        addNoteMenuEntries(menu, state->getElementId(), true);
        menu.addSeparator();
        const uint32_t stateId = state->getElementId();
        connect(menu.addAction(tr("Enter Actions...")), &QAction::triggered, this, [this, stateId]() { openStateOperationsDialog(stateId, true); });
        connect(menu.addAction(tr("Exit Actions...")), &QAction::triggered, this, [this, stateId]() { openStateOperationsDialog(stateId, false); });
        addGotoDeclarationMenu(menu, state->getElementId(), true);
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
        const uint32_t transitionId = edge->getElementId();
        connect(menu.addAction(tr("Operations...")), &QAction::triggered, this, [this, transitionId]() { openTransitionOperationsDialog(transitionId); });
        menu.addAction(mActRaisePriority);
        menu.addAction(mActLowerPriority);
        menu.addSeparator();
        addEdgeShapeMenu(menu, *edge);
        menu.addAction(mActEdgeColor);
        addNoteMenuEntries(menu, edge->getElementId(), false);
        addGotoDeclarationMenu(menu, edge->getElementId(), false);
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
        // The shared command group: present in every element context so the four
        // core actions stay reachable (the canvas menu above already leads with them).
        menu.addSeparator();
        menu.addAction(mActAddState);
        menu.addAction(mActAddTransition);
        menu.addAction(mActUndo);
        menu.addAction(mActRedo);
    }

    menu.addSeparator();
    QMenu* viewMenu = menu.addMenu(tr("View"));
    // viewMenu->setIcon(NELusanCommon::iconViewFsmDesign(NELusanCommon::SizeSmall));

    const auto addPair = [this, viewMenu](const QString& designText, const QString& naviText, int widget, int place)
    {
        // Unchecking the current home hides the widget, matching the View menu; without it the
        // context menu could only move a widget between its two homes, never put it away.
        QAction* inDesign = viewMenu->addAction(designText);
        inDesign->setCheckable(true);
        inDesign->setChecked(place == 1);
        connect(inDesign, &QAction::triggered, this, [this, widget](bool on) { emit signalPlaceDesignWidget(widget, on ? 1 : 0); });

        QAction* inNavi = viewMenu->addAction(naviText);
        inNavi->setCheckable(true);
        inNavi->setChecked(place == 2);
        connect(inNavi, &QAction::triggered, this, [this, widget](bool on) { emit signalPlaceDesignWidget(widget, on ? 2 : 0); });
    };

    addPair(tr("Show Toolbar in Design"), tr("Show Toolbar in Navigation"), 0, mPlaceToolbar);
    viewMenu->addSeparator();
    addPair(tr("Show Properties in Design"), tr("Show Properties in Navigation"), 1, mPlaceProperties);
    viewMenu->addSeparator();
    addPair(tr("Show Outline in Design"), tr("Show Outline in Navigation"), 2, mPlaceOutline);

    viewMenu->addSeparator();
    connect(viewMenu->addAction(tr("Show Validation Results")), &QAction::triggered
          , this, [this]() { emit signalShowValidation(0); });

    // Refresh the shared actions' enabled state against the (possibly just changed) selection
    // and current level so entries like Add Transition / Add Internal Transition open correctly
    // enabled or greyed (issue #516 bugs 2 and 5).
    updateNavActions();

    menu.exec(mView->viewport()->mapToGlobal(pos));
}

void SMDesign::openStateOperationsDialog(uint32_t stateId, bool entry)
{
    SMStateEntry* state = mModel.getData().findStateById(stateId);
    if (state == nullptr)
    {
        return;
    }

    SMOperationList& list = entry ? state->getEntryList() : state->getExitList();
    const QString title = (entry ? tr("On Enter: %1") : tr("On Exit: %1")).arg(state->getName());
    SMOperationsDialog dialog(mModel, title, stateId, eDocElementKind::State, 0u, state, &list, this);
    dialog.exec();
}

void SMDesign::openTransitionOperationsDialog(uint32_t transitionId)
{
    SMTransitionEntry* transition = mModel.getData().findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return;
    }

    const QString stim = transition->getStimulus().isEmpty() ? tr("transition") : transition->getStimulus();
    // The transition is its own Param scope: a transition operation may map stimulus params.
    SMOperationsDialog dialog(mModel, tr("Operations: %1").arg(stim), transitionId, eDocElementKind::Transition, transitionId, transition, &transition->getOperations(), this);
    dialog.exec();
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

    for (const SMDocumentIndex::Stimulus& stimulus : SMDocumentIndex(data).stimuli())
    {
        add(stimulus.kind, stimulus.name);
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
                                                             , 0u, QList<QPointF>()
                                                             , tr("Add internal transition to %1").arg(state->getName())));
}

void SMDesign::addEdgeShapeMenu(QMenu& menu, SMEdgeItem& edge)
{
    // ONE flat entry that names what it will do, not a submenu of shapes to compare: the shape a
    // transition already has is visible on the canvas, so the only thing worth saying is the other
    // one. A nested "Shape >" was shipped first and went unfound.
    menu.addAction(shapeToggleAction(&edge));
}

QAction* SMDesign::shapeToggleAction(SMEdgeItem* edge)
{
    const bool isArc = (edge != nullptr) && (edge->getShape() == SMLayoutEdge::eShape::Arc);
    mActEdgeShape->setText(isArc ? tr("Make Polyline") : tr("Make Arc"));
    // Every transition can take either shape, a self-loop included: the only thing that disables
    // the entry is having no single transition to apply it to.
    mActEdgeShape->setEnabled(edge != nullptr);
    mActEdgeShape->setData(edge != nullptr ? edge->getElementId() : 0u);
    return mActEdgeShape;
}

SMEdgeItem* SMDesign::selectedEdge() const
{
    const QList<uint32_t> selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        return nullptr;     // "the shape of which one?" has no answer for a multi-selection
    }

    return dynamic_cast<SMEdgeItem*>(getScene().findCanvasItem(selection.first()));
}

void SMDesign::onToggleEdgeShape()
{
    // Re-resolve from the id the menu was built with: the action outlives the press, and a
    // rebuild between the two would leave a dangling item pointer.
    const uint32_t edgeId = mActEdgeShape->data().toUInt();
    SMEdgeItem* edge = (edgeId != 0u) ? dynamic_cast<SMEdgeItem*>(getScene().findCanvasItem(edgeId)) : selectedEdge();
    if (edge != nullptr)
    {
        edge->setShape(edge->getShape() == SMLayoutEdge::eShape::Arc
                        ? SMLayoutEdge::eShape::Line
                        : SMLayoutEdge::eShape::Arc);
    }
}

void SMDesign::addHistoryMenu(QMenu& menu, uint32_t stateId)
{
    const SMStateEntry* state = (stateId != 0u) ? mModel.getData().findStateById(stateId) : nullptr;
    const bool composite = (state != nullptr) && state->isComposite();

    // A greyed entry with no reason is a dead end, and menu tooltips are off everywhere in this
    // app, so the reason goes into the title where it is always readable.
    QMenu* history = menu.addMenu(composite ? tr("History") : tr("History (needs a submachine)"));
    history->setEnabled(composite);
    history->setToolTipsVisible(true);
    if (state == nullptr)
    {
        return;             // present but dead, never a silent no-op on a guess
    }

    QActionGroup* group = new QActionGroup(history);
    group->setExclusive(true);

    const struct { SMStateEntry::eHistory mode; QString label; QString hint; } modes[] =
    {
          { SMStateEntry::eHistory::None,    tr("None"),    tr("Coming back always starts from the Start state again") }
        , { SMStateEntry::eHistory::Shallow, tr("Shallow"), tr("Coming back activates the substate that was active last time") }
        , { SMStateEntry::eHistory::Deep,    tr("Deep"),    tr("Coming back restores the whole path that was active last time, down to the leaf") }
    };

    for (const auto& entry : modes)
    {
        QAction* action = history->addAction(entry.label);
        action->setCheckable(true);
        action->setChecked(state->getHistory() == entry.mode);
        action->setToolTip(entry.hint);
        group->addAction(action);

        const SMStateEntry::eHistory mode = entry.mode;
        connect(action, &QAction::triggered, this, [this, stateId, mode]()
        {
            const SMStateEntry* target = mModel.getData().findStateById(stateId);
            if ((target != nullptr) && target->isComposite() && (target->getHistory() != mode))
            {
                mModel.getUndoStack().push(new SMSetHistoryCommand(mModel.getData(), mModel.getNotifier(), stateId, mode, tr("Set history mode")));
            }
        });
    }
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

    // A scene keeps its own tool, and a freshly created one starts on Select; re-sync so the
    // checked action and the cursor describe the level that is now shown.
    onToolChanged(mScene->getActiveTool());
}

void SMDesign::onToolChanged(NESMDesign::eCanvasTool tool)
{
    const auto sync = [tool](QAction* action, NESMDesign::eCanvasTool owned)
    {
        if (action != nullptr)
        {
            action->setChecked(tool == owned);
        }
    };

    sync(mActAddState     , NESMDesign::eCanvasTool::AddState);
    sync(mActAddFinal     , NESMDesign::eCanvasTool::AddFinalState);
    sync(mActAddTransition, NESMDesign::eCanvasTool::AddTransition);
    sync(mActAddNote      , NESMDesign::eCanvasTool::AddNote);

    if (mView != nullptr)
    {
        mView->setToolCursor(tool);
    }
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

    // A document opened as somebody's import starts its path in that host, so the breadcrumb says
    // where the crossing happened; without it a read-only window looks like an ordinary one.
    const QString origin = mModel.getReadOnlyOrigin();
    if (origin.isEmpty() == false)
    {
        QLabel* crossing = new QLabel(origin, mBreadcrumb);
        QFont font{ crossing->font() };
        font.setItalic(true);
        crossing->setFont(font);
        crossing->setToolTip(tr("Opened read only from this machine"));
        mBreadcrumbLayout->addWidget(crossing);
        mBreadcrumbLayout->addWidget(new QLabel(QStringLiteral(">"), mBreadcrumb));
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

//////////////////////////////////////////////////////////////////////////
// Canvas search / go-to
//////////////////////////////////////////////////////////////////////////

void SMDesign::beginSearch()
{
    // Plain Find keeps whatever free-text query the box already had; drop any entry seed so
    // the next edit scans by name/ID again.
    mSeedActive = false;
    mSearchEdit->setFocus();
    mSearchEdit->selectAll();
}

void SMDesign::beginSearch(const QString& text, SMReferences::eTarget target, uint32_t id)
{
    mSeedActive = true;
    mSeedTarget = target;
    mSeedId     = id;
    mSeedName   = text;

    // Setting the text drives onSearchTextChanged, which lists the seeded entry's usages while
    // the seed is active; if the text is unchanged, no signal fires, so recompute directly.
    if (mSearchEdit->text() == text)
        onSearchTextChanged();
    else
        mSearchEdit->setText(text);

    mSearchEdit->setFocus();
    mSearchEdit->selectAll();
}

void SMDesign::revealReference(uint32_t elementId, bool isState)
{
    navigateToIssue(elementId, isState ? eDocElementKind::State : eDocElementKind::Transition);
}

void SMDesign::whereUsedForSelection()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        QMessageBox::information(this, tr("Where used"), tr("Select a single state to list where it is used."));
        return;
    }

    SMStateEntry* state = mModel.getData().findStateById(selection.first());
    if (state == nullptr)
    {
        // A transition or note is selected: where-used lists references to a named target, so
        // it applies to states (and to the registry entries on the other pages), not edges.
        QMessageBox::information(this, tr("Where used"), tr("Where used applies to a state or a registry entry."));
        return;
    }

    const QList<SMReferences::Use> uses = SMWhereUsed::collect(mModel.getData(), SMReferences::eTarget::State, state->getName(), state->getId());
    SMWhereUsedMenu::present(this, uses, mModel.getSelectionModel(), state->getName());
}

void SMDesign::gotoDefinitionForSelection()
{
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    if (selection.size() != 1)
    {
        QMessageBox::information(this, tr("Go to Declaration"),
            tr("Select a single state or transition to go to the declarations it uses."));
        return;
    }

    const uint32_t id = selection.first();
    const bool isState = (mModel.getData().findStateById(id) != nullptr);
    if ((isState == false) && (mModel.getData().findTransitionById(id) == nullptr))
    {
        // A note (or nothing navigable) is selected: go-to-declaration applies to states and
        // transitions, whose stimulus / operations / guards reference registry declarations.
        QMessageBox::information(this, tr("Go to Declaration"),
            tr("Select a state or transition to go to the declarations it uses."));
        return;
    }

    // The picker (for several targets) opens at the pointer, matching where the user is looking.
    gotoDefinitionFor(id, isState, QCursor::pos());
}

void SMDesign::gotoDefinitionFor(uint32_t elementId, bool isState, const QPoint& globalPos, int scope)
{
    // A Ctrl+Shift click on the stimulus part of an edge label: the stimulus is a single
    // declaration (the trigger / event / timer that fires the transition), so jump straight to it.
    if (scope == SMScene::GotoStimulus)
    {
        const SMGoToDef::Target stim = SMGoToDef::stimulusOf(mModel.getData(), elementId);
        if (stim.declId == 0u)
        {
            QMessageBox::information(this, tr("Go to Declaration"),
                tr("This transition's stimulus does not reference a declaration."));
            return;
        }

        emit signalNavigateToDefinition(stim.kind, stim.declId);
        return;
    }

    QList<SMGoToDef::Target> targets = SMGoToDef::collect(mModel.getData(), elementId, isState);

    // A Ctrl+Shift click on the action part: keep only the operations (action calls, sent events,
    // started/stopped timers), never the stimulus (which can share an event/timer with an
    // operation) and never the guard symbols (those are reached through the guard editor).
    if (scope == SMScene::GotoAction)
    {
        const SMGoToDef::Target stim = SMGoToDef::stimulusOf(mModel.getData(), elementId);
        QList<SMGoToDef::Target> operations;
        for (const SMGoToDef::Target& target : targets)
        {
            const bool isOperation = (target.kind == SMReferences::eTarget::Action)
                                  || (target.kind == SMReferences::eTarget::Event)
                                  || (target.kind == SMReferences::eTarget::Timer);
            const bool isStimulus = (target.kind == stim.kind) && (target.declId == stim.declId);
            if (isOperation && (isStimulus == false))
            {
                operations.append(target);
            }
        }

        targets = operations;
    }

    navigateTargets(targets, globalPos);
}

void SMDesign::gotoDefinitionForRefs(const QList<SMReferences::Ref>& refs, const QPoint& globalPos)
{
    navigateTargets(SMGoToDef::resolveRefs(mModel.getData(), refs), globalPos);
}

void SMDesign::navigateTargets(const QList<SMGoToDef::Target>& targets, const QPoint& globalPos)
{
    if (targets.isEmpty())
    {
        QMessageBox::information(this, tr("Go to Declaration"),
            tr("This element does not reference any declaration."));
        return;
    }

    if (targets.size() == 1)
    {
        // Exactly one referenced declaration: no guessing is needed, jump straight to it.
        emit signalNavigateToDefinition(targets.first().kind, targets.first().declId);
        return;
    }

    // Several referenced declarations: the user picks, so the app never guesses which to open.
    QMenu menu(this);
    for (const SMGoToDef::Target& target : targets)
    {
        QAction* action = menu.addAction(tr("%1  (%2)").arg(target.name, SMGoToDef::kindWord(target.kind)));
        const SMReferences::eTarget kind = target.kind;
        const uint32_t declId = target.declId;
        connect(action, &QAction::triggered, this, [this, kind, declId]() { emit signalNavigateToDefinition(kind, declId); });
    }
    menu.exec(globalPos);
}

void SMDesign::addGotoDeclarationMenu(QMenu& menu, uint32_t elementId, bool isState)
{
    const QList<SMGoToDef::Target> targets = SMGoToDef::collect(mModel.getData(), elementId, isState);
    if (targets.isEmpty())
        return;

    menu.addSeparator();
    if (targets.size() == 1)
    {
        // One target: a direct, self-describing item ("Go to Declaration: onTimer (timer)").
        const SMGoToDef::Target target = targets.first();
        QAction* action = menu.addAction(tr("Go to Declaration: %1  (%2)").arg(target.name, SMGoToDef::kindWord(target.kind)));
        const SMReferences::eTarget kind = target.kind;
        const uint32_t declId = target.declId;
        connect(action, &QAction::triggered, this, [this, kind, declId]() { emit signalNavigateToDefinition(kind, declId); });
    }
    else
    {
        // Several targets: the submenu itself is the picker, one entry per referenced declaration.
        QMenu* submenu = menu.addMenu(tr("Go to Declaration"));
        for (const SMGoToDef::Target& target : targets)
        {
            QAction* action = submenu->addAction(tr("%1  (%2)").arg(target.name, SMGoToDef::kindWord(target.kind)));
            const SMReferences::eTarget kind = target.kind;
            const uint32_t declId = target.declId;
            connect(action, &QAction::triggered, this, [this, kind, declId]() { emit signalNavigateToDefinition(kind, declId); });
        }
    }
}

void SMDesign::onSearchTextChanged()
{
    const QString query = mSearchEdit->text().trimmed();
    mSearchHits.clear();
    mSearchIndex = -1;

    if (query.isEmpty())
    {
        mSeedActive = false;
        mSearchStatus->clear();
        return;
    }

    if (mSeedActive && (query == mSeedName))
    {
        // Seeded from a selected entry: list exactly its usage sites, keyed by id and kind, so a
        // same-named entry of another kind is ignored (issue #538). Options do not apply here -
        // the query is the entry's own name, not a free-text pattern.
        const QList<SMReferences::Use> uses = SMWhereUsed::collect(mModel.getData(), mSeedTarget, mSeedName, mSeedId);
        for (const SMReferences::Use& use : uses)
        {
            const uint32_t level = levelOfElement(use.navId, use.isState);
            if (level != 0)
                mSearchHits.append({ level, use.navId, use.isState });
        }
    }
    else
    {
        // Any edit away from the seed name reverts to the free-text name/ID scan.
        mSeedActive = false;
        collectSearchHits(query, mModel.getData().getStates(), mSceneManager->getRootLevel(), mSearchHits);
    }

    if (mSearchHits.isEmpty())
    {
        // No match: leave the canvas untouched, show a clear affordance.
        mSearchStatus->setText(tr("No match"));
        return;
    }

    mSearchIndex = 0;
    focusSearchHit(0);
    updateSearchStatus();
}

void SMDesign::advanceSearch()
{
    if (mSearchEdit->text().trimmed().isEmpty())
    {
        mSearchStatus->clear();
        return;
    }

    if (mSearchHits.isEmpty())
    {
        onSearchTextChanged();
        return;
    }

    mSearchIndex = (mSearchIndex + 1) % mSearchHits.size();
    focusSearchHit(mSearchIndex);
    updateSearchStatus();
}

void SMDesign::focusSearchHit(int index)
{
    if ((index < 0) || (index >= mSearchHits.size()))
    {
        return;
    }

    const SearchHit hit = mSearchHits.at(index);

    // navigateTo swaps the shown scene and sets the active level (which clears the
    // selection), so select the element afterwards; the scene highlights it through the
    // shared selection path (SMCanvasItem/SMEdgeItem).
    mSceneManager->navigateTo(hit.level);
    mModel.getSelectionModel().setSelection(QList<uint32_t>{ hit.elementId });

    if (SMCanvasItem* item = getScene().findCanvasItem(hit.elementId))
    {
        mView->centerOn(item);
    }
}

namespace
{
    //!< The owner id of the level that directly contains \p stateId, or 0 if none does.
    uint32_t levelOfState(const SMStateData& level, uint32_t levelId, uint32_t stateId)
    {
        for (const SMStateEntry* st : level.getElements())
        {
            if (st == nullptr)
                continue;
            if (st->getId() == stateId)
                return levelId;
            if (st->hasNestedStates())
            {
                const uint32_t found = levelOfState(*st->getNestedStates(), st->getId(), stateId);
                if (found != 0)
                    return found;
            }
        }
        return 0;
    }

    //!< If \p elementId is a transition, or a condition row / operation owned by one, returns
    //!< that transition's id and sets \p levelOut to the level drawing its source state; else 0.
    uint32_t transitionForElement(const SMStateData& level, uint32_t levelId, uint32_t elementId, uint32_t& levelOut)
    {
        for (const SMStateEntry* st : level.getElements())
        {
            if (st == nullptr)
                continue;
            for (const SMTransitionEntry* tr : st->getTransitions().getElements())
            {
                if (tr == nullptr)
                    continue;
                bool match = (tr->getId() == elementId);
                if (match == false)
                    for (const SMConditionEntry* leaf : tr->getConditions().collectLeaves())
                        if ((leaf != nullptr) && (leaf->getId() == elementId)) { match = true; break; }
                if (match == false)
                    for (const SMOperationBase* op : tr->getOperations().getOperations())
                        if ((op != nullptr) && (op->getId() == elementId)) { match = true; break; }
                if (match)
                {
                    levelOut = levelId;
                    return tr->getId();
                }
            }
            if (st->hasNestedStates())
            {
                const uint32_t owner = transitionForElement(*st->getNestedStates(), st->getId(), elementId, levelOut);
                if (owner != 0)
                    return owner;
            }
        }
        return 0;
    }

    //!< The state whose entry/exit/do list owns operation \p opId (and its level), or 0.
    uint32_t stateForOperation(const SMStateData& level, uint32_t levelId, uint32_t opId, uint32_t& levelOut)
    {
        auto owns = [opId](const SMOperationList& ops) -> bool
        {
            for (const SMOperationBase* op : ops.getOperations())
                if ((op != nullptr) && (op->getId() == opId)) return true;
            return false;
        };
        for (const SMStateEntry* st : level.getElements())
        {
            if (st == nullptr)
                continue;
            if (owns(st->getEntryList()) || owns(st->getExitList()) || owns(st->getDoList()))
            {
                levelOut = levelId;
                return st->getId();
            }
            if (st->hasNestedStates())
            {
                const uint32_t owner = stateForOperation(*st->getNestedStates(), st->getId(), opId, levelOut);
                if (owner != 0)
                    return owner;
            }
        }
        return 0;
    }
}

void SMDesign::revealOnCanvas(uint32_t levelId, uint32_t canvasElementId)
{
    // The same reveal path the canvas search uses: navigate the level, select through the
    // shared selection model, and center the viewport on the resulting item.
    mSceneManager->navigateTo(levelId);
    mModel.getSelectionModel().setSelection(QList<uint32_t>{ canvasElementId });
    if (SMCanvasItem* item = getScene().findCanvasItem(canvasElementId))
    {
        mView->centerOn(item);
    }
}

void SMDesign::navigateToIssue(uint32_t elementId, eDocElementKind kind)
{
    const SMStateData& root = mModel.getData().getStates();
    const uint32_t rootLevel = mSceneManager->getRootLevel();

    switch (kind)
    {
    case eDocElementKind::State:
    {
        const uint32_t levelId = levelOfState(root, rootLevel, elementId);
        if (levelId != 0)
            revealOnCanvas(levelId, elementId);
        break;
    }

    case eDocElementKind::Transition:
    case eDocElementKind::Condition:
    case eDocElementKind::Operation:
    {
        uint32_t levelId = 0;
        const uint32_t transitionId = transitionForElement(root, rootLevel, elementId, levelId);
        if (transitionId != 0)
        {
            revealOnCanvas(levelId, transitionId);
            if (mProperties != nullptr)
            {
                if ((mPropertiesDock != nullptr) && (mPropertiesDock->widget() == mProperties) && (mPropertiesDock->isVisible() == false))
                {
                    mPropertiesDock->show();
                    mPropertiesDock->raise();
                }
                mProperties->focusConditions(transitionId);
            }
        }
        else if (kind == eDocElementKind::Operation)
        {
            // A state entry/exit/do operation belongs to a state, not a transition.
            uint32_t stateLevel = 0;
            const uint32_t stateId = stateForOperation(root, rootLevel, elementId, stateLevel);
            if (stateId != 0)
                revealOnCanvas(stateLevel, stateId);
        }
        break;
    }

    default:
        // A registry entry: its editor is another page, owned by the window.
        emit signalNavigateToPage(kind);
        break;
    }
}

void SMDesign::updateSearchStatus()
{
    mSearchStatus->setText(tr("%1 / %2").arg(mSearchIndex + 1).arg(mSearchHits.size()));
}

void SMDesign::collectSearchHits(const QString& query, const SMStateData& level, uint32_t levelId, QList<SearchHit>& out) const
{
    // A purely numeric query additionally matches an element by its exact ID (spec 11:
    // find by name or ID); a name substring match still applies to the same digits.
    bool numeric = false;
    const uint32_t queryId = query.toUInt(&numeric);

    for (const SMStateEntry* state : level.getElements())
    {
        if (state == nullptr)
        {
            continue;
        }

        if (matchText(state->getName(), query)
            || (numeric && (state->getId() == queryId)))
        {
            out.append({ levelId, state->getId(), true });
        }

        for (const SMTransitionEntry* transition : state->getTransitions().getElements())
        {
            if (transition == nullptr)
            {
                continue;
            }

            if (matchText(transition->getStimulus(), query)
                || matchText(transition->getTargetName(), query)
                || (numeric && (transition->getId() == queryId)))
            {
                out.append({ levelId, transition->getId(), false });
            }
        }

        if (state->hasNestedStates())
        {
            collectSearchHits(query, *state->getNestedStates(), state->getId(), out);
        }
    }
}

bool SMDesign::matchText(const QString& hay, const QString& needle) const
{
    if (needle.isEmpty())
        return false;

    const Qt::CaseSensitivity cs = mSearchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegularExpression::PatternOptions opt = QRegularExpression::NoPatternOption;
    if (cs == Qt::CaseInsensitive)
        opt |= QRegularExpression::CaseInsensitiveOption;

    if (mSearchRegex->isChecked())
    {
        const QRegularExpression re(needle, opt);
        return re.isValid() && re.match(hay).hasMatch();
    }

    if (mSearchWord->isChecked())
    {
        // \b...\b so the needle matches only as a whole word, not as a substring.
        const QRegularExpression re(QStringLiteral("\\b") + QRegularExpression::escape(needle) + QStringLiteral("\\b"), opt);
        return re.match(hay).hasMatch();
    }

    return hay.contains(needle, cs);
}

uint32_t SMDesign::levelOfElement(uint32_t id, bool isState) const
{
    const SMStateData& root = mModel.getData().getStates();
    const uint32_t rootLevel = mSceneManager->getRootLevel();
    if (isState)
        return levelOfState(root, rootLevel, id);

    uint32_t level = 0;
    transitionForElement(root, rootLevel, id, level);
    return level;
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
    const bool normal   = (single != nullptr) && (single->getKind() == SMStateEntry::eStateKind::Normal);
    const bool imported = (single != nullptr) && single->isImportedSubmachine();
    const bool painted  = (single != nullptr) && single->hasNestedStates();

    // At most one of the three is ever meaningful, so one control carries all three -- and says
    // which one it is, because two of them change the document. A state that hosts an imported
    // machine cannot gain a painted one, a painted one can only be entered, and a bare Normal
    // state gets a painted subtree (imported is set through the Submachine picker, not here).
    mActEnterSubmachine->setEnabled(imported || painted || normal);
    if (imported)
    {
        mActEnterSubmachine->setText(tr("Open Imported Machine"));
        mActEnterSubmachine->setIcon(SMToolIcons::icon(SMToolIcons::eIcon::EnterSubmachine));
    }
    else if (painted)
    {
        mActEnterSubmachine->setText(tr("Enter Submachine"));
        mActEnterSubmachine->setIcon(SMToolIcons::icon(SMToolIcons::eIcon::EnterSubmachine));
    }
    else if (normal)
    {
        mActEnterSubmachine->setText(tr("Add Substate"));
        mActEnterSubmachine->setIcon(SMToolIcons::icon(SMToolIcons::eIcon::AddState));
    }

    mActAddSubstate->setEnabled(normal && (imported == false) && (painted == false));
    // Registering a machine is useful whether or not a state is selected; linking it to the
    // selection is the part that needs a bare Normal state.
    mActAddSubmachine->setEnabled(true);
    mActRemoveSubmachine->setEnabled(imported || painted);

    // Internal transitions run operations without leaving the state; only a Normal (possibly
    // composite) state can carry them. A Start state is a pure entry marker with no entry /
    // exit / internal behaviour, and a Final state has no outgoing transitions at all, so
    // both exclude the action (issue #516 bug 5).
    mActAddInternal->setEnabled((single != nullptr) && (single->getKind() == SMStateEntry::eStateKind::Normal));

    // Add Transition needs at least one valid target on the current level. A Start state can
    // never be a transition target, so a level that holds only its Start (no other state)
    // offers nowhere to draw a transition: disable the tool button and the menu entry until a
    // non-Start state exists (issue #516 bug 2).
    bool hasTargetState = false;
    const SMStateData* level = mModel.getData().findLevel(getScene().getLevelId());
    if (level != nullptr)
    {
        for (const SMStateEntry* entry : level->getElements())
        {
            if ((entry != nullptr) && (entry->getKind() != SMStateEntry::eStateKind::Start))
            {
                hasTargetState = true;
                break;
            }
        }
    }

    if (mActAddTransition != nullptr)
    {
        mActAddTransition->setEnabled(hasTargetState);
    }

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

    // The nested Start is a compact marker box (same size as the root level's Start), not a
    // full normal-state box: it must match the topmost Start/Final marker size (issue #516).
    const QRectF geometry{ 64.0, 64.0, NESMDesign::MarkerStateWidth, NESMDesign::MarkerStateHeight };
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

    if (state->isImportedSubmachine())
    {
        // The machine lives in another file and the host never reaches inside it, so descending
        // means opening that document -- read-only, because it is not this document's to change.
        emit signalOpenImport(id, state->getSubmachine());
    }
    else if (state->hasNestedStates())
    {
        mSceneManager->enterSubmachine(id);
    }
    else if (state->getKind() == SMStateEntry::eStateKind::Normal)
    {
        // The state has no submachine yet: create a painted composite (with its Start state)
        // and descend into it, so "Enter Submachine" doubles as "start designing one here".
        // (Follow-up: an auto-created submachine left with only its Start state should revert
        // to a plain state when the user leaves it; that needs an undoable revert command.)
        addSubstateToSelection();
    }
}

void SMDesign::addSubmachineToSelection()
{
    const StateMachineData& data = mModel.getData();
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    const SMStateEntry* single = (selection.size() == 1 ? data.findStateById(selection.first()) : nullptr);
    const bool linkable = (single != nullptr)
                       && (single->getKind() == SMStateEntry::eStateKind::Normal)
                       && (single->isImportedSubmachine() == false)
                       && (single->hasNestedStates() == false);

    emit signalAddSubmachineRequested(linkable ? single->getId() : 0u);
}

void SMDesign::removeSubmachineFromSelection()
{
    StateMachineData& data = mModel.getData();
    const QList<uint32_t>& selection = mModel.getSelectionModel().getSelection();
    const SMStateEntry* single = (selection.size() == 1 ? data.findStateById(selection.first()) : nullptr);
    if (single == nullptr)
    {
        return;
    }

    const uint32_t stateId = single->getId();
    if (single->isImportedSubmachine())
    {
        // Unlinking removes nothing -- the registration and the imported file both stay -- so
        // there is nothing to confirm.
        SMSetSubmachineCommand* unlink = new SMSetSubmachineCommand(data, mModel.getNotifier(), stateId, QString()
                                                                   , tr("Remove submachine from %1").arg(single->getName()));
        if (unlink->isEffective())
        {
            mModel.getUndoStack().push(unlink);
        }
        else
        {
            delete unlink;
        }

        return;
    }

    SMRemoveCompositeCommand* command = new SMRemoveCompositeCommand(data, mModel.getNotifier(), stateId
                                                                    , tr("Remove submachine of %1").arg(single->getName()));
    if (command->isEffective() == false)
    {
        delete command;
        return;
    }

    // The extent is not visible: the subtree may be several levels deep, and the transitions that
    // point into it live on states the user is not looking at.
    const int states = command->removedStateCount();
    const int edges  = command->removedTransitionCount();
    const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Remove Submachine")
                        , tr("Remove the submachine of '%1'? This deletes %2 state%3 and %4 transition%5 that target them. This can be undone.")
                          .arg(single->getName())
                          .arg(states).arg(states == 1 ? QString() : QStringLiteral("s"))
                          .arg(edges).arg(edges == 1 ? QString() : QStringLiteral("s"))
                        , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (choice != QMessageBox::Yes)
    {
        delete command;
        return;
    }

    // Standing inside the level that is about to disappear leaves the canvas on a scene with no
    // owner, so step out to the host's own level first.
    const QList<uint32_t> path = mSceneManager->getCurrentPath();
    const int hostIndex = static_cast<int>(path.indexOf(stateId));
    if (hostIndex > 0)
    {
        mSceneManager->navigateTo(path.at(hostIndex - 1));
    }

    mModel.getUndoStack().push(command);
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

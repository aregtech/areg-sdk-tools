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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMDesign.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM editor Design page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMDesign.hpp"

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMCanvasItem.hpp"
#include "lusan/view/sm/SMGraphicsView.hpp"
#include "lusan/view/sm/SMScene.hpp"

#include <QAction>
#include <QKeyEvent>
#include <QPainter>
#include <QVBoxLayout>

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
}

SMDesign::SMDesign(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget       (parent)
    , mModel        (model)
    , mView         (new SMGraphicsView(this))
    , mScene        (nullptr)
    , mActZoomIn    (nullptr)
    , mActZoomOut   (nullptr)
    , mActZoomReset (nullptr)
    , mActZoomFit   (nullptr)
    , mActToggleGrid(nullptr)
    , mActToggleSnap(nullptr)
    , mActSelectAll (nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mView);

    rebuildScene();
    setupActions();
    mView->installEventFilter(this);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMDesign::onDocumentReloaded);
}

bool SMDesign::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == mView)
    {
        if (event->type() == QEvent::ShortcutOverride)
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
        getScene().setGridVisible(checked);
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

    const QList<QAction*> actions{ mActZoomIn, mActZoomOut, mActZoomReset, mActZoomFit
                                 , mActToggleGrid, mActToggleSnap, mActSelectAll };
    for (QAction* action : actions)
    {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        addAction(action);
    }

    mActToggleGrid->setChecked(getScene().isGridVisible());
    mActToggleSnap->setChecked(getScene().isSnapToGrid());
}

void SMDesign::rebuildScene()
{
    SMScene* previous = mScene;
    const uint32_t rootLevel = mModel.getData().getOverview().getId();

    mScene = new SMScene(mModel, rootLevel, this);
    applyGridSettings();
    populateStressContent();
    mView->setScene(mScene);
    mModel.getSelectionModel().setActiveLevel(rootLevel);
    mView->zoomReset();

    if (mActToggleGrid != nullptr)
    {
        mActToggleGrid->setChecked(mScene->isGridVisible());
        mActToggleSnap->setChecked(mScene->isSnapToGrid());
    }

    delete previous;
}

void SMDesign::applyGridSettings()
{
    const SMLayoutData& layout = mModel.getData().getLayout();
    mScene->setGridSize(layout.getGridSize());
    mScene->setGridVisible(layout.isGridVisible());
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

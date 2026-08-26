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
 *  \file        lusan/view/common/NavigationDock.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The navigation docking widget of lusan.
 *
 ************************************************************************/
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/common/NaviFsmToolbar.hpp"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QSettings>
#include <QShowEvent>
#include <algorithm>

namespace
{
    //!< The stored choice of the captions under the rail icons.
    const QString   KEY_RAIL_LABELS { QStringLiteral("navigation/railLabels") };
}

QString NavigationDock::panelName(NavigationDock::eNaviWindow navi)
{
    switch (navi)
    {
    case NavigationDock::eNaviWindow::NaviWorkspace:
        return QCoreApplication::translate("NavigationDock", "Workspace");
    case NavigationDock::eNaviWindow::NaviLiveLogs:
        return QCoreApplication::translate("NavigationDock", "Live Logs");
    case NavigationDock::eNaviWindow::NaviOfflineLogs:
        return QCoreApplication::translate("NavigationDock", "Offline Logs");
    case NavigationDock::eNaviWindow::NaviDesignToolbar:
        return QCoreApplication::translate("NavigationDock", "Toolbar");
    case NavigationDock::eNaviWindow::NaviDesignProperties:
        return QCoreApplication::translate("NavigationDock", "Properties");
    case NavigationDock::eNaviWindow::NaviDesignOutline:
        return QCoreApplication::translate("NavigationDock", "Outline");
    default:
        return QString();
    }
}

QString NavigationDock::panelHint(NavigationDock::eNaviWindow navi)
{
    switch (navi)
    {
    case NavigationDock::eNaviWindow::NaviWorkspace:
        return QCoreApplication::translate("NavigationDock", "Workspace file explorer");
    case NavigationDock::eNaviWindow::NaviLiveLogs:
        return QCoreApplication::translate("NavigationDock", "Live logs scope explorer");
    case NavigationDock::eNaviWindow::NaviOfflineLogs:
        return QCoreApplication::translate("NavigationDock", "Offline logs scope explorer");
    case NavigationDock::eNaviWindow::NaviDesignToolbar:
        return QCoreApplication::translate("NavigationDock", "State machine drawing tools");
    case NavigationDock::eNaviWindow::NaviDesignProperties:
        return QCoreApplication::translate("NavigationDock", "Properties of the selected design element");
    case NavigationDock::eNaviWindow::NaviDesignOutline:
        return QCoreApplication::translate("NavigationDock", "Outline of the state machine design");
    default:
        return QString();
    }
}

QString NavigationDock::panelIcon(NavigationDock::eNaviWindow navi)
{
    switch (navi)
    {
    case NavigationDock::eNaviWindow::NaviWorkspace:
        return QStringLiteral(":/icons/nav-workspace");
    case NavigationDock::eNaviWindow::NaviLiveLogs:
        return QStringLiteral(":/icons/nav-live-logs");
    case NavigationDock::eNaviWindow::NaviOfflineLogs:
        return QStringLiteral(":/icons/nav-offline-logs");
    case NavigationDock::eNaviWindow::NaviDesignToolbar:
        return QStringLiteral(":/icons/nav-design-toolbar");
    case NavigationDock::eNaviWindow::NaviDesignProperties:
        return QStringLiteral(":/icons/nav-sm-properties");
    case NavigationDock::eNaviWindow::NaviDesignOutline:
        return QStringLiteral(":/icons/nav-sm-outline");
    default:
        return QString();
    }
}

QKeySequence NavigationDock::panelShortcut(NavigationDock::eNaviWindow navi)
{
    switch (navi)
    {
    case NavigationDock::eNaviWindow::NaviWorkspace:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_1);
    case NavigationDock::eNaviWindow::NaviLiveLogs:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_2);
    case NavigationDock::eNaviWindow::NaviOfflineLogs:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_3);
    case NavigationDock::eNaviWindow::NaviDesignToolbar:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_4);
    case NavigationDock::eNaviWindow::NaviDesignProperties:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_5);
    case NavigationDock::eNaviWindow::NaviDesignOutline:
        return QKeySequence(Qt::Modifier::ALT | Qt::Key::Key_6);
    default:
        return QKeySequence();
    }
}

bool NavigationDock::isDesignPanel(NavigationDock::eNaviWindow navi)
{
    return (navi == NavigationDock::eNaviWindow::NaviDesignToolbar)
        || (navi == NavigationDock::eNaviWindow::NaviDesignProperties)
        || (navi == NavigationDock::eNaviWindow::NaviDesignOutline);
}

NavigationDock::NavigationDock(MdiMainWindow* parent)
    : QWidget       (parent)

    , mMainWindow   (parent)
    , mLayout       (new QHBoxLayout(this))
    , mRail         (new NaviTabRail(this))
    , mStack        (new QStackedWidget(this))
    , mEmptyHint    (new QLabel(this))
    , mLiveScopes   (parent, this)
    , mOfflineScopes(parent, this)
    , mFileSystem   (parent, this)
    , mPanels       ( )
    , mRailOrdered  (false)
    , mCollapsed    (false)
    , mExpandStamp  ( )
{
    mEmptyHint->setAlignment(Qt::AlignmentFlag::AlignCenter);
    mEmptyHint->setWordWrap(true);
    mEmptyHint->setMargin(12);
    mEmptyHint->setEnabled(false);
    mEmptyHint->setText(tr("No navigator is shown.\n\nRight click the icon strip, or open View > Navigation, to bring one back."));

    mStack->addWidget(mEmptyHint);

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    mLayout->addWidget(mRail);
    mLayout->addWidget(mStack, 1);

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    mRail->setLabelsPreferred(settings.value(KEY_RAIL_LABELS, false).toBool());

    registerPanel(NavigationDock::eNaviWindow::NaviWorkspace  , &mFileSystem);
    registerPanel(NavigationDock::eNaviWindow::NaviLiveLogs   , &mLiveScopes);
    registerPanel(NavigationDock::eNaviWindow::NaviOfflineLogs, &mOfflineScopes);

    connect(mRail, &NaviTabRail::signalItemActivated, this, [this](int id) {
        setCurrentPanel(static_cast<NavigationDock::eNaviWindow>(id));
        setContentCollapsed(false);
    });

    // Clicking the navigator that is already up opens a collapsed panel, otherwise it moves the
    // keyboard focus into the navigator body.
    connect(mRail, &NaviTabRail::signalCurrentItemClicked, this, [this]() {
        if (mCollapsed)
        {
            setContentCollapsed(false);
            return;
        }

        NavigationWindow* panel = getPanel(currentPanel());
        if (panel != nullptr)
        {
            panel->setFocus(Qt::FocusReason::MouseFocusReason);
        }
    });

    connect(mRail, &NaviTabRail::signalHidePanelRequested, this, &NavigationDock::signalCollapseRequested);
    // A double-click that lands right after a click opened the panel is the second half of that
    // gesture, not a request to close it again.
    connect(mRail, &NaviTabRail::signalToggleCollapseRequested, this, [this]() {
        if (justExpanded() == false)
        {
            setContentCollapsed(mCollapsed == false);
        }
    });

    connect(mRail, &NaviTabRail::signalItemVisibilityToggled, this, [this](int id, bool visible) {
        const NavigationDock::eNaviWindow navi{ static_cast<NavigationDock::eNaviWindow>(id) };
        if (NavigationDock::isDesignPanel(navi))
        {
            // A design widget belongs to the Design page; the main window decides where it goes.
            if (visible == false)
            {
                emit signalDesignPanelHidden(navi);
            }
        }
        else
        {
            setPanelVisible(navi, visible);
        }
    });

    connect(mRail, &NaviTabRail::signalLabelsToggled, this, &NavigationDock::setRailLabels);

    initSize();
    showPanel(NavigationDock::eNaviWindow::NaviWorkspace);

    connect(mMainWindow, &MdiMainWindow::signalOptionsOpening   , this, &NavigationDock::onOptionsOpening);
    connect(mMainWindow, &MdiMainWindow::signalOptionsApplied   , this, &NavigationDock::onOptionsApplied);
    connect(mMainWindow, &MdiMainWindow::signalOptionsClosed    , this, &NavigationDock::onOptionsClosed);
}

void NavigationDock::registerPanel(NavigationDock::eNaviWindow navi, NavigationWindow* content)
{
    if ((content == nullptr) || (navi == NavigationDock::eNaviWindow::NaviUnknown))
        return;

    mPanels.insert(static_cast<int>(navi), content);
    if (mStack->indexOf(content) < 0)
    {
        mStack->addWidget(content);
    }

    mRail->addItem(static_cast<int>(navi)
                 , NavigationDock::panelIcon(navi)
                 , NavigationDock::panelName(navi)
                 , NavigationDock::panelHint(navi)
                 , NavigationDock::panelShortcut(navi).toString(QKeySequence::SequenceFormat::NativeText));
}

NavigationWindow* NavigationDock::getPanel(NavigationDock::eNaviWindow navi) const
{
    return mPanels.value(static_cast<int>(navi), nullptr);
}

bool NavigationDock::hasPanel(NavigationDock::eNaviWindow navi) const
{
    return mPanels.contains(static_cast<int>(navi));
}

bool NavigationDock::showPanel(NavigationDock::eNaviWindow navi)
{
    NavigationWindow* content = getPanel(navi);
    if (content == nullptr)
        return false;

    // An explicit request wins over a navigator the user had hidden earlier.
    mRail->setItemVisible(static_cast<int>(navi), true);
    if (mRail->currentItem() == static_cast<int>(navi))
    {
        setCurrentPanel(navi);
    }
    else
    {
        mRail->setCurrentItem(static_cast<int>(navi));
    }

    return true;
}

NavigationDock::eNaviWindow NavigationDock::currentPanel(void) const
{
    const int id = mRail->currentItem();
    return (id > 0 ? static_cast<NavigationDock::eNaviWindow>(id) : NavigationDock::eNaviWindow::NaviUnknown);
}

void NavigationDock::setCurrentPanel(NavigationDock::eNaviWindow navi)
{
    NavigationWindow* content = getPanel(navi);
    mStack->setCurrentWidget(content != nullptr ? static_cast<QWidget*>(content) : static_cast<QWidget*>(mEmptyHint));
    emit signalPanelChanged(content != nullptr ? navi : NavigationDock::eNaviWindow::NaviUnknown);
}

void NavigationDock::selectFallbackPanel(void)
{
    const int next = mRail->firstVisibleItem();
    if (next > 0)
    {
        mRail->setCurrentItem(next);
    }
    else
    {
        setCurrentPanel(NavigationDock::eNaviWindow::NaviUnknown);
    }
}

void NavigationDock::showDesignPanel(NavigationDock::eNaviWindow navi, NavigationWindow* content)
{
    if ((content == nullptr) || (NavigationDock::isDesignPanel(navi) == false))
        return;

    NavigationWindow* hosted = getPanel(navi);
    if (hosted == content)
    {
        mRail->setItemVisible(static_cast<int>(navi), true);
        return;
    }

    if (hosted != nullptr)
    {
        hideDesignPanel(navi);
    }

    registerPanel(navi, content);
    mRail->applyPanelWidth(width());
}

void NavigationDock::hideDesignPanel(NavigationDock::eNaviWindow navi)
{
    NavigationWindow* content = getPanel(navi);
    if (content == nullptr)
        return;

    const bool wasCurrent = (currentPanel() == navi);
    mPanels.remove(static_cast<int>(navi));
    mRail->removeItem(static_cast<int>(navi));
    mStack->removeWidget(content);

    // The main window owns the widget: it may go back to the Design page or return here later.
    content->setParent(mMainWindow);
    content->hide();

    if (wasCurrent)
    {
        selectFallbackPanel();
    }

    mRail->applyPanelWidth(width());
}

bool NavigationDock::isDesignPanelShown(NavigationDock::eNaviWindow navi) const
{
    return hasPanel(navi);
}

void NavigationDock::setPanelVisible(NavigationDock::eNaviWindow navi, bool visible)
{
    if (hasPanel(navi) == false)
        return;

    const bool wasCurrent = (currentPanel() == navi);
    mRail->setItemVisible(static_cast<int>(navi), visible);
    if (visible)
    {
        if (currentPanel() == NavigationDock::eNaviWindow::NaviUnknown)
        {
            mRail->setCurrentItem(static_cast<int>(navi));
        }
    }
    else if (wasCurrent)
    {
        selectFallbackPanel();
    }
}

bool NavigationDock::isPanelVisible(NavigationDock::eNaviWindow navi) const
{
    return hasPanel(navi) && mRail->isItemVisible(static_cast<int>(navi));
}

void NavigationDock::setLiveLogsConnected(bool connected)
{
    mRail->setItemBadge(static_cast<int>(NavigationDock::eNaviWindow::NaviLiveLogs)
                      , connected ? NaviTabRail::eBadge::Active : NaviTabRail::eBadge::None);
}

void NavigationDock::setContentCollapsed(bool collapsed)
{
    if (mCollapsed == collapsed)
        return;

    mCollapsed = collapsed;
    mRail->setCollapsed(collapsed);

    if (collapsed)
    {
        mStack->hide();
    }
    else
    {
        mStack->show();
        mExpandStamp.start();
    }

    // The dock reads its floor from this widget, so the floor has to drop before the panel can.
    setMinimumWidth(collapsed ? mRail->width() : static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH_ABS));
    emit signalContentCollapsed(collapsed);
}

bool NavigationDock::justExpanded(void) const
{
    return (mExpandStamp.isValid() && (mExpandStamp.elapsed() < 400));
}

void NavigationDock::setRailLabels(bool labels)
{
    QSettings store(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    store.setValue(KEY_RAIL_LABELS, labels);
    mRail->setLabelsPreferred(labels);
    mRail->applyPanelWidth(width());
}

bool NavigationDock::railLabels(void) const
{
    return mRail->labelsPreferred();
}

void NavigationDock::initSize(void)
{
    // The user may shrink the dock to the absolute minimum width while the preferred width stays
    // larger. The page stack must accept the same floor, or its minimum would hold the dock open.
    const int absMinWidth = static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH_ABS);
    const int defWidth = static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH);
    const int minHeight = static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT);
    setMinimumWidth(absMinWidth);
    setMinimumHeight(minHeight);
    mStack->setMinimumWidth(0);
    mStack->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);
    resize(QSize{ defWidth, std::max(minHeight, height()) });
    setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
}

void NavigationDock::updateRailSide(void)
{
    QWidget* top = window();
    if (top == nullptr)
        return;

    const QPoint center = mapTo(top, rect().center());
    const NaviTabRail::eSide side = ((center.x() * 2) <= top->width())
                                  ? NaviTabRail::eSide::West
                                  : NaviTabRail::eSide::East;
    if (mRailOrdered && (side == mRail->side()))
        return;

    mRailOrdered = true;
    mRail->setSide(side);
    mLayout->removeWidget(mRail);
    mLayout->removeWidget(mStack);
    if (side == NaviTabRail::eSide::West)
    {
        mLayout->addWidget(mRail);
        mLayout->addWidget(mStack, 1);
    }
    else
    {
        mLayout->addWidget(mStack, 1);
        mLayout->addWidget(mRail);
    }
}

void NavigationDock::resizeEvent(QResizeEvent* event)
{
    mRail->applyPanelWidth(width());
    updateRailSide();
    QWidget::resizeEvent(event);
}

void NavigationDock::moveEvent(QMoveEvent* event)
{
    updateRailSide();
    QWidget::moveEvent(event);
}

void NavigationDock::showEvent(QShowEvent* event)
{
    mRail->applyPanelWidth(width());
    updateRailSide();
    QWidget::showEvent(event);
}

QSize NavigationDock::sizeHint(void) const
{
    QSize result{ QWidget::sizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH));
    return result;
}

QSize NavigationDock::minimumSizeHint(void) const
{
    QSize result{ QWidget::minimumSizeHint() };
    result.setWidth(static_cast<int>(NELusanCommon::MIN_NAVI_WIDTH_ABS));
    result.setHeight(std::max(result.height(), static_cast<int>(NELusanCommon::MIN_NAVI_HEIGHT)));
    return result;
}

void NavigationDock::onOptionsOpening(void)
{
    static_cast<NavigationWindow &>(mFileSystem).optionOpenning();
    static_cast<NavigationWindow &>(mLiveScopes).optionOpenning();
    static_cast<NavigationWindow &>(mOfflineScopes).optionOpenning();
}

void NavigationDock::onOptionsApplied(void)
{
    static_cast<NavigationWindow &>(mFileSystem).optionApplied();
    static_cast<NavigationWindow &>(mLiveScopes).optionApplied();
    static_cast<NavigationWindow &>(mOfflineScopes).optionApplied();
}

void NavigationDock::onOptionsClosed(bool pressedOK)
{
    static_cast<NavigationWindow &>(mFileSystem).optionClosed(pressedOK);
    static_cast<NavigationWindow &>(mLiveScopes).optionClosed(pressedOK);
    static_cast<NavigationWindow &>(mOfflineScopes).optionClosed(pressedOK);
}

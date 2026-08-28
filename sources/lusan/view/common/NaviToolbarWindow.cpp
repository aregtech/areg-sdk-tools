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
 *  \file        lusan/view/common/NaviToolbarWindow.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base of navigation windows built of a tool button row and a tree view.
 *
 ************************************************************************/

#include "lusan/view/common/NaviToolbarWindow.hpp"

#include "lusan/common/NELusanCommon.hpp"

#include <algorithm>

#include <QEvent>
#include <QFrame>
#include <QLayoutItem>
#include <QMargins>
#include <QPalette>
#include <QSet>
#include <QHBoxLayout>
#include <QIcon>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

NaviToolbarWindow::NaviToolbarWindow(int naviWindow, MdiMainWindow* wndMain, QWidget* parent)
    : NavigationWindow  (naviWindow, wndMain, parent)

    , mNaviLayout       (new QVBoxLayout(this))
    , mToolbar          (new QWidget(this))
    , mToolLayout       (new QHBoxLayout(mToolbar))
    , mNaviTree         (new QTreeView(this))
    , mToolOverflow     (new QToolButton(mToolbar))
    , mOverflowRow      (nullptr)
    , mOverflowLayout   (nullptr)
    , mToolItems        ( )
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    mNaviLayout->setContentsMargins(0, 0, 0, 0);
    mNaviLayout->setSpacing(7);
    mNaviLayout->addWidget(mToolbar, 0, Qt::AlignmentFlag::AlignLeft);
    mNaviLayout->addWidget(mNaviTree);

    mToolbar->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

    const QSize chevron(NAVI_TOOL_ICON, NAVI_TOOL_ICON);
    mToolOverflow->setIcon(NELusanCommon::chevronIcon(true, mToolOverflow->palette().color(QPalette::ColorRole::ButtonText), chevron));
    mToolOverflow->setIconSize(chevron);
    mToolOverflow->setAutoRaise(true);
    mToolOverflow->setToolTip(tr("More tools"));
    mToolOverflow->setStatusTip(tr("Show the tools that do not fit the row."));
    mToolOverflow->setAccessibleName(mToolOverflow->toolTip());
    mToolOverflow->setVisible(false);
    mToolLayout->addWidget(mToolOverflow);

    connect(mToolOverflow, &QToolButton::clicked, this, [this]() { showToolOverflow(); });
}

QToolButton* NaviToolbarWindow::addToolButton(const QIcon& icon, const QString& toolTip, const QString& statusTip, bool checkable /*= false*/)
{
    QToolButton* button = new QToolButton(mToolbar);
    button->setText(QStringLiteral("..."));
    button->setIcon(icon);
    button->setToolTip(toolTip);
    button->setStatusTip(statusTip);
    // The buttons show no text, so without this a screen reader announces the placeholder text.
    button->setAccessibleName(toolTip);
    button->setCheckable(checkable);
    button->setAutoRaise(true);
    // One extent for every navigation panel toolbar, so their marks carry the same stroke
    // weight instead of whatever the active style would pick.
    button->setIconSize(QSize(NAVI_TOOL_ICON, NAVI_TOOL_ICON));
    mToolLayout->insertWidget(mToolLayout->count() - 1, button);
    mToolItems.append(sToolItem{ button, ToolRankNormal, false });
    return button;
}

void NaviToolbarWindow::addToolSeparator(void)
{
    QFrame* line = new QFrame(mToolbar);
    line->setFrameShape(QFrame::Shape::VLine);
    line->setFrameShadow(QFrame::Shadow::Sunken);
    mToolLayout->insertWidget(mToolLayout->count() - 1, line);
    mToolItems.append(sToolItem{ line, ToolRankFixed, true });
}

void NaviToolbarWindow::addToolWidget(QWidget* widget)
{
    if (widget != nullptr)
    {
        widget->setParent(mToolbar);
        mToolLayout->insertWidget(mToolLayout->count() - 1, widget);
        mToolItems.append(sToolItem{ widget, ToolRankFixed, false });
    }
}

void NaviToolbarWindow::setToolRank(QWidget* widget, int rank)
{
    for (sToolItem& item : mToolItems)
    {
        if (item.widget == widget)
        {
            item.rank = rank;
            break;
        }
    }
}

void NaviToolbarWindow::setNaviHeader(QWidget* header)
{
    if (header != nullptr)
    {
        header->setParent(this);
        mNaviLayout->insertWidget(0, header);
    }
}

void NaviToolbarWindow::addNaviBar(QWidget* bar)
{
    if (bar != nullptr)
    {
        bar->setParent(this);
        mNaviLayout->insertWidget(mNaviLayout->indexOf(mNaviTree), bar);
    }
}

void NaviToolbarWindow::setupTreeView(const QSize& iconSize)
{
    mNaviTree->setAutoFillBackground(false);
    mNaviTree->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    mNaviTree->setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);
    mNaviTree->setDropIndicatorShown(false);
    mNaviTree->setAlternatingRowColors(false);
    mNaviTree->setIconSize(iconSize);
    mNaviTree->setHeaderHidden(true);
}

void NaviToolbarWindow::capToolButtonIconSizes(int iconExtent /*= NAVI_TOOL_ICON*/)
{
    const QSize extent(iconExtent, iconExtent);
    const QList<QToolButton*> buttons = findChildren<QToolButton*>();
    for (QToolButton* button : buttons)
    {
        button->setIconSize(extent);
    }
}

void NaviToolbarWindow::resizeEvent(QResizeEvent* event)
{
    NavigationWindow::resizeEvent(event);
    updateToolOverflow();
}

void NaviToolbarWindow::updateToolOverflow(void)
{
    if (mToolItems.isEmpty())
        return;

    // The second row holds the entries while it is open, so their widths are not measurable.
    if ((mOverflowRow != nullptr) && mOverflowRow->isVisible())
        return;

    const int spacing{ mToolLayout->spacing() > 0 ? mToolLayout->spacing() : 0 };
    QMargins margins{ mToolLayout->contentsMargins() };
    const int available{ width() - margins.left() - margins.right() };

    int needed{ 0 };
    for (const sToolItem& item : mToolItems)
    {
        needed += item.widget->sizeHint().width() + spacing;
    }

    // Every entry fits, so the chevron is not needed.
    if (needed <= available)
    {
        for (const sToolItem& item : mToolItems)
        {
            item.widget->setVisible(true);
        }

        mToolOverflow->setVisible(false);
        _hideDanglingSeparators();
        return;
    }

    int budget{ available - mToolOverflow->sizeHint().width() - spacing };
    QList<int> order;
    for (int pos = 0; pos < mToolItems.size(); ++pos)
    {
        order.append(pos);
    }

    // The lowest rank gives up its place first; equal ranks give up from the right.
    std::stable_sort(order.begin(), order.end(), [this](int left, int right) {
        return mToolItems[left].rank != mToolItems[right].rank
                ? mToolItems[left].rank < mToolItems[right].rank
                : left > right;
    });

    QSet<int> dropped;
    for (int pos : order)
    {
        if (needed <= budget)
            break;
        else if (mToolItems[pos].rank >= ToolRankFixed)
            continue;

        needed -= mToolItems[pos].widget->sizeHint().width() + spacing;
        dropped.insert(pos);
    }

    for (int pos = 0; pos < mToolItems.size(); ++pos)
    {
        mToolItems[pos].widget->setVisible(dropped.contains(pos) == false);
    }

    mToolOverflow->setVisible(dropped.isEmpty() == false);
    _hideDanglingSeparators();
}

void NaviToolbarWindow::showToolOverflow(void)
{
    if (mOverflowRow == nullptr)
    {
        mOverflowRow = new QWidget(this, Qt::WindowType::Popup);
        mOverflowRow->setObjectName(QStringLiteral("naviToolOverflow"));
        mOverflowLayout = new QHBoxLayout(mOverflowRow);
        mOverflowLayout->setContentsMargins(4, 4, 4, 4);
        mOverflowLayout->setSpacing(2);
        mOverflowRow->installEventFilter(this);
    }

    for (const sToolItem& item : mToolItems)
    {
        if ((item.widget->isVisible() == false) && (item.divider == false))
        {
            mOverflowLayout->addWidget(item.widget);
            item.widget->setVisible(true);
        }
    }

    mOverflowRow->adjustSize();
    const QPoint below{ mToolOverflow->mapToGlobal(QPoint(0, mToolOverflow->height())) };
    mOverflowRow->move(below.x() + mToolOverflow->width() - mOverflowRow->width(), below.y() + 2);
    mOverflowRow->show();
}

void NaviToolbarWindow::closeToolOverflow(void)
{
    if (mOverflowRow == nullptr)
        return;

    bool borrowed{ false };
    for (const sToolItem& item : mToolItems)
    {
        if (item.widget->parentWidget() == mOverflowRow)
        {
            borrowed = true;
            break;
        }
    }

    if (borrowed == false)
        return;

    // The entries left the row, so the layout is rebuilt from the recorded order instead of
    // guessing the index each one came from.
    QLayoutItem* taken{ nullptr };
    while ((taken = mToolLayout->takeAt(0)) != nullptr)
    {
        delete taken;
    }

    for (const sToolItem& item : mToolItems)
    {
        item.widget->setParent(mToolbar);
        mToolLayout->addWidget(item.widget);
    }

    mToolLayout->addWidget(mToolOverflow);
    updateToolOverflow();
}

bool NaviToolbarWindow::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mOverflowRow) && (event->type() == QEvent::Type::Hide))
    {
        closeToolOverflow();
    }

    return NavigationWindow::eventFilter(watched, event);
}

void NaviToolbarWindow::_hideDanglingSeparators(void)
{
    // A separator with nothing visible after it draws a line at the end of the row.
    bool seen{ false };
    for (int pos = mToolItems.size() - 1; pos >= 0; --pos)
    {
        const sToolItem& item = mToolItems[pos];
        if (item.divider)
        {
            item.widget->setVisible(seen);
        }
        else if (item.widget->isVisible())
        {
            seen = true;
        }
    }
}

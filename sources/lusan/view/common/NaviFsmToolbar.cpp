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
 *  \file        lusan/view/common/NaviFsmToolbar.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The FSM design toolbar navigation window.
 *
 ************************************************************************/

#include "lusan/view/common/NaviFsmToolbar.hpp"
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/view/sm/NESMDesign.hpp"
#include "lusan/view/sm/SMDesign.hpp"

#include <QAction>
#include <QEvent>
#include <QLayout>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
    /**
     * \class   FlowLayout
     * \brief   A layout that arranges its items left to right and wraps to the next row when
     *          the available width runs out (the canonical Qt "Flow Layout" example). Used
     *          for the icon-only toolbar mode so the tool buttons flow into 2-3 columns that
     *          reflow with the navigation dock's width.
     **/
    class FlowLayout : public QLayout
    {
    public:
        explicit FlowLayout(QWidget* parent, int margin = 0, int hSpacing = -1, int vSpacing = -1)
            : QLayout   (parent)
            , mHSpace   (hSpacing)
            , mVSpace   (vSpacing)
        {
            setContentsMargins(margin, margin, margin, margin);
        }

        ~FlowLayout() override
        {
            QLayoutItem* item = nullptr;
            while ((item = takeAt(0)) != nullptr)
            {
                delete item;
            }
        }

        void addItem(QLayoutItem* item) override               { mItems.append(item); }
        int count() const override                             { return static_cast<int>(mItems.size()); }
        QLayoutItem* itemAt(int index) const override          { return mItems.value(index); }
        Qt::Orientations expandingDirections() const override  { return {}; }
        bool hasHeightForWidth() const override                { return true; }

        QLayoutItem* takeAt(int index) override
        {
            return ((index >= 0) && (index < mItems.size())) ? mItems.takeAt(index) : nullptr;
        }

        int heightForWidth(int width) const override
        {
            return doLayout(QRect(0, 0, width, 0), true);
        }

        void setGeometry(const QRect& rect) override
        {
            QLayout::setGeometry(rect);
            doLayout(rect, false);
        }

        QSize sizeHint() const override                        { return minimumSize(); }

        QSize minimumSize() const override
        {
            QSize size;
            for (const QLayoutItem* item : mItems)
            {
                size = size.expandedTo(item->minimumSize());
            }

            const QMargins m = contentsMargins();
            size += QSize(m.left() + m.right(), m.top() + m.bottom());
            return size;
        }

    private:
        int spacing(bool horizontal) const
        {
            const int stored = (horizontal ? mHSpace : mVSpace);
            if (stored >= 0)
            {
                return stored;
            }

            QStyle* style = (parentWidget() != nullptr ? parentWidget()->style() : nullptr);
            if (style == nullptr)
            {
                return 0;
            }

            const QStyle::PixelMetric pm = (horizontal ? QStyle::PM_LayoutHorizontalSpacing : QStyle::PM_LayoutVerticalSpacing);
            return style->pixelMetric(pm, nullptr, parentWidget());
        }

        int doLayout(const QRect& rect, bool testOnly) const
        {
            const QMargins m = contentsMargins();
            const QRect effective = rect.adjusted(m.left(), m.top(), -m.right(), -m.bottom());
            int x = effective.x();
            int y = effective.y();
            int lineHeight = 0;

            for (QLayoutItem* item : mItems)
            {
                const QSize hint = item->sizeHint();
                int nextX = x + hint.width() + spacing(true);
                if ((nextX - spacing(true) > effective.right()) && (lineHeight > 0))
                {
                    x = effective.x();
                    y = y + lineHeight + spacing(false);
                    nextX = x + hint.width() + spacing(true);
                    lineHeight = 0;
                }

                if (testOnly == false)
                {
                    item->setGeometry(QRect(QPoint(x, y), hint));
                }

                x = nextX;
                lineHeight = qMax(lineHeight, hint.height());
            }

            return (y + lineHeight - rect.y()) + m.bottom();
        }

        QList<QLayoutItem*> mItems;     //!< The flowed items.
        int                 mHSpace;    //!< Horizontal spacing (-1 = style default).
        int                 mVSpace;    //!< Vertical spacing (-1 = style default).
    };
}

NaviFsmToolbar::NaviFsmToolbar(MdiMainWindow* wndMain, QWidget* parent /*= nullptr*/)
    : NavigationWindow(static_cast<int>(NavigationDock::eNaviWindow::NaviDesignToolbar), wndMain, parent)
    , mScroll       (nullptr)
    , mGroups       (nullptr)
    , mGroupsLayout (nullptr)
    , mFallback     (nullptr)
    , mBound        ( )
    , mBuilt        (false)
    , mStyle        (Qt::ToolButtonIconOnly)
    , mActive       (false)
    , mStickyButtons( )
    , mCollapsed    ( )
{
    mGroups = new QWidget(this);
    mGroupsLayout = new QVBoxLayout(mGroups);
    mGroupsLayout->setContentsMargins(4, 4, 4, 4);
    mGroupsLayout->setSpacing(2);

    QWidget* content = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(mGroups);
    contentLayout->addStretch(1);

    mScroll = new QScrollArea(this);
    mScroll->setWidgetResizable(true);
    mScroll->setFrameShape(QFrame::NoFrame);
    mScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mScroll->setWidget(content);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mScroll);

    // The tab always shows the tool set: without a bound Design page these are the
    // disabled stand-in buttons (issue #514).
    rebuild();
}

void NaviFsmToolbar::bindDesign(SMDesign* design)
{
    if (mBuilt && (design == mBound.data()))
    {
        return;     // already bound; setToolsActive() handles the enabled state separately
    }

    mBound = design;
    rebuild();
}

void NaviFsmToolbar::setDisplayStyle(Qt::ToolButtonStyle style)
{
    mStyle = style;
    rebuild();
}

Qt::ToolButtonStyle NaviFsmToolbar::getDisplayStyle() const
{
    return mStyle;
}

void NaviFsmToolbar::setToolsActive(bool active)
{
    mActive = active;
    mGroups->setEnabled(active && (mBound != nullptr));
}

bool NaviFsmToolbar::showsText() const
{
    return (mStyle != Qt::ToolButtonIconOnly);
}

bool NaviFsmToolbar::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::MouseButtonDblClick) && (mBound != nullptr) && mStickyButtons.contains(watched))
    {
        mBound->armStickyTool(static_cast<NESMDesign::eCanvasTool>(mStickyButtons.value(watched)));
        return true;
    }

    return NavigationWindow::eventFilter(watched, event);
}

void NaviFsmToolbar::rebuild()
{
    mStickyButtons.clear();

    QLayoutItem* item = nullptr;
    while ((item = mGroupsLayout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    // Unbound: show the same groups built from disabled stand-in actions, so the tab
    // always presents the full tool set (issue #514). The stand-ins are owned by a
    // dedicated holder recreated per rebuild, so they never accumulate.
    delete mFallback;
    mFallback = nullptr;

    QList<SMDesign::ToolGroup> groups;
    if (mBound != nullptr)
    {
        groups = mBound->toolGroups();
    }
    else
    {
        mFallback = new QObject(this);
        groups = SMDesign::placeholderToolGroups(*mFallback);
    }

    for (const SMDesign::ToolGroup& group : groups)
    {
        if (group.actions.isEmpty() == false)
        {
            mGroupsLayout->addWidget(buildGroup(group.title, group.actions));
        }
    }

    mGroups->setEnabled(mActive && (mBound != nullptr));
    mBuilt = true;
}

QWidget* NaviFsmToolbar::buildGroup(const QString& title, const QList<QAction*>& actions)
{
    QWidget* group = new QWidget(mGroups);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(0, 0, 0, 4);
    groupLayout->setSpacing(2);

    const bool expanded = (mCollapsed.contains(title) == false);

    QToolButton* header = new QToolButton(group);
    header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    header->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    header->setText(title);
    header->setCheckable(true);
    header->setChecked(expanded);
    header->setAutoRaise(true);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setStyleSheet(QStringLiteral("QToolButton { text-align: left; font-weight: bold; border: none; }"));
    groupLayout->addWidget(header);

    QWidget* body = new QWidget(group);
    if (showsText())
    {
        // Row layout: full-width, left-aligned buttons, one per row.
        QVBoxLayout* rows = new QVBoxLayout(body);
        rows->setContentsMargins(12, 0, 2, 0);
        rows->setSpacing(1);
        for (QAction* action : actions)
        {
            rows->addWidget(buildToolButton(action));
        }
    }
    else
    {
        // Icon-only: flow the buttons into as many columns as the width allows (2-3).
        FlowLayout* flow = new FlowLayout(body, 0, 3, 3);
        flow->setContentsMargins(10, 0, 2, 0);
        for (QAction* action : actions)
        {
            flow->addWidget(buildToolButton(action));
        }
    }

    body->setVisible(expanded);
    groupLayout->addWidget(body);

    connect(header, &QToolButton::toggled, this, [this, header, body, title](bool checked) {
        body->setVisible(checked);
        header->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
        if (checked)
        {
            mCollapsed.remove(title);
        }
        else
        {
            mCollapsed.insert(title);
        }
    });

    return group;
}

QToolButton* NaviFsmToolbar::buildToolButton(QAction* action)
{
    QToolButton* button = new QToolButton(this);
    button->setDefaultAction(action);
    button->setToolButtonStyle(mStyle);
    button->setAutoRaise(true);

    if (showsText())
    {
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setStyleSheet(QStringLiteral("QToolButton { text-align: left; padding: 2px 6px; }"));
    }
    else
    {
        button->setIconSize(QSize(20, 20));
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    // A placement tool arms sticky on a double-click (single click already activated it).
    NESMDesign::eCanvasTool tool = NESMDesign::eCanvasTool::Select;
    if ((mBound != nullptr) && mBound->placementToolFor(action, tool))
    {
        button->installEventFilter(this);
        mStickyButtons.insert(button, static_cast<int>(tool));
    }

    return button;
}

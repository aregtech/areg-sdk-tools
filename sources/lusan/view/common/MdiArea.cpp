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
 *  \file        lusan/view/common/MdiArea.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application MdiMainWindow setup.
 *
 ************************************************************************/

#include "lusan/view/common/MdiArea.hpp"

#include <QApplication>
#include <QChildEvent>
#include <QEvent>
#include <QMdiSubWindow>
#include <QPalette>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>

MdiArea::MdiArea(QWidget* parent /*= nullptr*/)
    : QMdiArea(parent)
{
    setSizeAdjustPolicy(QMdiArea::SizeAdjustPolicy::AdjustIgnored);
    setDocumentMode(true);
    setViewMode(ViewMode::TabbedView);
    // Windows are maximized when created and stay so, a tab switch does not restore the one left.
    setOption(QMdiArea::AreaOption::DontMaximizeSubWindowOnActivation, true);
    setTabsClosable(true);
    setTabsMovable(true);
    setTabShape(QTabWidget::TabShape::Rounded);
    setTabPosition(QTabWidget::TabPosition::North);
    applyThemeBackground();

    connect(this, &QMdiArea::subWindowActivated, this, &MdiArea::onSubWindowActivated);
}

void MdiArea::changeEvent(QEvent* event)
{
    QMdiArea::changeEvent(event);
    if (event != nullptr)
    {
        const QEvent::Type type{ event->type() };
        if ((type == QEvent::Type::PaletteChange) || (type == QEvent::Type::ApplicationPaletteChange))
        {
            applyThemeBackground();
        }
    }
}

inline void MdiArea::applyThemeBackground(void)
{
    setBackground(QApplication::palette().brush(QPalette::ColorRole::Dark));
}

bool MdiArea::viewportEvent(QEvent* event)
{
    if ((event == nullptr) || (event->type() != QEvent::Type::ChildRemoved))
        return QMdiArea::viewportEvent(event);

    const QObject* removed{ static_cast<QChildEvent*>(event)->child() };
    const QList<QMdiSubWindow*> windows{ subWindowList() };
    if (std::find(windows.cbegin(), windows.cend(), removed) == windows.cend())
        return QMdiArea::viewportEvent(event);

    QMdiSubWindow* target{ nullptr };
    for (auto it = mActivationOrder.crbegin(); it != mActivationOrder.crend(); ++it)
    {
        QMdiSubWindow* window{ it->data() };
        if ((window != nullptr) && (window != removed) && (window->isHidden() == false))
        {
            target = window;
            break;
        }
    }

    mActivationOrder.removeIf([removed](const QPointer<QMdiSubWindow>& entry) { return (entry.isNull() || (entry.data() == removed)); });
    if (target == nullptr)
        return QMdiArea::viewportEvent(event);

    // The area picks the next window by its own rules, which are silenced and then overridden.
    QMdiSubWindow* before{ activeSubWindow() };
    QSignalBlocker blocker(this);
    const bool result{ QMdiArea::viewportEvent(event) };
    blocker.unblock();

    if (activeSubWindow() != target)
    {
        setActiveSubWindow(target);
    }
    else if (before != target)
    {
        emit subWindowActivated(target);
    }

    return result;
}

void MdiArea::onSubWindowActivated(QMdiSubWindow* window)
{
    if (window != nullptr)
    {
        mActivationOrder.removeIf([window](const QPointer<QMdiSubWindow>& entry) { return (entry.isNull() || (entry.data() == window)); });
        mActivationOrder.append(window);
    }
}

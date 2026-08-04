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
 *  \file        lusan/view/common/NaviDesignPanel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The FSM design Properties / Outline navigation window host.
 *
 ************************************************************************/

#include "lusan/view/common/NaviDesignPanel.hpp"
#include "lusan/view/common/NavigationDock.hpp"
#include "lusan/view/sm/SMDesign.hpp"
#include "lusan/view/sm/SMOutlinePanel.hpp"
#include "lusan/view/sm/SMPropertiesPanel.hpp"

#include <QAction>
#include <QLabel>
#include <QVBoxLayout>

NaviDesignPanel::NaviDesignPanel(NaviDesignPanel::eKind kind, MdiMainWindow* wndMain, QWidget* parent /*= nullptr*/)
    : NavigationWindow( static_cast<int>(kind == NaviDesignPanel::eKind::Properties
                            ? NavigationDock::eNaviWindow::NaviDesignProperties
                            : NavigationDock::eNaviWindow::NaviDesignOutline)
                      , wndMain, parent)
    , mKind     (kind)
    , mLayout   (nullptr)
    , mContent  (nullptr)
    , mBound    ( )
    , mEmpty    (true)
{
    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    // Starts empty until bound to an active Design page (issue #516).
    showPlaceholder();
}

void NaviDesignPanel::bindDesign(SMDesign* design)
{
    // mBound is a guarded pointer that reads nullptr once the page is gone, so "same page" alone
    // would keep a panel bound to a destroyed model. The empty flag tells the two apart.
    const bool unchanged = (design != nullptr) ? ((design == mBound.data()) && (mEmpty == false)) : mEmpty;
    if (unchanged && (mContent != nullptr))
    {
        // Already reflecting this Design page (or the same empty state); the hosted panel
        // tracks selection/model changes on its own, so there is nothing to rebuild.
        return;
    }

    mBound = design;
    delete mContent;
    mContent = nullptr;

    if (design == nullptr)
    {
        showPlaceholder();
        return;
    }

    mEmpty = false;
    if (mKind == NaviDesignPanel::eKind::Properties)
    {
        SMPropertiesPanel* properties = new SMPropertiesPanel(design->getModel(), this);
        // The docked copy drives the same actions the page owns, so both surfaces agree on what
        // is enabled and produce the same undo entries.
        properties->bindSubmachineActions(design->actionEnterSubmachine(), design->actionGoToParent()
                                          , design->actionAddSubmachine(), design->actionRemoveSubmachine());
        mContent = properties;
    }
    else
    {
        SMOutlinePanel* outline = new SMOutlinePanel(design->getModel(), design->getSceneManager(), this);
        // The outline's Rename/Delete context entries drive the bound page's own actions; the
        // Design page is the connection context, so a closed document tears the wiring down.
        connect(outline, &SMOutlinePanel::signalRenameRequested, design, [design](uint32_t) {
            if (design->actionRename() != nullptr)
            {
                design->actionRename()->trigger();
            }
        });
        connect(outline, &SMOutlinePanel::signalDeleteRequested, design, [design]() {
            design->deleteSelection();
        });
        mContent = outline;
    }

    mLayout->addWidget(mContent);
}

void NaviDesignPanel::showPlaceholder()
{
    mEmpty = true;
    QLabel* placeholder = new QLabel(tr("Open a State Machine Design page to use this panel."), this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);
    placeholder->setEnabled(false);
    mContent = placeholder;
    mLayout->addWidget(mContent);
}

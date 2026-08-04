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
 *  \file        lusan/view/sm/SMWhereUsedMenu.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared where-used popup for the FSM registry pages.
 *
 ************************************************************************/

#include "lusan/view/sm/SMWhereUsedMenu.hpp"

#include "lusan/model/sm/SMSelectionModel.hpp"

#include <QCursor>
#include <QMenu>
#include <QMessageBox>

void SMWhereUsedMenu::present(QWidget* parent, const QList<SMReferences::Use>& uses, SMSelectionModel& selection, const QString& name)
{
    if (uses.isEmpty())
    {
        QMessageBox::information(parent, QObject::tr("Where used"), QObject::tr("'%1' is not referenced anywhere.").arg(name));
        return;
    }

    QMenu menu(parent);
    for (const SMReferences::Use& use : uses)
    {
        QAction* action = menu.addAction(use.location);
        const uint32_t navId = use.navId;
        const bool isState = use.isState;
        // Ask the window and canvas to reveal the referencing element: it brings the Design page
        // forward, navigates to the element's level, then selects and centers it.
        QObject::connect(action, &QAction::triggered, &selection, [&selection, navId, isState]()
        {
            selection.requestReveal(navId, isState);
        });
    }

    menu.exec(QCursor::pos());
}

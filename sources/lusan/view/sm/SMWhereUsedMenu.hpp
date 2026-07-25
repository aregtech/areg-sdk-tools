#ifndef LUSAN_VIEW_SM_SMWHEREUSEDMENU_HPP
#define LUSAN_VIEW_SM_SMWHEREUSEDMENU_HPP
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
 *  \file        lusan/view/sm/SMWhereUsedMenu.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, shared where-used popup for the FSM registry pages.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMReferences.hpp"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QWidget;
class SMSelectionModel;

/**
 * \namespace   SMWhereUsedMenu
 * \brief   The single presentation of a where-used result across every FSM registry page and
 *          the Find Usages (Shift+F12) command, so the popup looks and navigates identically
 *          no matter which page raised it. Kept out of the individual pages to avoid five
 *          copies of the same menu-building code.
 **/
namespace SMWhereUsedMenu
{
    /**
     * \brief   Shows the where-used result as a popup at the cursor. Each entry, when chosen,
     *          selects the referencing state/transition in the shared selection model (the
     *          canvas reflects it). An empty result shows an information box instead.
     * \param   parent      The widget that owns the popup / message box.
     * \param   uses        The reference sites, as returned by SMWhereUsed::collect.
     * \param   selection   The document-wide selection model that navigation drives.
     * \param   name        The entry name, used only for the empty-result message.
     **/
    void present(QWidget* parent, const QList<SMReferences::Use>& uses, SMSelectionModel& selection, const QString& name);
}

#endif  // LUSAN_VIEW_SM_SMWHEREUSEDMENU_HPP

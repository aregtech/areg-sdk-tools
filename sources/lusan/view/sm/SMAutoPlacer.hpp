#ifndef LUSAN_VIEW_SM_SMAUTOPLACER_HPP
#define LUSAN_VIEW_SM_SMAUTOPLACER_HPP
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
 *  \file        lusan/view/sm/SMAutoPlacer.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM canvas automatic placement of missing layout.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/sm/SMLayoutData.hpp"

#include <QList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class StateMachineData;

/**
 * \brief   Automatic placement of states whose Node layout entry is missing, as happens
 *          for hand-written or hand-stripped documents. The result is a plain,
 *          non-overlapping arrangement per machine level; it is not a graph layout.
 **/
namespace SMAutoPlacer
{
    /**
     * \brief   Computes a Node entry for every state of every machine level that has none,
     *          placed on a free grid cell of its level so it overlaps no existing box.
     * \param   data    The document to inspect; it is not modified.
     * \return  The Node entries to add, empty when the document has complete layout.
     **/
    QList<SMLayoutNode> missingNodes(const StateMachineData& data);
}

#endif  // LUSAN_VIEW_SM_SMAUTOPLACER_HPP

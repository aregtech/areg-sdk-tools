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
 *  \file        lusan/view/sm/SMAutoPlacer.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM canvas automatic placement of missing layout.
 *
 ************************************************************************/

#include "lusan/view/sm/SMAutoPlacer.hpp"

#include "lusan/data/sm/SMState.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/view/sm/NESMDesign.hpp"

#include <QRectF>

#include <algorithm>
#include <cmath>

namespace
{
    //!< The free space kept between two automatically placed boxes.
    constexpr double PlacementGap{ 40.0 };

    //!< The top-left origin of automatic placement: four grid cells off the corner, so the
    //!< auto-placed Start state still reads as the machine's entry point.
    constexpr double PlacementOriginX{ 64.0 };
    constexpr double PlacementOriginY{ 64.0 };

    //!< Upper bound of grid cells scanned per state; a level cannot be crowded beyond this.
    constexpr int PlacementCellLimit{ 4096 };

    QRectF boxOf(const SMLayoutNode& node)
    {
        return QRectF(  node.x, node.y
                      , std::max(node.width, NESMDesign::StateMinWidth)
                      , std::max(node.height, NESMDesign::StateMinHeight));
    }

    bool isFree(const QList<QRectF>& occupied, const QRectF& box)
    {
        for (const QRectF& taken : occupied)
        {
            if (taken.intersects(box))
            {
                return false;
            }
        }

        return true;
    }

    void placeLevel(const SMStateData& level, const SMLayoutData& layout, int gridSize, QList<SMLayoutNode>& result)
    {
        const QList<SMStateEntry*>& states = level.getElements();

        QList<QRectF>               occupied;
        QList<const SMStateEntry*>  missing;
        QRectF                      startBox;
        for (const SMStateEntry* state : states)
        {
            const SMLayoutNode* node = layout.findNode(state->getId());
            if (node != nullptr)
            {
                occupied.append(boxOf(*node));
                if (state->getKind() == SMStateEntry::eStateKind::Start)
                {
                    startBox = occupied.last();
                }
            }
            else if (state->getKind() == SMStateEntry::eStateKind::Start)
            {
                // The Start state takes the first (top-left) cell: the entry point reads
                // top-left even when it is not the first element in document order.
                missing.prepend(state);
            }
            else
            {
                missing.append(state);
            }
        }

        const double cellWidth  = NESMDesign::StateDefaultWidth  + PlacementGap;
        const double cellHeight = NESMDesign::StateDefaultHeight + PlacementGap;
        const int    columns    = std::max(1, static_cast<int>(std::ceil(std::sqrt(static_cast<double>(states.size())))));

        // The History marker is placed right after the Start it belongs beside, so no later state
        // can take the cell reserved for it.
        for (int i = 1; i < missing.size(); ++i)
        {
            if (missing.at(i)->getKind() == SMStateEntry::eStateKind::History)
            {
                const SMStateEntry* historyMarker = missing.takeAt(i);
                const bool startFirst = (missing.isEmpty() == false)
                                        && (missing.first()->getKind() == SMStateEntry::eStateKind::Start);
                missing.insert(startFirst ? 1 : 0, historyMarker);
                break;
            }
        }

        int cell = 0;
        for (const SMStateEntry* state : missing)
        {
            // Start / Final marker states use the compact pill box size; a History marker is square.
            const bool   marker  = (state->getKind() != SMStateEntry::eStateKind::Normal);
            const bool   history = (state->getKind() == SMStateEntry::eStateKind::History);
            const double width   = history ? NESMDesign::HistoryMarkerSize
                                           : (marker ? NESMDesign::MarkerStateWidth  : NESMDesign::StateDefaultWidth);
            const double height  = history ? NESMDesign::HistoryMarkerSize
                                           : (marker ? NESMDesign::MarkerStateHeight : NESMDesign::StateDefaultHeight);

            QRectF box;
            if (history && (startBox.isNull() == false))
            {
                // Under the level's entry point: Start and the marker answer the same question,
                // how this level begins -- fresh, or resumed where it left off.
                const QRectF docked{ NESMDesign::snapValue(startBox.center().x() - width / 2.0, gridSize)
                                   , NESMDesign::snapValue(startBox.bottom() + PlacementGap, gridSize)
                                   , width, height };
                if (isFree(occupied, docked))
                {
                    box = docked;
                }
            }

            bool placed = (box.isNull() == false);
            while ((placed == false) && (cell < PlacementCellLimit))
            {
                const double x = NESMDesign::snapValue(PlacementOriginX + (cell % columns) * cellWidth, gridSize);
                const double y = NESMDesign::snapValue(PlacementOriginY + (cell / columns) * cellHeight, gridSize);
                ++cell;

                box = QRectF(x, y, width, height);
                placed = isFree(occupied, box);
            }

            occupied.append(box);
            if (state->getKind() == SMStateEntry::eStateKind::Start)
            {
                startBox = box;
            }

            SMLayoutNode node;
            node.owner  = state->getId();
            node.x      = box.x();
            node.y      = box.y();
            node.width  = box.width();
            node.height = box.height();
            result.append(node);
        }

        for (const SMStateEntry* state : states)
        {
            if (state->hasNestedStates())
            {
                placeLevel(*state->getNestedStates(), layout, gridSize, result);
            }
        }
    }
}

QList<SMLayoutNode> SMAutoPlacer::missingNodes(const StateMachineData& data)
{
    QList<SMLayoutNode> result;
    placeLevel(data.getStates(), data.getLayout(), data.getLayout().getGridSize(), result);
    return result;
}

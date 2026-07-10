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

    //!< The top-left origin of automatic placement: comfortably off the (0;0) corner
    //!< (four default grid cells) but still clearly in the top-left region, so the
    //!< auto-placed Start state reads as the machine's entry point.
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
        for (const SMStateEntry* state : states)
        {
            const SMLayoutNode* node = layout.findNode(state->getId());
            if (node != nullptr)
            {
                occupied.append(boxOf(*node));
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

        int cell = 0;
        for (const SMStateEntry* state : missing)
        {
            // Start / Final marker states use the compact pill box size.
            const bool   marker = (state->getKind() != SMStateEntry::eStateKind::Normal);
            const double width  = (marker ? NESMDesign::MarkerStateWidth  : NESMDesign::StateDefaultWidth);
            const double height = (marker ? NESMDesign::MarkerStateHeight : NESMDesign::StateDefaultHeight);

            QRectF box;
            while (cell < PlacementCellLimit)
            {
                const double x = NESMDesign::snapValue(PlacementOriginX + (cell % columns) * cellWidth, gridSize);
                const double y = NESMDesign::snapValue(PlacementOriginY + (cell / columns) * cellHeight, gridSize);
                ++cell;

                box = QRectF(x, y, width, height);
                if (isFree(occupied, box))
                {
                    break;
                }
            }

            occupied.append(box);

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

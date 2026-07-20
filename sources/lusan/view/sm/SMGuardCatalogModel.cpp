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
 *  \file        lusan/view/sm/SMGuardCatalogModel.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog model (spec 8, SM-21-04).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardCatalogModel.hpp"

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMGuardCatalogModel::SMGuardCatalogModel(QObject* parent /*= nullptr*/)
    : QAbstractTableModel   (parent)
    , mSymbols              ( )
    , mUseCounts            ( )
{
}

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////

void SMGuardCatalogModel::setSymbols(const QList<SMGuardSymbol>& symbols)
{
    beginResetModel();
    mSymbols = symbols;
    endResetModel();
}

void SMGuardCatalogModel::setUseCounts(const QHash<uint32_t, int>& counts)
{
    mUseCounts = counts;
    if (mSymbols.isEmpty() == false)
    {
        // Only the used-N column changes; refresh it in place (no structural reset).
        const QModelIndex topLeft = index(0, ColUsed);
        const QModelIndex bottomRight = index(static_cast<int>(mSymbols.size()) - 1, ColUsed);
        emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole });
    }
}

const SMGuardSymbol* SMGuardCatalogModel::symbolAt(int row) const
{
    return ((row >= 0) && (row < mSymbols.size())) ? &mSymbols.at(row) : nullptr;
}

int SMGuardCatalogModel::useCountOf(uint32_t symbolId) const
{
    return (symbolId != 0u) ? mUseCounts.value(symbolId, 0) : 0;
}

//////////////////////////////////////////////////////////////////////////
// QAbstractTableModel overrides
//////////////////////////////////////////////////////////////////////////

int SMGuardCatalogModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return parent.isValid() ? 0 : static_cast<int>(mSymbols.size());
}

int SMGuardCatalogModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return parent.isValid() ? 0 : static_cast<int>(ColCount);
}

QVariant SMGuardCatalogModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if ((index.isValid() == false) || (index.row() >= mSymbols.size()))
    {
        return QVariant();
    }

    const SMGuardSymbol& sym = mSymbols.at(index.row());

    if ((role == Qt::DisplayRole) || (role == Qt::UserRole))
    {
        switch (index.column())
        {
        case ColHue:
            return sym.glyph;

        case ColName:
            return sym.name;

        case ColType:
            return sym.typeText;

        case ColUsed:
            {
                const int count = useCountOf(sym.symbolId);
                // The sort key stays a number; the shown text is blank at 0 (quiet catalog).
                if (role == Qt::UserRole)
                {
                    return count;
                }

                return (count > 0) ? QString::number(count) : QString();
            }

        default:
            break;
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        QString tip = sym.display();
        if (sym.provenance.isEmpty() == false)
        {
            tip += QStringLiteral("  --  ") + sym.provenance;
        }

        return tip;
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == ColUsed)
        {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

QVariant SMGuardCatalogModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
    {
        return QVariant();
    }

    switch (section)
    {
    case ColHue:    return QString();
    case ColName:   return tr("Name");
    case ColType:   return tr("Type");
    case ColUsed:   return tr("Used");
    default:        return QVariant();
    }
}

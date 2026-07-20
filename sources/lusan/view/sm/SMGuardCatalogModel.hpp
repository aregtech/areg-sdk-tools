#ifndef LUSAN_VIEW_SM_SMGUARDCATALOGMODEL_HPP
#define LUSAN_VIEW_SM_SMGUARDCATALOGMODEL_HPP
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
 *  \file        lusan/view/sm/SMGuardCatalogModel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Data catalog model (spec 8, SM-21-04).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractTableModel>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QHash>
#include <QList>
#include <cstdint>

/**
 * \class   SMGuardCatalogModel
 * \brief   The Data catalog's model (spec 8): a `QAbstractTableModel` that WRAPS the existing
 *          `SMGuardCatalog::build` symbol enumeration (it does not re-enumerate) and exposes
 *          it in four columns -- `[hue/glyph | name | type | used-N]`. The `used-N` column is a
 *          per-symbol bound-reference count the host computes over the CURRENT guard tree and
 *          pushes in via \ref setUseCounts, refreshed on every commit and re-projection
 *          (D-HILITE / A2: bound refs only). The model is sortable (a UserRole sort key per
 *          column) and filterable (through a proxy over the name column).
 **/
class SMGuardCatalogModel : public QAbstractTableModel
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////////
public:
    enum eColumn
    {
          ColHue    = 0     //!< The owner/kind glyph.
        , ColName           //!< The declared name.
        , ColType           //!< The declared type (return type for a call).
        , ColUsed           //!< The bound-reference count in the current guard.
        , ColCount          //!< The column count sentinel.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardCatalogModel(QObject* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    //!< Replaces the symbol universe (the view rebuilds it when the transition changes).
    void setSymbols(const QList<SMGuardSymbol>& symbols);

    //!< Sets the per-symbol-id bound-reference counts shown in the `used-N` column.
    void setUseCounts(const QHash<uint32_t, int>& counts);

    //!< The symbol on \p row (0-based over the unfiltered model), or nullptr when out of range.
    const SMGuardSymbol* symbolAt(int row) const;

//////////////////////////////////////////////////////////////////////////
// QAbstractTableModel overrides
//////////////////////////////////////////////////////////////////////////
public:
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    //!< The current use count for \p symbolId (0 when absent).
    int useCountOf(uint32_t symbolId) const;

    QList<SMGuardSymbol>    mSymbols;   //!< The wrapped enumeration.
    QHash<uint32_t, int>    mUseCounts; //!< symbol id -> bound-reference count in the guard.
};

#endif  // LUSAN_VIEW_SM_SMGUARDCATALOGMODEL_HPP

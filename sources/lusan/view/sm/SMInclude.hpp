#ifndef LUSAN_VIEW_SM_SMINCLUDE_HPP
#define LUSAN_VIEW_SM_SMINCLUDE_HPP
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
 *  \file        lusan/view/sm/SMInclude.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

#include <QScrollArea>
#include "lusan/view/common/TableCell.hpp"

#include <QStringList>
#include <cstdint>

class IncludeDetailsView;
class IncludeEntry;
class IncludeListView;
class QEvent;
class QModelIndex;
class QTreeWidgetItem;
class SMIncludeModel;

/**
 * \brief   The FSM Includes page: header include locations, the counterpart of the Service
 *          Interface Includes page. The page splits into two equal-width
 *          panels — the multi-column include list (Location, Type, Name, Version) on the
 *          left and the selected-include details editor on the right. Every edit is
 *          committed through SMIncludeModel's undo commands; the page mutates no model state
 *          directly and rebuilds the list from the live model on every Include-kind notifier
 *          signal, self-triggered or from undo/redo alike.
 **/
class SMInclude : public    QScrollArea
                , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMInclude(SMIncludeModel& model, QWidget* parent = nullptr);
    virtual ~SMInclude() = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    /**
     * \brief   Returns the number of columns in the include list (IETableHelper).
     **/
    int getColumnCount() const override;

    /**
     * \brief   Returns the text of the given list cell (IETableHelper).
     * \param   cell    The index of the cell.
     **/
    QString getCellText(const QModelIndex& cell) const override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddClicked();
    void onInsertClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onLocationCommitted();

    //!< Commits an inline edit of the Location column into the model (undo command); the derived
    //!< Type/Name columns then refresh from the notifier. Escape is handled by the delegate.
    void onInlineLocationEdited(const QModelIndex& index, const QString& newValue);

    void onBrowseClicked();
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();

    //!< Re-derives the Type/Name/Version columns from the current include locations.
    void onUpdateClicked();

    //!< Rebuilds the whole list on any Include-kind notifier signal.
    void onNotifierChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the include by ID; returns false if not found (and selects nothing).
    bool selectInclude(uint32_t id);
    //!< Populates the details panel for the given include.
    void selectedInclude(const IncludeEntry* entry);
    //!< Clears the details panel and disables the row-only tool buttons.
    void showClean();

    void setNodeText(QTreeWidgetItem* node, const IncludeEntry& entry) const;
    void updateMoveButtons(int row, int rowCount);

    //!< The include ID stored on the currently selected row, or 0 if none.
    uint32_t currentIncludeId() const;
    QString genName();

    static QStringList getSupportedExtensions();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMIncludeModel&     mModel;
    IncludeListView*    mList;
    IncludeDetailsView* mDetails;
    TableCell*          mTableCell;     //!< Inline editor delegate for the Location column.
    uint32_t            mNameCounter;
};

#endif  // LUSAN_VIEW_SM_SMINCLUDE_HPP

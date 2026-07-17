#ifndef LUSAN_VIEW_SM_SMATTRIBUTE_HPP
#define LUSAN_VIEW_SM_SMATTRIBUTE_HPP
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
 *  \file        lusan/view/sm/SMAttribute.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page.
 *
 ************************************************************************/

#include "lusan/view/common/TableCell.hpp"

#include <QScrollArea>
#include <cstdint>

class AttributeDetailsView;
class AttributeListView;
class QEvent;
class QModelIndex;
class QTreeWidgetItem;
class SMAttributeEntry;
class SMAttributeModel;
class TableCell;

/**
 * \brief   The FSM Attributes page: typed internal machine variables with a default value
 *          and no update subscriptions.
 *          Every edit is committed through SMAttributeModel's undo commands; the page
 *          mutates no model state directly, and refreshes by rebuilding the list from the
 *          live model on every relevant DocModelNotifier signal (self-triggered or
 *          external/undo alike) rather than patching individual rows.
 **/
class SMAttribute : public    QScrollArea
                  , protected IETableHelper
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMAttribute(SMAttributeModel& model, QWidget* parent = nullptr);
    virtual ~SMAttribute() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel (its Add button lets a caller start a new attribute,
     *          e.g. from the Design page's Declare dropdown).
     **/
    AttributeListView* getList() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    //!< IETableHelper: number of columns in the shared attribute list (for the inline delegate).
    int getColumnCount() const override;
    //!< IETableHelper: current text of the given list cell (used to seed and cancel inline edits).
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

    void onNameCommitted();
    void onTypeChanged(int index);
    void onValueCommitted();
    //!< Mirrors the value into the list row and re-validates while the user types; the
    //!< model commit still happens once on editing-finished.
    void onValueTextChanged(const QString& text);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();

    //!< Routes an inline (in-table) edit of the Name/Type/Value column back to the model. The
    //!< commit is deferred to the next event-loop turn so the notifier-driven list rebuild does
    //!< not run while the delegate editor is still closing.
    void onEditorDataChanged(const QModelIndex& index, const QString& newValue);

    //!< Rebuilds the whole list on any Attribute-kind notifier signal.
    void onNotifierChanged();
    //!< Repopulates the type combo and re-validates the current value on any DataType-kind
    //!< notifier signal (a declared type may have been added, renamed, converted or removed).
    void onDataTypesChanged();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    //!< Rebuilds the whole list from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the attribute by ID; returns false if not found (and selects nothing).
    bool selectAttribute(uint32_t id);
    //!< Populates the details panel for the given attribute.
    void selectedAttribute(const SMAttributeEntry* entry);
    //!< Clears the details panel and disables the row-only tool buttons.
    void showClean();

    //!< Configures the value control (free-text, enumerator picker, or disabled) and shows
    //!< the current literal's validation hint for the given attribute.
    void updateValueControl(const SMAttributeEntry* entry);
    //!< Re-evaluates and displays the value hint for the given type/literal pair.
    void updateValueValidation(const QString& typeName, const QString& value);
    //!< Empty string if the literal is valid for the type (a missing value always is),
    //!< otherwise a short, user-facing reason.
    QString valueValidationReason(const QString& typeName, const QString& value) const;
    //!< Fills a list row's columns, flagging an invalid stored value with a warning icon.
    void setNodeText(QTreeWidgetItem* node, const SMAttributeEntry& entry) const;

    void populateTypeCombo();
    void updateMoveButtons(int row, int rowCount);

    //!< Installs the shared TableCell delegate on the Name/Type/Value columns so the list rows
    //!< can be edited inline, exactly like the details panel fields.
    void setupInlineEditing();
    //!< Commits a single inline edit (by attribute ID) through the model's undo commands. A
    //!< no-op edit (same value) is skipped so it does not push a redundant undo step.
    void commitInlineEdit(uint32_t id, int col, const QString& newValue);

    //!< The attribute ID stored on the currently selected row, or 0 if none.
    uint32_t currentAttributeId() const;
    QString genName();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMAttributeModel&    mModel;
    AttributeListView*   mList;
    AttributeDetailsView* mDetails;
    TableCell*           mTableCell;    //!< Inline (in-table) cell editor delegate for the list.
    uint32_t             mNameCounter;
};

#endif  // LUSAN_VIEW_SM_SMATTRIBUTE_HPP

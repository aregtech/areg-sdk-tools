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

#include <QScrollArea>
#include <cstdint>

class QEvent;
class QTreeWidgetItem;
class SMAttributeDetails;
class SMAttributeEntry;
class SMAttributeList;
class SMAttributeModel;

/**
 * \brief   The FSM Attributes page: typed internal machine variables with a default value
 *          and no update subscriptions.
 *          Every edit is committed through SMAttributeModel's undo commands; the page
 *          mutates no model state directly, and refreshes by rebuilding the list from the
 *          live model on every relevant DocModelNotifier signal (self-triggered or
 *          external/undo alike) rather than patching individual rows.
 **/
class SMAttribute : public QScrollArea
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
    SMAttributeList* getList() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

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

    //!< The attribute ID stored on the currently selected row, or 0 if none.
    uint32_t currentAttributeId() const;
    QString genName();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMAttributeModel&   mModel;
    SMAttributeList*    mList;
    SMAttributeDetails* mDetails;
    uint32_t            mNameCounter;
};

#endif  // LUSAN_VIEW_SM_SMATTRIBUTE_HPP

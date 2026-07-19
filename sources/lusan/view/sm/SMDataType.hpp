#ifndef LUSAN_VIEW_SM_SMDATATYPE_HPP
#define LUSAN_VIEW_SM_SMDATATYPE_HPP
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
 *  \file        lusan/view/sm/SMDataType.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Data Types page.
 *
 ************************************************************************/

#include <QScrollArea>
#include "lusan/data/common/DataTypeBase.hpp"

class DataTypeCustom;
class DataTypeEnum;
class DataTypeImported;
class DataTypeContainer;
class DataTypeStructure;
class DocumentElem;
class ElementBase;
class DataTypeDetailsView;
class DataTypeFieldDetailsView;
class DataTypeListView;
class QComboBox;
class QTreeWidgetItem;
class SMDataTypeModel;

/**
 * \brief   The FSM Data Types page: enumerations, structures, imported types and containers
 *          Every edit is committed through SMDataTypeModel's undo commands; the page
 *          mutates no model state directly, and refreshes by rebuilding the tree from the
 *          live model on every relevant DocModelNotifier signal (self-triggered or
 *          external/undo alike) rather than patching individual tree nodes.
 **/
class SMDataType : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMDataType(SMDataTypeModel& model, QWidget* parent = nullptr);
    virtual ~SMDataType() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the list panel (its Add button lets a caller start a new data type,
     *          e.g. from the Design page's Declare dropdown).
     **/
    DataTypeListView* getList() const;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddClicked();
    void onInsertClicked();
    void onRemoveClicked();
    void onAddFieldClicked();
    void onInsertFieldClicked();
    void onRemoveFieldClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();

    void onNameCommitted();
    void onStructSelected(bool checked);
    void onEnumSelected(bool checked);
    void onImportSelected(bool checked);
    void onContainerSelected(bool checked);
    void onEnumDerivedChanged(int index);
    void onImportLocationCommitted();
    void onImportNamespaceCommitted();
    void onImportObjectCommitted();
    void onImportBrowse();
    void onContainerObjectChanged(int index);
    void onContainerKeyChanged(int index);
    void onContainerValueChanged(int index);
    void onDeprecatedToggled(bool checked);
    void onDeprecateHintCommitted();

    void onFieldNameCommitted();
    void onFieldTypeChanged(int index);
    void onFieldValueCommitted();
    void onFieldDeprecatedToggled(bool checked);
    void onFieldDeprecateHintCommitted();

    void onNotifierChanged();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildUi();
    void setupSignals();

    //!< Rebuilds the whole tree from the live model and restores the selection by ID.
    void refreshAll();
    //!< Selects the data type / field by ID; returns false if not found (and selects nothing).
    bool selectDataType(uint32_t typeId, uint32_t fieldId = 0);
    //!< Clears the details/fields panels and disables the field-only tool buttons.
    void showClean();

    QTreeWidgetItem* createNode(DataTypeCustom* dataType) const;
    void setNodeText(QTreeWidgetItem* node, const DocumentElem* elem) const;
    //!< Empty string if a structure field's default is valid for its type (a missing value,
    //!< or a declared enum/structure/container/imported type, is always valid), otherwise a
    //!< short, user-facing reason.
    QString validateFieldValue(const QString& typeName, const QString& value) const;

    void selectedStruct(DataTypeStructure* dataType);
    void selectedEnum(DataTypeEnum* dataType);
    void selectedImport(DataTypeImported* dataType);
    void selectedContainer(DataTypeContainer* dataType);
    void selectedStructField(DataTypeStructure* parent, uint32_t fieldId);
    void selectedEnumField(DataTypeEnum* parent, uint32_t fieldId);

    void activateFields(bool activate);
    void updateMoveButtons(int row, int rowCount);

    void populateTypeCombo(QComboBox* combo, const DataTypeCustom* exclude) const;
    void populateIntegerCombo(QComboBox* combo) const;
    void populateContainerObjectCombo(QComboBox* combo) const;

    //!< The data type / field currently selected in the tree, or nullptr / 0.
    DataTypeCustom* currentDataType() const;
    uint32_t currentFieldId() const;

    QString genTypeName();
    QString genFieldName(const DataTypeCustom* dataType) const;

    //!< Creates and selects a new data type of the given category (the Add split button and
    //!< its category drop-down entries all route here).
    void addNewType(DataTypeBase::eCategory category);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMDataTypeModel&        mModel;
    DataTypeListView*       mList;
    DataTypeDetailsView*    mDetails;
    DataTypeFieldDetailsView* mFields;
    uint32_t                mNameCounter;
    QString                 mCurUrl;
    QString                 mCurFile;
};

#endif  // LUSAN_VIEW_SM_SMDATATYPE_HPP

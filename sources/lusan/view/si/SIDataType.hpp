#ifndef LUSAN_APPLICATION_SI_SIDATATYPE_HPP
#define LUSAN_APPLICATION_SI_SIDATATYPE_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataType.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>
#include "lusan/data/common/DataTypeBase.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"

class DataTypeBasicContainer;
class DataTypeCustom;
class DataTypeContainer;
class DataTypeContainer;
class DataTypeEnum;
class DataTypeImported;
class DataTypePrimitive;
class DataTypeStructure;
class DataTypesModel;
class EnumEntry;
class FieldEntry;
class SIDataTypeData;
class SIDataTypeDetails;
class SIDataTypeList;
class SIDataTypeFieldDetails;
class SIDataTypeModel;
class TableCell;
class QTreeWidgetItem;

namespace Ui {
    class SIDataType;
}

class SIDataTypeWidget : public QWidget
{
    friend class SIDataType;

    Q_OBJECT

public:
    explicit SIDataTypeWidget(QWidget* parent);

private:
    Ui::SIDataType * ui;
};

class SIDataType    : public QScrollArea
                    , public IEDataTypeConsumer
{
    Q_OBJECT
    
public:
    explicit SIDataType(SIDataTypeModel & model, QWidget *parent = nullptr);

    virtual ~SIDataType(void);

signals:

    void signalDataTypeCreated(DataTypeCustom* dataType);

    void signalDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    void signalDataTypeRemoved(DataTypeCustom* dataType);

    void signalDataTypeUpdated(DataTypeCustom* dataType);

protected:
    
    /**
     * \brief Triggered when the current cell is changed.
     * \param currentRow The current row index.
     * \param currentColumn The current column index.
     * \param previousRow The previous row index.
     * \param previousColumn The previous column index.
     */
    void onCurCellChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    /**
     * \brief Triggered when the add button is clicked.
     */
    void onAddClicked(void);
    
    void onAddFieldClicked(void);

    /**
     * \brief Triggered when the remove button is clicked.
     */
    void onRemoveClicked(void);

    /**
     * \brief Triggered when the name is changed.
     * \param newName The new name of the attribute.
     */
    void onTypeNameChanged(const QString& newName);
    
    /**
     * \brief   Triggered when the deprecated flag is changed.
     * \param   isChecked   The flag to indicate if the constant is deprecated.
     **/
    void onDeprectedChecked(bool isChecked);

    /**
     * \brief   Triggered when the deprecation hint is changed.
     * \param   newText The new deprecation hint.
     **/
    void onDeprecateHintChanged(const QString& newText);

    /**
     * \brief   Triggered when the description is changed.
     **/
    void onDescriptionChanged(void);

    void onStructSelected(bool checked);

    void onEnumSelected(bool checked);

    void onImportSelected(bool checked);

    void onContainerSelected(bool checked);
    
    void onContainerObjectChanged(int index);
    
    void onContainerKeyChanged(int index);
    
    void onContainerValueChanged(int index);

    void onEnumDerivedChanged(int index);

    void onImportLocationChanged(const QString& newText);

    void onImportNamespaceChanged(const QString& newText);

    void onImportObjectChanged(const QString& newText);

    void onImportLocationBrowse(void);

    void onFieldNameChanged(const QString& newName);

    void onFieldTypeChanged(int index);

    void onFieldValueChanged(const QString& newValue);

    void onFieldDescriptionChanged(void);

    void onFieldDeprecatedChecked(bool isChecked);

    void onFieldDeprecateHint(const QString& newText);
    
private:

    void convertDataType(QTreeWidgetItem* current, DataTypeBase::eCategory newCategory);

    void showEnumDetails(bool show);

    void showImportDetails(bool show);

    void showContainerDetails(bool show);
    
    /**
     * \brief Updates the data in the table.
     */
    void updateData(void);
    
    /**
     * \brief Updates the widgets.
     */
    void updateWidgets(void);
    
    /**
     * \brief Initializes the signals.
     */
    void setupSignals(void);
    
    /**
     * \brief Blocks the basic signals.
     * \param doBlock If true, blocks the signals, otherwise unblocks.
     */
    void blockBasicSignals(bool doBlock);

    void selectedStruct(DataTypeCustom* oldType, DataTypeStructure* dataType);

    void selectedEnum(DataTypeCustom* oldType, DataTypeEnum* dataType);

    void selectedImport(DataTypeCustom* oldType, DataTypeImported* dataType);

    void selectedContainer(DataTypeCustom* oldType, DataTypeContainer* dataType);

    void selectedStructField(DataTypeCustom* oldType, const FieldEntry& field, DataTypeStructure* parent);

    void selectedEnumField(DataTypeCustom* oldType, const EnumEntry& field, DataTypeEnum* parent);

    QTreeWidgetItem* createNodeStructure(DataTypeStructure* dataType) const;

    QTreeWidgetItem* createNodeEnum(DataTypeEnum* dataType) const;

    QTreeWidgetItem* createNodeImported(DataTypeImported* dataType) const;
    
    QTreeWidgetItem* createNodeContainer(DataTypeContainer* dataType) const;

    void updateNodeStructure(QTreeWidgetItem* node, DataTypeStructure* dataType) const;

    void updateNodeEnum(QTreeWidgetItem* node, DataTypeEnum* dataType) const;

    void updateNodeImported(QTreeWidgetItem* node, DataTypeImported* dataType) const;

    void updateNodeContainer(QTreeWidgetItem* node, DataTypeContainer* dataType) const;

    void updateChildNodeStruct(QTreeWidgetItem* child, DataTypeStructure* dataType, const FieldEntry& field) const;

    void updateChildNodeEnum(QTreeWidgetItem* child, DataTypeEnum* dataType, const EnumEntry& field) const;

    void activateFields(bool activate);

    void updateContainerNames(QTreeWidgetItem* node, DataTypeContainer *dataType) const;

    void updateImportNames(QTreeWidgetItem* node, DataTypeImported* dataType) const;

    inline ElementBase* getSelectedField(void) const;
    
    static const QList<DataTypeBasicContainer *> & _getContainerTypes(void);
    
    static const QList<DataTypeBase *>& _getIntegerTypes(void);
    
    static const QList<DataTypeBase *>& _getPredefinedTypes(void);

private:
    SIDataTypeDetails*      mDetails;
    SIDataTypeList*         mList;
    SIDataTypeFieldDetails* mFields;
    SIDataTypeWidget*       mWidget;
    Ui::SIDataType &        ui;
    SIDataTypeModel&        mModel;
    DataTypesModel*         mTypeModel;
    TableCell*              mTableCell; //!< The table cell object.
    QString                 mCurUrl;
    QString                 mCurFile;
    int                     mCurView;

    uint32_t                mCount;
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPE_HPP

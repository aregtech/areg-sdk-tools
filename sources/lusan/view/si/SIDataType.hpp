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

    /**
     * \brief Triggered when the add field button is clicked.
     */
    void onAddFieldClicked(void);

    /**
     * \brief Triggered when the remove button is clicked.
     */
    void onRemoveClicked(void);

    /**
     * \brief Triggered when the remove field button is clicked.
     */
    void onRemoveFieldClicked(void);

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

    /**
     * \brief   Triggered when the struct is selected.
     * \param   checked The flag to indicate if the struct is selected.
     **/
    void onStructSelected(bool checked);

    /**
     * \brief   Triggered when the enum is selected.
     * \param   checked The flag to indicate if the enum is selected.
     **/
    void onEnumSelected(bool checked);

    /**
     * \brief   Triggered when the import is selected.
     * \param   checked The flag to indicate if the import is selected.
     **/
    void onImportSelected(bool checked);

    /**
     * \brief   Triggered when the container is selected.
     * \param   checked The flag to indicate if the container is selected.
     **/
    void onContainerSelected(bool checked);

    /**
     * \brief   Triggered when the container object is changed,
     *          i.e. other container object is selected.
     * \param   index The index of the container object.
     **/
    void onContainerObjectChanged(int index);

    /**
     * \brief   Triggered when the container key is changed,
     * \param   index The index of the container key.
     **/
    void onContainerKeyChanged(int index);

    /**
     * \brief   Triggered when the container value is changed,
     * \param   index The index of the container value.
     **/
    void onContainerValueChanged(int index);

    /**
     * \brief   Triggered when the enum derived is changed,
     * \param   index The index of the enum derived.
     **/
    void onEnumDerivedChanged(int index);

    /**
     * \brief   Triggered when the import location is changed,
     * \param   newText The new location.
     **/
    void onImportLocationChanged(const QString& newText);

    /**
     * \brief   Triggered when the import namespace is changed,
     * \param   newText The new namespace.
     **/
    void onImportNamespaceChanged(const QString& newText);

    /**
     * \brief   Triggered when the import object is changed,
     * \param   newText The new object.
     **/
    void onImportObjectChanged(const QString& newText);

    /**
     * \brief   Triggered when the import location browse button is clicked.
     **/
    void onImportLocationBrowse(void);

    /**
     * \brief   Triggered when the field name is changed.
     * \param   newName The new name of the field.
     **/
    void onFieldNameChanged(const QString& newName);

    /**
     * \brief   Triggered when the field type is changed.
     * \param   index The index of the field type.
     **/
    void onFieldTypeChanged(int index);

    /**
     * \brief   Triggered when the field value is changed.
     * \param   newValue The new value of the field.
     **/
    void onFieldValueChanged(const QString& newValue);

    /**
     * \brief   Triggered when the field description is changed.
     **/
    void onFieldDescriptionChanged(void);

    /**
     * \brief   Triggered when the field deprecated flag is changed.
     * \param   isChecked The flag to indicate if the field is deprecated.
     **/
    void onFieldDeprecatedChecked(bool isChecked);

    /**
     * \brief   Triggered when the field deprecation hint is changed.
     * \param   newText The new deprecation hint.
     **/
    void onFieldDeprecateHint(const QString& newText);

protected:

    /**
     * \brief   Triggered when new data type is created.
     * \param   dataType    New created data type object.
     * \return  Returns true if new created data type is in the list. Otherwise, returns false.
     **/
    virtual void dataTypeCreated(DataTypeCustom* dataType) override;

    /**
     * \brief   Triggered when the data type is converted.
     * \param   oldType     The old data type object.
     * \param   newType     The new data type object.
     * \return  Returns true if the old data type is converted to the new data type. Otherwise, returns false.
     **/
    virtual void dataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType) override;

    /**
     * \brief   Triggered when the data type is deleted and invalidated.
     * \param   dataType    The data type object to be deleted.
     * \return  Returns true if the data type is removed from the list. Otherwise, returns false.
     **/
    virtual void dataTypeDeleted(DataTypeCustom* dataType) override;

    /**
     * \brief   Triggered when the data type is updated.
     * \param   dataType    The data type object to update.
     * \return  Returns true if the data type is updated. Otherwise, returns false.
     **/
    virtual void dataTypeUpdated(DataTypeCustom* dataType) override;

private:

    /**
     * \brief   Called to convert the data type saved in specified tree node.
     * \param   current     The tree node to convert the data type.
     * \param   newCategory The new category of the data type.
     **/
    void convertDataType(QTreeWidgetItem* current, DataTypeBase::eCategory newCategory);

    /**
     * \brief   Called to show or hide the details of enumeration object.
     * \param   current     The tree node to update the data type.
     * \param   show        If true, shows the details, otherwise hides.
     **/
    void showEnumDetails(bool show);

    /**
     * \brief   Called to show or hide the details of structure object.
     * \param   current     The tree node to update the data type.
     * \param   show        If true, shows the details, otherwise hides.
     **/
    void showImportDetails(bool show);

    /**
     * \brief   Called to show or hide the details of structure object.
     * \param   current     The tree node to update the data type.
     * \param   show        If true, shows the details, otherwise hides.
     **/
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

    /**
     * \brief   Triggered when in the tree table the structure data type item is selected.
     * \param   oldType     The previously selected data type object.
     * \param   dataType    The actual selected structure data type object.
     **/
    void selectedStruct(DataTypeCustom* oldType, DataTypeStructure* dataType);

    /**
     * \brief   Triggered when in the tree table the enumeration data type item is selected.
     * \param   oldType     The previously selected data type object.
     * \param   dataType    The actual selected enumeration data type object.
     **/
    void selectedEnum(DataTypeCustom* oldType, DataTypeEnum* dataType);

     /**
      * \brief   Triggered when in the tree table the imported data type item is selected.
      * \param   oldType     The previously selected data type object.
      * \param   dataType    The actual selected imported data type object.
      **/
    void selectedImport(DataTypeCustom* oldType, DataTypeImported* dataType);

    /**
     * \brief   Triggered when in the tree table the container data type item is selected.
     * \param   oldType     The previously selected data type object.
     * \param   dataType    The actual selected container data type object.
     **/
    void selectedContainer(DataTypeCustom* oldType, DataTypeContainer* dataType);

    /**
     * \brief   Triggered when in the tree table the structure field item is selected.
     * \param   oldType     The previously selected data type object.
     * \param   field       The actual selected field entry object.
     * \param   parent      The parent structure data type object.
     **/
    void selectedStructField(DataTypeCustom* oldType, const FieldEntry& field, DataTypeStructure* parent);

    /**
     * \brief   Triggered when in the tree table the enumeration field item is selected.
     * \param   oldType     The previously selected data type object.
     * \param   field       The actual selected enumeration entry object.
     * \param   parent      The parent enumeration data type object.
     **/
    void selectedEnumField(DataTypeCustom* oldType, const EnumEntry& field, DataTypeEnum* parent);

    /**
     * \brief   Creates the tree node for the structure data type.
     * \param   dataType    The structure data type object.
     * \return  Returns the created tree node.
     **/
    QTreeWidgetItem* createNodeStructure(DataTypeStructure* dataType) const;

    /**
     * \brief   Creates the tree node for the enumeration data type.
     * \param   dataType    The enumeration data type object.
     * \return  Returns the created tree node.
     **/
    QTreeWidgetItem* createNodeEnum(DataTypeEnum* dataType) const;

    /**
     * \brief   Creates the tree node for the imported data type.
     * \param   dataType    The imported data type object.
     * \return  Returns the created tree node.
     **/
    QTreeWidgetItem* createNodeImported(DataTypeImported* dataType) const;

    /**
     * \brief   Creates the tree node for the container data type.
     * \param   dataType    The container data type object.
     * \return  Returns the created tree node.
     **/
    QTreeWidgetItem* createNodeContainer(DataTypeContainer* dataType) const;

    /**
     * \brief   Updates the tree node for the structure data type.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The structure data type object.
     **/
    void updateNodeStructure(QTreeWidgetItem* node, DataTypeStructure* dataType) const;

    /**
     * \brief   Updates the tree node for the enumeration data type.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The enumeration data type object.
     **/
    void updateNodeEnum(QTreeWidgetItem* node, DataTypeEnum* dataType) const;

    /**
     * \brief   Updates the tree node for the imported data type.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The imported data type object.
     **/
    void updateNodeImported(QTreeWidgetItem* node, DataTypeImported* dataType) const;

    /**
     * \brief   Updates the tree node for the container data type.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The container data type object.
     **/
    void updateNodeContainer(QTreeWidgetItem* node, DataTypeContainer* dataType) const;

    /**
     * \brief   Updates the tree node for the structure field.
     * \param   child       The tree node to update the field.
     * \param   dataType    The structure data type object.
     * \param   field       The field entry object.
     **/
    void updateChildNodeStruct(QTreeWidgetItem* child, DataTypeStructure* dataType, FieldEntry& field) const;

    /**
     * \brief   Updates the tree node for the enumeration field.
     * \param   child       The tree node to update the field.
     * \param   dataType    The enumeration data type object.
     * \param   field       The enumeration entry object.
     **/
    void updateChildNodeEnum(QTreeWidgetItem* child, DataTypeEnum* dataType, EnumEntry& field) const;

    /**
     * \brief   Activates or deactivates the field details widget.
     * \param   activate    If true, activates and shows the field details widget.
     *                      Otherwise, deactivates and hides.
     **/
    void activateFields(bool activate);

    /**
     * \brief   Updates the container names on the tree table.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The container data type object.
     **/
    void updateContainerNames(QTreeWidgetItem* node, DataTypeContainer *dataType) const;

    /**
     * \brief   Updates the import names on the tree table.
     * \param   node        The tree node to update the data type.
     * \param   dataType    The imported data type object.
     **/
    void updateImportNames(QTreeWidgetItem* node, DataTypeImported* dataType) const;

    /**
     * \brief   Gets the field object of structure or enumeration of selected node of tree table.
     **/
    inline ElementBase* getSelectedField(void) const;

    /**
     * \brief   Disables or enables the radio buttons to select data type object.
     **/
    inline void enableTypeSelection(bool enable);

    /**
     * \brief   Deletes the tree node from the tree table.
     * \param   node    The tree node to delete.
     **/
    inline void deleteTreeNode(QTreeWidgetItem* node);
    
    /**
     * \brief   Returns list of basic container data type objects.
     **/
    static const QList<DataTypeBasicContainer *> & _getContainerTypes(void);

    /**
     * \brief   Returns list of custom data type objects.
     **/
    static const QList<DataTypeBase *>& _getIntegerTypes(void);

    /**
     * \brief   Returns list of predefined data type objects.
     **/
    static const QList<DataTypeBase *>& _getPredefinedTypes(void);


//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIDataTypeDetails*      mDetails;   //!< The data type details widget.
    SIDataTypeList*         mList;      //!< The data type list (tree table) widget.
    SIDataTypeFieldDetails* mFields;    //!< The data type field details widget.
    SIDataTypeWidget*       mWidget;    //!< The helper widget.
    Ui::SIDataType &        ui;         //!< The UI helper object.
    SIDataTypeModel&        mModel;     //!< The data type main model object.
    DataTypesModel*         mTypeModel; //!< The model to display data types in the type combo-box.
    DataTypesModel*         mValueModel;//!< The model to display data types in the value combo-box.
    DataTypesModel*         mKeysModel; //!< The model to display data types in the key combo-box.
    TableCell*              mTableCell; //!< The table cell object.
    QString                 mCurUrl;    //!< The current URL used when browse imported file location.
    QString                 mCurFile;   //!< The current file to import data types.
    int                     mCurView;   //!< The current view of the file open dialog.

    uint32_t                mCount;     //!< The counter used to generate data type names.
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPE_HPP

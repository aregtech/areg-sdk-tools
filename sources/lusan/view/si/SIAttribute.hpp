#ifndef LUSAN_APPLICATION_SI_SIATTRIBUTE_HPP
#define LUSAN_APPLICATION_SI_SIATTRIBUTE_HPP
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
 *  \file        lusan/view/si/SIAttribute.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/view/common/IEDataTypeConsumer.hpp"

#include <QAbstractListModel>
#include <QScrollArea>

/************************************************************************
 * Dependencies
 ************************************************************************/
namespace Ui {
class SIAttribute;
}

class DataTypesModel;
class SIAttributeDetails;
class SIAttributeList;
class SIAttributeModel;
class TableCell;

//////////////////////////////////////////////////////////////////////////
// SIAttributeNotifyModel class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class SIAttributeNotifyModel
 * \brief Model to manage notification types for attributes.
 */
class SIAttributeNotifyModel    : public QAbstractListModel
{
    Q_OBJECT
    
private:
    static constexpr const AttributeEntry::eNotification NotificationList[2]{ AttributeEntry::eNotification::NotifyOnChange, AttributeEntry::eNotification::NotifyAlways };
    
public:
    /**
     * \brief Constructor with initialization.
     * \param parent The parent object.
     */
    SIAttributeNotifyModel(QObject* parent = nullptr);
    
    /**
     * \brief Returns the number of rows in the model.
     * \param parent The parent index.
     * \return The number of rows.
     */
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    
    /**
     * \brief Returns the data for the given role and section in the model.
     * \param index The index of the item.
     * \param role The role for which data is requested.
     * \return The data for the given role and section.
     */
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

/**
 * \class SIAttributeWidget
 * \brief The widget to display the data attribute details.
 */
class SIAttributeWidget : public QWidget
{
    friend class SIAttribute;
    
    Q_OBJECT
    
public:
    /**
     * \brief Constructor with initialization.
     * \param parent The parent widget.
     */
    explicit SIAttributeWidget(QWidget* parent);
    
private:
    Ui::SIAttribute* ui; //!< The user interface of the data attribute.
};


//////////////////////////////////////////////////////////////////////////
// SIAttribute class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \class SIAttribute
 * \brief The widget to display the data attribute details.
 */
class SIAttribute   : public QScrollArea
                    , public IEDataTypeConsumer
{
    /**
     * \brief   The column indexes of the data attribute list.
     **/
    enum eColumn
    {
          ColName   = 0 //!< The column index of the data attribute name.
        , ColType   = 1 //!< The column index of the data attribute type.
        , ColNotify = 2 //!< The column index of the data attribute notification.
    };
    
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief Constructor with initialization.
     * \param model The model of the data attribute.
     * \param parent The parent widget.
     */
    explicit SIAttribute(SIAttributeModel & model, QWidget* parent = nullptr);
    
    /**
     * \brief Destructor.
     */
    virtual ~SIAttribute(void);
    
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
    
//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:
    
    /**
     * \brief   Triggered when the current cell is changed.
     * \param   currentRow      The current row index.
     * \param   currentColumn   The current column index.
     * \param   previousRow     The previous row index.
     * \param   previousColumn  The previous column index.
     **/
    void onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    
    /**
     * \brief   Triggered when the add button is clicked.
     **/
    void onAddClicked(void);
    
    /**
     * \brief   Triggered when the remove button is clicked.
     **/
    void onRemoveClicked(void);
    
    /**
     * \brief   Triggered when the insert button is clicked.
     **/
    void onInsertClicked(void);

    /**
     * \brief   Triggered when the move up button is clicked.
     **/
    void onMoveUpClicked(void);

    /**
     * \brief   Triggered when the move down button is clicked.
     **/
    void onMoveDownClicked(void);
    
    /**
     * \brief   Triggered when the move up button is clicked.
     * \param   newName     The new name of the data attribute object.
     **/
    void onNameChanged(const QString& newName);
    
    /**
     * \brief   Triggered when the type is changed.
     * \param   newType The new type of the data attribute.
     **/
    void onTypeChanged(const QString& newType);
    
    /**
     * \brief   Triggered when the notification value is changed.
     * \param   newValue The new value of the data attribute update notification.
     */
    void onNotificationChanged(const QString& newValue);
    
    /**
     * \brief   Triggered when the cell editor data is changed.
     * \param   index       The index of the cell.
     * \param   newValue    The new value of the cell.
     **/
    void onEditorDataChanged(const QModelIndex &index, const QString &newValue);
    
    /**
     * \brief   Triggered when the deprecated flag is changed.
     * \param   isChecked   The flag to indicate if the attribute is deprecated.
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
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    
    /**
     * \brief   Initializes the SIAttribute object.
     **/
    void updateData(void);
    
    /**
     * \brief   Updates the widgets.
     **/
    void updateWidgets(void);
    
    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);
    
    /**
     * \brief   Blocks the basic signals.
     * \param   doBlock     If true, blocks the signals, otherwise unblocks.
     **/
    void blockBasicSignals(bool doBlock);
    
    /**
     * \brief Triggered when the cell data is changed to update other controls.
     * \param row The row index of the cell.
     * \param col The column index of the cell.
     * \param newValue The new value of the cell.
     */
    void cellChanged(int row, int col, const QString& newValue);
    
    /**
     * \brief   Sets the texts in the table of the attribute entry.
     * \param   row     The row index of the attribute.
     * \param   insert  If true, inserts new the row. Otherwise, updates the row.
     *                  If `row` parameter is negative, always inserts new entry at the end of the list.
     **/
    inline void setTexts(int row, const AttributeEntry& entry, bool insert = false);

    /**
     * \brief   Updates the controls to display the control entry details in the details widget.
     * \param   entry       The attribute entry object. If nullptr, it disables controls and displays empty entry.
     * \param   updateAll   If true, updates all details. Otherwise, updates only the name, type, and value.
     **/
    inline void updateDetails(const AttributeEntry* entry, bool updateAll = false);
    
    /**
     * \brief Finds and returns valid pointer to the attribute entry in the specified row.
     * \param row The row index of the attribute entry.
     * \return Pointer to the attribute entry.
     */
    inline AttributeEntry* findAttribute(int row);
    
    /**
     * \brief Finds and returns valid pointer to the attribute entry in the specified row.
     * \param row The row index of the attribute entry.
     * \return Pointer to the attribute entry.
     */
    inline const AttributeEntry* findAttribute(int row) const;

    /**
     * \brief   Swaps the attributes by given row indexes.
     *          The swapping will not change the order of IDs, but will swap the data.
     * \param   firstRow    The row index of the first attribute to swap.
     * \param   secondRow   The row index of the second attribute to swap.
     **/
    inline void swapAttributes(int firstRow, int secondRow);

    /**
     * \brief   Updates the tool buttons.
     * \param   row         The row index of the attribute.
     * \param   rowCount    The total number of rows in the table.
     **/
    inline void updateToolBottons(int row, int rowCount);

    /**
     * \brief   Generates the name of the attribute.
     * \return  The generated name of the attribute.
     **/
    inline QString genName(void);
    
//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIAttributeModel&       mModel;         //!< The model of the data attribute.
    SIAttributeDetails*     mDetails;       //!< The details widget of the data attribute.
    SIAttributeList*        mList;          //!< The list widget of the data attribute.
    SIAttributeWidget*      mWidget;        //!< The widget of the data attribute.
    Ui::SIAttribute&        ui;             //!< The user interface of the data attribute.
    DataTypesModel*         mTypeModel;     //!< The model of the data types.
    SIAttributeNotifyModel* mNotifyModel;   //!< The model of the notification types.
    TableCell*              mTableCell;     //!< The table cell object.
    uint32_t                mCount;         //!< The counter to generate names.
};

#endif // LUSAN_APPLICATION_SI_SIAttribute_HPP

#ifndef LUSAN_APPLICATION_SI_SIMETHOD_HPP
#define LUSAN_APPLICATION_SI_SIMETHOD_HPP
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
 *  \file        lusan/view/si/SIMethod.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QScrollArea>
#include "lusan/view/common/IEDataTypeConsumer.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class ElementBase;
class SIMethodBase;
class SIMethodDetails;
class SIMethodList;
class SIMethodParamDetails;
class SIMethodModel;
class QTreeWidgetItem;
class SIMethodBroadcast;
class SIMethodRequest;
class SIMethodResponse;
class MethodParameter;
class DataTypesModel;
class ReplyMethodModel;

namespace Ui {
    class SIMethod;
}

class SIMethodWidget : public QWidget
{
    friend class SIMethod;

    Q_OBJECT

public:
    explicit SIMethodWidget(QWidget* parent);

private:
    Ui::SIMethod* ui;
};

//////////////////////////////////////////////////////////////////////////
// SIMethod class declaration
//////////////////////////////////////////////////////////////////////////
class SIMethod  : public QScrollArea
                , public IEDataTypeConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIMethod(SIMethodModel & model, QWidget* parent = nullptr);

    virtual ~SIMethod(void);
    
//////////////////////////////////////////////////////////////////////////
// override
//////////////////////////////////////////////////////////////////////////
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
protected:

    /**
     * \brief Triggered when the method name is changed.
     * \param newName The new name of the method.
     **/
    void onNameChanged(const QString & newName);

    /**
     * \brief Triggered when the method type is changed to Request type.
     * \param isSelected    The selection flag. True if selected. Otherwise is false.
     **/
    void onRequestSelected(bool isSelected);

    /**
     * \brief Triggered when the method type is changed to Response type.
     * \param isSelected    The selection flag. True if selected. Otherwise is false.
     **/
    void onResponseSelected(bool isSelected);

    /**
     * \brief Triggered when the method type is changed to Broadcast type.
     * \param isSelected    The selection flag. True if selected. Otherwise is false.
     **/
    void onBroadcastSelected(bool isSelected);

    /**
     * \brief   Triggered when the deprecated flag is changed.
     * \param   isChecked   The flag to indicate if the constant is deprecated.
     **/
    void onDeprecateChecked(bool isChecked);

    /**
     * \brief   Triggered when the deprecation hint is changed.
     * \param   newText The new deprecation hint.
     **/
    void onDeprecateHintChanged(const QString & newText);

    /**
     * \brief   Triggered when the description is changed.
     **/
    void onDescriptionChanged(void);

    /**
     * \brief   Triggered when the request connected new response is selected.
     * \param   newText The new response.
     **/
    void onConnectedResponseChanged(const QString& newText);

    /**
     * \brief   Triggered when the add button is clicked to add new method.
     **/
    void onAddClicked(void);
    
    /**
     * \brief   Triggered when the insert button is clicked to insert new method.
     **/
    void onInsertClicked(void);
    
    /**
     * \brief   Triggered when the remove button is clicked to remove selected method.
     **/
    void onRemoveClicked(void);

    /**
     * \brief   Triggered when the add parameter button is clicked to add new parameter.
     **/
    void onParamAddClicked(void);

    /**
     * \brief   Triggered when the remove parameter button is clicked to remove selected parameter.
     **/
    void onParamRemoveClicked(void);

    /**
     * \brief   Triggered when the move up button is clicked to insert new method parameter.
     **/
    void onParamInsertClicked(void);

    /**
     * \brief   Triggered when the move up button is clicked to move a method up in the list.
     **/
    void onMoveUpClicked(void);

    /**
     * \brief   Triggered when the move down button is clicked to move a method down in the list.
     **/
    void onMoveDownClicked(void);

    /**
     * \brief   Triggered when the current cell is changed in the tree table.
     * \param   current     The current item.
     * \param   previous    The previous item.
     **/
    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    /**
     * \brief   Triggered when the parameter name is changed.
     * \param   newText The new name of the parameter.
     **/
    void onParamNameChanged(const QString& newText);

    /**
     * \brief   Triggered when the parameter type is changed.
     * \param   newText The new type of the parameter.
     **/
    void onParamTypeChanged(const QString & newText);

    /**
     * \brief   Triggered when the default value of the parameter is checked.
     * \param   isChecked   If true, the default value is checked. Otherwise, it is false.
     **/
    void onParamDefaultChecked(bool isChecked);

    /**
     * \brief   Triggered when the default value of the parameter is changed.
     * \param   newText The new default value of the parameter.
     **/
    void onParamDefaultChanged(const QString& newText);

    /**
     * \brief   Triggered when the description of the parameter is changed.
     **/
    void onParamDescriptionChanged(void);

    /**
     * \brief   Triggered when the deprecated flag of the parameter is changed.
     * \param   isChecked   The flag to indicate if the parameter is deprecated.
     **/
    void onParamDeprecateChecked(bool isChecked);

    /**
     * \brief   Triggered when the deprecation hint of the parameter is changed.
     * \param   newText The new deprecation hint.
     **/
    void onParamDeprecateHintChanged(const QString& newText);

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
private:
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
     * \brief   Updates the method tree-table node in the tree table.
     * \param   item    The item to update.
     * \param   method  The method object to update.
     * \return  Returns the updated tree-table node.
     **/
    QTreeWidgetItem* updateMethodNode(QTreeWidgetItem* item, SIMethodBase* method);

    /**
     * \brief   Shows the details of the method.
     * \param   method  The method object to show details.
     **/
    void showMethodDetails(SIMethodBase* method);

    /**
     * \brief   Shows the details of the parameter.
     * \param   method  The method object to show parameter details.
     * \param   param   The parameter object to show details.
     **/
    void showParamDetails(SIMethodBase* method, const MethodParameter& param);

    /**
     * \brief   Gets the current method and parameter.
     * \param   item    The current item in the tree table.
     * \param   method  The method object to get.
     * \return  Returns true if the method is found, false otherwise.
     **/
    bool getCurrentMethod(QTreeWidgetItem*& item, SIMethodBase*& method);

    /**
     * \brief   Gets the current parameter.
     * \param   item    The current item in the tree table.
     * \param   method  The method object to get.
     * \param   param   The parameter object to get.
     * \return  Returns true if the parameter is found, false otherwise.
     **/
    bool getCurrentParam(QTreeWidgetItem*& item, SIMethodBase*& method, MethodParameter*& param);

    /**
     * \brief   Sets the text of the node.
     * \param   node    The node to set text.
     * \param   method  The method object to get display texts.
     **/
    void setNodeText(QTreeWidgetItem* node, const ElementBase* method);

    /**
     * \brief   Triggered when a response is deleted, this updates the request entries connected with response object.
     * \param   response    The response object to be deleted.
     **/
    void responseDeleted(SIMethodResponse* response);

    /**
     * \brief   Moves the method up in the tree table.
     * \param   node    The node to move up.
     **/
    inline void moveMethodUp(QTreeWidgetItem* node);

    /**
     * \brief   Moves the method parameter up in the tree table.
     * \param   node    The node to move up.
     **/
    inline void moveMethodParamUp(QTreeWidgetItem* node);

    /**
     * \brief   Moves the method down in the tree table.
     * \param   node    The node to move down.
     **/
    inline void moveMethodDown(QTreeWidgetItem* node);

    /**
     * \brief   Moves the method parameter down in the tree table.
     * \param   node    The node to move down.
     **/
    inline void moveMethodParamDown(QTreeWidgetItem* node);

    /**
     * \brief   Swaps 2 methods in the tree table without changing the order of IDs.
     * \param   node    The node to swap.
     * \param   row     The row index of the selected data type element.
     * \param   moveRow The row index to move.
     **/
    inline void swapMethods(QTreeWidgetItem* node, int row, int moveRow);

    /**
     * \brief   Swaps 2 method parameters in the tree table without changing the order of IDs.
     * \param   node    The node to swap.
     * \param   parent  The parent node of the selected data type element.
     * \param   row     The row index of the selected data type element.
     * \param   moveRow The row index to move.
     **/
    inline void swapMethodParams(QTreeWidgetItem* node, QTreeWidgetItem* parent, int row, int moveRow);

    /**
     * \brief   Updates the tool buttons in the tree table.
     * \param   row         The row index of the selected data type element.
     * \param   rowCount    The total number of rows in the tree table.
     **/
    inline void updateToolButtonsForMethod(int row, int rowCount);
    
    inline void updateToolButtonsForParams(SIMethodBase * method, int pos);
    
    /**
     * \brief   Generates and returns new method name.
     **/
    inline QString genName(void);

    /**
     * \brief   Generates and returns new parameter name of the method.
     * \param   method  The method object to get name.
     **/
    inline QString genName(SIMethodBase * method);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIMethodModel &         mModel;         //!< The method model object.
    SIMethodDetails*        mDetails;       //!< The method details widget object.
    SIMethodList*           mList;          //!< The method list (tree table) widget object.
    SIMethodParamDetails* mParams;          //!< The method parameter details widget object.
    SIMethodWidget*         mWidget;        //!< The helper widget object.
    DataTypesModel*         mParamTypes;    //!< The parameter data type model object.
    ReplyMethodModel*       mReplyModel;    //!< The reply (response) method model object.
    Ui::SIMethod&           ui;             //!< The UI helper object.

    uint32_t                mCount;         //!< The counter to generate new method names.
};

#endif // LUSAN_APPLICATION_SI_SIMETHOD_HPP

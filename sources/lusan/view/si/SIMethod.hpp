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
#include <QScrollArea>
#include "lusan/view/common/IEDataTypeConsumer.hpp"

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

class SIMethod  : public QScrollArea
                , public IEDataTypeConsumer
{
    Q_OBJECT

public:
    explicit SIMethod(SIMethodModel & model, QWidget* parent = nullptr);

    virtual ~SIMethod(void);
    
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

protected:

    void onNameChanged(const QString & newName);

    void onRequestSelected(bool isSelected);

    void onResponseSelected(bool isSelected);

    void onBroadcastSelected(bool isSelected);
    
    void onDeprecateChecked(bool isChecked);
    
    void onDeprecateChanged(const QString & newText);
    
    void onDescriptionChanged(void);

    void onConnectedResponseChanged(const QString& newText);

    void onAddClicked(void);

    void onRemoveClicked(void);

    void onParamAddClicked(void);

    void onParamRemoveClicked(void);

    void onParamInsertClicked(void);

    void onMoveUpClicked(void);

    void onMoveDownClicked(void);

    void onCurCellChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    
    void onParamNameChanged(const QString& newText);
    
    void onParamTypeChanged(const QString & newText);
    
    void onParamDefaultChecked(bool isChecked);
    
    void onParamDefaultChanged(const QString& newText);
    
    void onParamDescriptionChanged(void);
    
    void onParamDeprecateChecked(bool isChecked);
    
    void onParamDeprecateHintChanged(const QString& newText);

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

    QTreeWidgetItem* updateMethodNode(QTreeWidgetItem* item, SIMethodBase* method);

    void showMethodDetails(SIMethodBase* method);

    void showParamDetails(SIMethodBase* method, const MethodParameter& param);

    bool getCurrentMethod(QTreeWidgetItem*& item, SIMethodBase*& method);

    bool getCurrentParam(QTreeWidgetItem*& item, SIMethodBase*& method, MethodParameter*& param);

    void setNodeText(QTreeWidgetItem* node, const ElementBase* method);

    void responseDeleted(SIMethodResponse* response);

private:
    SIMethodModel &         mModel;
    SIMethodDetails*        mDetails;
    SIMethodList*           mList;
    SIMethodParamDetails*   mParams;
    SIMethodWidget*         mWidget;
    DataTypesModel*         mParamTypes;
    ReplyMethodModel*       mReplyModel;
    Ui::SIMethod&           ui;

    uint32_t               mCount;
};

#endif // LUSAN_APPLICATION_SI_SIMETHOD_HPP

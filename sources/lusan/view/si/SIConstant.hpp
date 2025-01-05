#ifndef LUSAN_APPLICATION_SI_SICONSTANT_HPP
#define LUSAN_APPLICATION_SI_SICONSTANT_HPP
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
 *  \file        lusan/view/si/SIConstant.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>

namespace Ui {
    class SIConstant;
}

class SIConstantDetails;
class SIConstantList;
class SIConstantModel;
class DataTypesModel;

class SIConstantWidget : public QWidget
{
    friend class SIConstant;

    Q_OBJECT

public:
    explicit SIConstantWidget(QWidget* parent);

private:
    Ui::SIConstant* ui;
};

class SIConstant : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIConstant(SIConstantModel& model, QWidget* parent = nullptr);

    virtual ~SIConstant(void);

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
     * \brief   Triggered when the update button is clicked.
     **/
    void onUpdateClicked(void);

    void onNameChanged(const QString& newName);

    void onTypeChanged(const QString& newType);
    
    void onTypesOpened(void);

    void onValueChanged(const QString& newValue);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Initializes the SIInclude object.
     **/
    void updateData(void);
    
    void updateDataTypes(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

private:
    SIConstantModel&    mModel;
    SIConstantDetails*  mDetails;
    SIConstantList*     mList;
    SIConstantWidget*   mWidget;
    Ui::SIConstant&     ui;
    DataTypesModel*     mTypeModel;
    uint32_t            mCount;
};

#endif // LUSAN_APPLICATION_SI_SICONSTANT_HPP

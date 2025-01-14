#ifndef LUSAN_APPLICATION_SI_SERVICEINTERFACE_HPP
#define LUSAN_APPLICATION_SI_SERVICEINTERFACE_HPP

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
 *  \file        lusan/view/si/ServiceInterface.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/common/MdiChild.hpp"
#include "lusan/model/si/ServiceInterfaceModel.hpp"
#include "lusan/view/si/SIConstant.hpp"
#include "lusan/view/si/SIDataType.hpp"
#include "lusan/view/si/SIDataTopic.hpp"
#include "lusan/view/si/SIInclude.hpp"
#include "lusan/view/si/SIMethod.hpp"
#include "lusan/view/si/SIOverview.hpp"


#include <QTabWidget>

class ServiceInterface : public MdiChild
{
    Q_OBJECT
    
private:
    
    static  uint32_t                   _count;
    static constexpr const char* const _defName {"NewServiceInterface"};
    
public:
    
public:
    ServiceInterface(QWidget *parent = nullptr);
    virtual ~ServiceInterface(void);

public slots:

    void slotDataTypeCreated(DataTypeCustom* dataType);

    void slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    void slotlDataTypeRemoved(DataTypeCustom* dataType);

    void slotlDataTypeUpdated(DataTypeCustom* dataType);

private:
    ServiceInterfaceModel   mModel;
    QTabWidget  mTabWidget;
    SIOverview  mOverview;
    SIDataType  mDataType;
    SIDataTopic mDataTopic;
    SIMethod    mMethod;
    SIConstant  mConstant;
    SIInclude   mInclude;
};

#endif // LUSAN_APPLICATION_SI_SERVICEINTERFACEVIEW_HPP

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
    
//////////////////////////////////////////////////////////////////////////
// Public static methods
//////////////////////////////////////////////////////////////////////////
public:
    static const QString& fileExtension(void);

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    ServiceInterface(const QString & filePath = QString(), QWidget *parent = nullptr);

    virtual ~ServiceInterface(void);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:

    void slotDataTypeCreated(DataTypeCustom* dataType);

    void slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    void slotlDataTypeRemoved(DataTypeCustom* dataType);

    void slotlDataTypeUpdated(DataTypeCustom* dataType);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Returns the file open operation success flag.
     **/
    virtual bool openSucceeded(void) const override;
    
protected:
    
    /**
     * \brief   Returns the default file name of new created document.
     **/
    virtual QString newDocumentName(void) override;

    /**
     * \brief   Returns the default name of new created document.
     **/
    virtual const QString& newDocument(void) const override;

    /**
     * \brief   Returns the default extension of new created document.
     **/
    virtual const QString& newDocumentExt(void) const override;

    /**
     * \brief   Reads the document from the file.
     * \param   filePath    The path of the file to read.
     * \return  True if the document was successfully read, false otherwise.
     **/
    virtual bool writeToFile(const QString& filePath) override;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    ServiceInterfaceModel   mModel; //!< The model of the service interface
    QTabWidget  mTabWidget; //!< The tab widget to display the service interface elements
    SIOverview  mOverview;  //!< The overview widget
    SIDataType  mDataType;  //!< The data type widget
    SIDataTopic mDataTopic; //!< The data topic widget
    SIMethod    mMethod;    //!< The method widget
    SIConstant  mConstant;  //!< The constant widget
    SIInclude   mInclude;   //!< The include widget
};

#endif // LUSAN_APPLICATION_SI_SERVICEINTERFACEVIEW_HPP

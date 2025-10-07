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
#include "lusan/view/si/SIAttribute.hpp"
#include "lusan/view/si/SIInclude.hpp"
#include "lusan/view/si/SIMethod.hpp"
#include "lusan/view/si/SIOverview.hpp"

#include <QTabWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;

/**
 * \brief   The ServiceInterface class represents the MDI window for the service interface in the Lusan application.
 **/
class ServiceInterface  : public MdiChild
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Internal constants and types
//////////////////////////////////////////////////////////////////////////
private:
    
    static  uint32_t                   _count;
    static constexpr const char* const _defName {"NewServiceInterface"};
    
//////////////////////////////////////////////////////////////////////////
// Public static methods
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns the file extension of the service interface document.
     **/
    static const QString& fileExtension(void);
    
    /**
     * \brief   The list of pages in the service interface
     **/
    enum eSIPages
    {
          PageOverview      = 0 //!< The overview page
        , PageDataTypes         //!< The data types page
        , PageAttributes        //!< The data attributes page
        , PageMethods           //!< The methods page
        , PageConstants         //!< The constants page
        , PageIncludes          //!< The includes page
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor for ServiceInterface.
     * \param   wndMain    Pointer to the main MDI window.
     * \param   filePath   The path of the file to open (optional).
     * \param   parent     Pointer to the parent widget (optional).
     **/
    ServiceInterface(MdiMainWindow *wndMain, const QString & filePath = QString(), QWidget *parent = nullptr);

    virtual ~ServiceInterface(void);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:

    /**
     * \brief   Triggered when a new data type is created.
     * \param   dataType    The pointer to the created data type.
     **/
    void slotDataTypeCreated(DataTypeCustom* dataType);

    /**
     * \brief   Triggered when a data type is converted.
     * \param   oldType     The pointer to the old data type.
     * \param   newType     The pointer to the new data type.
     **/
    void slotDataTypeConverted(DataTypeCustom* oldType, DataTypeCustom* newType);

    /**
     * \brief   Triggered when a data type is removed.
     * \param   dataType    The pointer to the removed data type.
     **/
    void slotDataTypeDeleted(DataTypeCustom* dataType);

    /**
     * \brief   Triggered when a data type is updated.
     * \param   dataType    The pointer to the updated data type.
     **/
    void slotDataTypeUpdated(DataTypeCustom* dataType);

    /**
     * \brief   Triggered when a page link is clicked.
     * \param   page    The index of the clicked page.
     **/
    void slotPageLinkClicked(int page);

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
     * \brief   Returns the default file suffix.
     **/
    virtual const QString& fileSuffix(void) const override;

    /**
     * \brief   Returns the default file filter.
     **/
    virtual const QString& fileFilter(void) const override;

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
    SIAttribute mAttribute; //!< The data attribute widget
    SIMethod    mMethod;    //!< The method widget
    SIConstant  mConstant;  //!< The constant widget
    SIInclude   mInclude;   //!< The include widget
};

#endif // LUSAN_APPLICATION_SI_SERVICEINTERFACEVIEW_HPP

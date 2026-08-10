#ifndef LUSAN_APPLICATION_SI_SERVICEINTERFACE_HPP
#define LUSAN_APPLICATION_SI_SERVICEINTERFACE_HPP

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
 *  \file        lusan/view/si/ServiceInterface.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface main window.
 *
 ************************************************************************/

#include "lusan/view/common/MdiChild.hpp"

#include "lusan/model/si/ServiceInterfaceModel.hpp"

#include <QList>
#include <QTabWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class ConstantPage;
class MdiMainWindow;
class AttributePage;
class DataTypePage;
class IncludePage;
class MethodPage;
class SIOverview;

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
    static const QString& fileExtension();
    
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

    virtual ~ServiceInterface();

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:

    /**
     * \brief   Tells every open page that the document's data types changed, so the pages that
     *          offer them or declare their elements with them re-read the current set.
     **/
    void slotDataTypesChanged();

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
    bool openSucceeded() const override;

    /**
     * \brief   Steps the document's history back one entry.
     **/
    void undo() override;

    /**
     * \brief   Steps the document's history forward one entry.
     **/
    void redo() override;

    bool canUndo() const override;
    bool canRedo() const override;

    /**
     * \brief   The document facade, which the output window's Validation tab binds to.
     **/
    IEDocumentModel* documentModel() override;

    /**
     * \brief   Reveals a validation finding of this document: switches to the page that owns the
     *          element, building it on demand, and puts the accent on the field at fault.
     **/
    void navigateToIssue(uint32_t elementId, eDocElementKind kind, int rule) override;

    //!< The model of the service interface.
    inline ServiceInterfaceModel& getModel();

protected:
    
    /**
     * \brief   Returns the default file name of new created document.
     **/
    QString newDocumentName() override;

    /**
     * \brief   Returns the default name of new created document.
     **/
    const QString& newDocument() const override;

    /**
     * \brief   Returns the default extension of new created document.
     **/
    const QString& newDocumentExt() const override;

    /**
     * \brief   Returns the default file suffix.
     **/
    const QString& fileSuffix() const override;

    /**
     * \brief   Returns the default file filter.
     **/
    const QString& fileFilter() const override;

    /**
     * \brief   Reads the document from the file.
     * \param   filePath    The path of the file to read.
     * \return  True if the document was successfully read, false otherwise.
     **/
    bool writeToFile(const QString& filePath) override;

    /**
     * \brief   Collects the text every open page is still holding in a field that has the focus,
     *          so a save from the keyboard writes what the author already typed.
     **/
    void commitPendingEdits(void) override;

    /**
     * \brief   Returns the page tab host, enabling the shared Ctrl+PageDown / Ctrl+PageUp cycling.
     **/
    QTabWidget* pageTabWidget() override;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    //!< Returns true if the index is a valid SI page index.
    static bool isValidTabIndex(int index);
    //!< Returns true if the page at given index is already created.
    bool isTabInitialized(int index) const;
    //!< Returns the tab title of the page at given index.
    QString tabTitle(int index) const;
    //!< Initializes next queued page and re-schedules itself while queue is not empty.
    void processQueuedTabInitialization();
    //!< Creates the page at given index if it does not exist yet.
    void ensureTabInitialized(int index);
    //!< Places the created page into the tab holder widget at given index.
    void attachPage(int index, QWidget* page);

private:
    ServiceInterfaceModel   mModel; //!< The model of the service interface
    QTabWidget  mTabWidget; //!< The tab widget to display the service interface elements
    SIOverview*  mOverview;  //!< The overview widget
    DataTypePage*  mDataType;  //!< The data type widget
    AttributePage* mAttribute; //!< The data attribute widget
    MethodPage*  mMethod;    //!< The method widget
    ConstantPage*  mConstant;  //!< The constant widget
    IncludePage* mInclude;   //!< The include widget
    QList<int>   mPendingInitTabs; //!< Background initialization queue
};

inline ServiceInterfaceModel& ServiceInterface::getModel()
{
    return mModel;
}

#endif // LUSAN_APPLICATION_SI_SERVICEINTERFACEVIEW_HPP

#ifndef LUSAN_VIEW_DT_DATATYPEDOCUMENT_HPP
#define LUSAN_VIEW_DT_DATATYPEDOCUMENT_HPP
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
 *  \file        lusan/view/dt/DataTypeDocument.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Data Type document window.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/MdiChild.hpp"

#include "lusan/model/dt/DataTypeDocumentModel.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"

#include <QTabWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class DataTypePage;
class IncludePage;
class MdiMainWindow;
class OverviewPage;

/**
 * \brief   The MDI window of a `.dtml` document: a set of data types meant to be shared between
 *          service interfaces and state machines, with the headers those types need.
 *
 *          It carries three pages and no more. Each of the three is the shared page controller
 *          the other two editors already use, so this window is the pages plus the document
 *          plumbing around them.
 **/
class DataTypeDocument : public MdiChild
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Public static methods and types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The file extension of a data type document.
     **/
    static const QString& fileExtension(void);

    /**
     * \brief   The pages of the document, in tab order.
     **/
    enum eDTPages
    {
          PageOverview      = 0 //!< What the document says about itself.
        , PageDataTypes         //!< The types it declares.
        , PageIncludes          //!< The C++ headers those types need.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Builds the window.
     * \param   wndMain     The main MDI window.
     * \param   filePath    The document to open, empty for a new one.
     * \param   parent      The parent widget.
     **/
    DataTypeDocument(MdiMainWindow* wndMain, const QString& filePath = QString(), QWidget* parent = nullptr);

    virtual ~DataTypeDocument(void) = default;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
public slots:
    /**
     * \brief   Switches to the page a quick link on the Overview names.
     **/
    void slotPageLinkClicked(int page);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    bool openSucceeded(void) const override;

    void undo(void) override;
    void redo(void) override;
    bool canUndo(void) const override;
    bool canRedo(void) const override;

    /**
     * \brief   The document facade the output window's Validation tab binds to.
     **/
    IEDocumentModel* documentModel(void) override;

    /**
     * \brief   Reveals a validation finding: switches to the page that owns the element,
     *          building it on demand, and puts the accent on the field at fault.
     **/
    void navigateToIssue(uint32_t elementId, eDocElementKind kind, int rule) override;

    //!< The model of the document.
    inline DataTypeDocumentModel& getModel(void);

protected:
    QString newDocumentName(void) override;
    const QString& newDocument(void) const override;
    const QString& newDocumentExt(void) const override;
    const QString& fileSuffix(void) const override;
    const QString& fileFilter(void) const override;

    bool writeToFile(const QString& filePath) override;

    /**
     * \brief   Collects the text a page is still holding in a focused field, so a save from the
     *          keyboard writes what the author already typed.
     **/
    void commitPendingEdits(void) override;

    /**
     * \brief   The page tab host, enabling the shared Ctrl+PageDown / Ctrl+PageUp cycling.
     **/
    QTabWidget* pageTabWidget(void) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    //!< True when the index names a page of this document.
    static bool isValidTabIndex(int index);
    //!< True when the page at the index is built already.
    bool isTabInitialized(int index) const;
    //!< The tab title of the page at the index.
    QString tabTitle(int index) const;
    //!< Builds the next queued page and re-schedules itself while the queue is not empty.
    void processQueuedTabInitialization(void);
    //!< Builds the page at the index if it is not there yet.
    void ensureTabInitialized(int index);
    //!< Puts a built page into its tab holder.
    void attachPage(int index, QWidget* page);
    //!< Marks the title while the history stands away from the saved point, or while a field
    //!< holds text the document has not received yet.
    void refreshModified(void);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    DataTypeDocumentModel   mModel;             //!< The document model.
    QTabWidget              mTabWidget;         //!< The page tabs.
    OverviewPage*           mOverview;          //!< The Overview page.
    DataTypePage*           mDataType;          //!< The Data Types page.
    IncludePage*            mInclude;           //!< The Includes page.
    QList<int>              mPendingInitTabs;   //!< Background initialization queue.
    PendingEditWatcher      mPendingEdits;      //!< Text typed into a field but not handed over yet.
};

//////////////////////////////////////////////////////////////////////////
// DataTypeDocument inline methods
//////////////////////////////////////////////////////////////////////////

inline DataTypeDocumentModel& DataTypeDocument::getModel(void)
{
    return mModel;
}

#endif  // LUSAN_VIEW_DT_DATATYPEDOCUMENT_HPP

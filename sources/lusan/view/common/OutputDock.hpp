#ifndef LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP
#define LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP
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
 *  \file        lusan/view/common/OutputDock.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Output docking window.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>
#include "areg/base/areg_global.h"

#include "lusan/view/log/ScopeOutputViewer.hpp"

#include <QPointer>
#include <QTabWidget>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;
class QVBoxLayout;
class DocValidationPanel;
class MdiChild;

/**
 * \brief   The OutputDock class is the output-window content (a tab widget of analysis views).
 *          It is a plain content widget hosted inside a Qt-Advanced-Docking-System dock widget
 *          (issue #516); the ADS dock provides the title bar, floating, and drag/tab behavior.
 **/
class OutputDock : public QWidget
{
    Q_OBJECT

////////////////////////////////////////////////////////////////////////////
// Internal types and constants
////////////////////////////////////////////////////////////////////////////
public:
    /**
     * /brief   Defines the possible status values for a window.
     **/
    enum eOutputDock
    {
          OutputUnknown     //!< Unknown output window
        , OutputLogging     //!< Status window for log analyzes
        , OutputValidation  //!< Validation findings of the open documents
    };

    //!< The tab name for the logging output window
    static const QString    TabNameLogging;

    //!< The tab name for the validation findings window
    static const QString    TabNameValidation;

    //!< Returns the tab name of the specified output window
    static const QString& getTabName(OutputDock::eOutputDock wndStatus);

    //!< Returns the output window type by specified tab name.
    static OutputDock::eOutputDock getOutputDock(const QString& tabName);

//////////////////////////////////////////////////////////////////////////
// Constructor and destructor
//////////////////////////////////////////////////////////////////////////
public:

    OutputDock(MdiMainWindow* parent);
    
    virtual ~OutputDock();

//////////////////////////////////////////////////////////////////////////
// Actions and attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the tab widget of the navigation.
     **/
    inline QTabWidget& getTabWidget();

    /**
     * \brief   Returns the scope output viewer.
     **/
    inline ScopeOutputViewer& getScopeLogsView();

    /**
     * \brief   Points the Validation tab at the open documents that carry validation, or at a
     *          placeholder when there are none. The list is rebuilt whenever the set of open
     *          documents changes.
     **/
    void setDocuments(const QList<MdiChild*>& docs);

    /**
     * \brief   Brings the Validation tab forward and, when \p step is non-zero, moves to the
     *          next (+1) or previous (-1) finding.
     **/
    void showValidation(int step = 0);

    /**
     * \brief   The findings panel of the currently bound document, or nullptr when none.
     **/
    inline DocValidationPanel* getValidationView() const;

    /**
     * \brief   A dock content widget must be free to grow and shrink along the docking axis,
     *          otherwise the splitter that holds it has no room to move and shows no handle.
     **/
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    //!< Initializes the size of the output dock.
    void initSize();

    //!< Replaces the Validation tab body with the disabled "no document" note.
    void showValidationPlaceholder();

    //!< Builds the findings panel on first use and returns it.
    DocValidationPanel* ensureValidationPanel();

    //!< Retitles the tab `Validation (N)`, or plain `Validation` when nothing is pending.
    void updateValidationTitle(int pending);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    MdiMainWindow*          mMainWindow;    //!< Main window
    QTabWidget              mTabs;          //!< The tab widget of the output windows.
    ScopeOutputViewer       mScopeOutput;   //<!< The scope output viewer for displaying logs from scopes.
    QWidget*                mValidationTab; //!< The Validation tab body, host of the findings panel.
    QVBoxLayout*            mValidationBody;//!< The single-child layout of the Validation tab.
    QWidget*                mValidationView;//!< The hosted findings panel, or the placeholder note.
    DocValidationPanel*     mValidation;    //!< mValidationView when it is a real findings panel.
    QList<QPointer<MdiChild> > mBoundDocs;  //!< The documents the Validation tab currently shows.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    AREG_NOCOPY_NOMOVE(OutputDock);
};

//////////////////////////////////////////////////////////////////////////
// OutputDock class inline methods
//////////////////////////////////////////////////////////////////////////

inline QTabWidget& OutputDock::getTabWidget()
{
    return mTabs;
}

inline ScopeOutputViewer& OutputDock::getScopeLogsView()
{
    return mScopeOutput;
}

inline DocValidationPanel* OutputDock::getValidationView() const
{
    return mValidation;
}

#endif  // LUSAN_VIEW_COMMON_OUTPUTDOCK_HPP

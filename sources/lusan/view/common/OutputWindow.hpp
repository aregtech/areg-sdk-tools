#ifndef LUSAN_VIEW_COMMON_OUTPUTWINDOW_HPP
#define LUSAN_VIEW_COMMON_OUTPUTWINDOW_HPP
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
 *  \file        lusan/view/common/OutputWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan project, the output windows elements.
 *
 ************************************************************************/

#include <QWidget>

class MdiMainWindow;

/**
 * \brief   Base class for output windows in the Lusan application.
 **/
class OutputWindow : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor for OutputWindow.
     * \param   outWindow   The type of the output window.
     * \param   wndMain     Pointer to the main MDI window.
     * \param   parent      Pointer to the parent widget.
     **/
    OutputWindow(int outWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);

    virtual ~OutputWindow(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   This method is called when the options dialog is opened.
     **/
    virtual void optionOpenning(void);

    /**
     * \brief   This method is called when the apply button in options dialog is pressed.
     *          It can be used to apply changes made in the options dialog.
     **/
    virtual void optionApplied(void);

    /**
     * \brief   This method is called when the options dialog is closed.
     * \param   OKpressed   True if OK button was pressed, false if Cancel button was pressed.
     **/
    virtual void optionClosed(bool OKpressed);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the type of the output window.
     **/
    inline int getOutputWindowType(void) const;

    /**
     * \brief   Returns true if output window is for scopes logs output.
     **/
    bool isScopesOutputWindow(void) const;

//////////////////////////////////////////////////////////////////////////
// OutputWindow class inline methods
//////////////////////////////////////////////////////////////////////////
protected:

    const int       mOutWindowType; //!< The type of the output window
    MdiMainWindow*  mMainWindow;    //!< Pointer to the main MDI window
};

//////////////////////////////////////////////////////////////////////////
// OutputWindow class inline methods
//////////////////////////////////////////////////////////////////////////

inline int OutputWindow::getOutputWindowType(void) const
{
    return mOutWindowType;
}

#endif  // LUSAN_VIEW_COMMON_OUTPUTWINDOW_HPP

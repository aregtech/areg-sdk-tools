#ifndef LUSAN_VIEW_COMMON_IEMDIWINDOW_HPP
#define LUSAN_VIEW_COMMON_IEMDIWINDOW_HPP
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
 *  \file        lusan/view/common/IEMdiWindow.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       MDI Window type.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

class IEMdiWindow
{
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    //! \brief   MDI Window type.
    enum eMdiWindow
    {
          MdiUnknown            = 0 //!< Unknown MDI Window type
        , MdiServiceInterface       //!< Service Interface MDI Window type
        , MdiLogViewer              //!< Log Viewer MDI Window type
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    IEMdiWindow(eMdiWindow windowType);
    virtual ~IEMdiWindow() = default;

public:

    /**
     * \brief   Returns the MDI Window type.
     * \return  The MDI Window type.
     **/
    inline eMdiWindow getMdiWindowType(void) const;

    inline bool isServiceInterfaceWindow(void) const;

    inline bool isLogViewerWindow(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables.
//////////////////////////////////////////////////////////////////////////
protected:
    const eMdiWindow    mMdiWindowType; //!< MDI Window type

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    IEMdiWindow(void) = delete;
    IEMdiWindow(const IEMdiWindow&) = delete;
    IEMdiWindow& operator=(const IEMdiWindow&) = delete;
};

//////////////////////////////////////////////////////////////////////////
// IEMdiWindow inline methods
//////////////////////////////////////////////////////////////////////////

inline IEMdiWindow::eMdiWindow IEMdiWindow::getMdiWindowType(void) const
{
    return mMdiWindowType;
}

inline bool IEMdiWindow::isServiceInterfaceWindow(void) const
{
    return (mMdiWindowType == MdiServiceInterface);
}

inline bool IEMdiWindow::isLogViewerWindow(void) const
{
    return (mMdiWindowType == MdiLogViewer);
}

#endif // LUSAN_VIEW_COMMON_IEMDIWINDOW_HPP

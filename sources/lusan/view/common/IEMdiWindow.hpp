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
public:
    enum eMdiWindow
    {
          MdiUnknown            = 0 //!< Unknown MDI Window type
        , MdiServiceInterface       //!< Service Interface MDI Window type
        , MdiLogViewer              //!< Log Viewer MDI Window type
    };

protected:
    IEMdiWindow(eMdiWindow windowType);

    virtual ~IEMdiWindow() = default;

public:

    /**
     * \brief   Returns the MDI Window type.
     * \return  The MDI Window type.
     **/
    inline eMdiWindow getMdiWindowType(void) const;

protected:
    const eMdiWindow    mMdiWindowType; //!< MDI Window type

private:
    IEMdiWindow(void) = delete;
    IEMdiWindow(const IEMdiWindow&) = delete;
    IEMdiWindow& operator=(const IEMdiWindow&) = delete;
};

inline IEMdiWindow::eMdiWindow IEMdiWindow::getMdiWindowType(void) const
{
    return mMdiWindowType;
}

#endif // LUSAN_VIEW_COMMON_IEMDIWINDOW_HPP

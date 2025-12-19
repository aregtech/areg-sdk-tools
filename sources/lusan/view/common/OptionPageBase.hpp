#ifndef LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/OptionPageBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, options page dialog base class.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>
#include "areg/base/GEMacros.h"

class QDialog;

/**
 * \brief   The base class of option page in the application settings.
 **/
class OptionPageBase : public QWidget
{
    Q_OBJECT
    
//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
public:
    //!< Directory path data
    struct sWorkspaceDir
    {
        bool    isValid {false};    //!< True, if parameter is valid. False, otherwise
        QString location{ };        //!< The path of directory
    };
        
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    OptionPageBase(QDialog* parent);

    virtual ~OptionPageBase(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Call when the option should apply the changes.
     **/
    virtual void applyChanges(void);

    /**
     * \brief   Call when the option page is closing.
     * \param   OKpressed   If true, the user pressed OK button, otherwise Cancel button.
     **/
    virtual void closingOptions(bool OKpressed);

    /**
     * \brief   Triggered, letting option page object to display a warning message.
     **/
    virtual void warnMessage(void);

    /**
     * \brief   Called when the workspace directories in option pages are updated.
     * \param   sources    The sources directory.
     * \param   includes   The includes directory.
     * \param   delivery   The delivery directory.
     * \param   logs       The logs directory.
     **/
    virtual void updateWorkspaceDirectories(const sWorkspaceDir & sources, const sWorkspaceDir& includes, const sWorkspaceDir& delivery, const sWorkspaceDir& logs);
    
//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns true if data of the option page is modified.
     **/
    inline bool isDataModified(void) const;

    /**
     * \brief   Set data modified flag in the option page.
     **/
    inline void setDataModified(bool modified);

    /**
     * \brief   Returns true if the data in option page can be save. False, otherwise.
     **/
    inline bool canSave(void) const;

    /**
     * \brief   Sets saving flag in the option page.
     **/
    inline void setCanSave(bool canSave);

    /**
     * \brief   Returns true if the data in option page can be accepted and the settings dialog can be closed. Otherwise, returns false.
     **/
    inline bool canAcceptOptions(void) const;
    
//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:

    /**
     * \brief   The signal is triggered when the workspace locations are changed.
     * \param   sources    The sources directory.
     * \param   includes   The includes directory.
     * \param   delivery   The delivery directory.
     * \param   logs       The logs directory.
     **/
    void signalWorkspaceLocationsChanged( const OptionPageBase::sWorkspaceDir& sources
                                        , const OptionPageBase::sWorkspaceDir& includes
                                        , const OptionPageBase::sWorkspaceDir& delivery
                                        , const OptionPageBase::sWorkspaceDir& logs);
    
//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:

    bool    mDataModified;  //!< Flag, indicating whether the data in option page is modified.
    bool    mCanSave;       //!< Flag, indicating whether the data in option page can be saved.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageBase(void) = delete;
    DECLARE_NOCOPY_NOMOVE(OptionPageBase);
};

//////////////////////////////////////////////////////////////////////////
// OptionPageBase inline methods
//////////////////////////////////////////////////////////////////////////

inline bool OptionPageBase::isDataModified(void) const
{
    return mDataModified;
}

inline void OptionPageBase::setDataModified(bool modified)
{
    mDataModified = modified;
}

inline bool OptionPageBase::canSave(void) const
{
    return mCanSave;
}

inline void OptionPageBase::setCanSave(bool canSave)
{
    mCanSave = canSave;
}

inline bool OptionPageBase::canAcceptOptions(void) const
{
    return mCanSave || (mDataModified == false);
}

#endif  // LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP

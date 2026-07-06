#ifndef LUSAN_VIEW_COMMON_OPTIONPAGEPROJECTDIRS_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGEPROJECTDIRS_HPP
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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/OptionPageProjectDirs.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, project settings widget.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageBase.hpp"

#include <QString>
#include <memory>

namespace Ui {
class OptionPageProjectDirsDlg;
}

class QDialog;
class QLineEdit;

//////////////////////////////////////////////////////////////////////////
// OptionPageProjectDirs class declaration
//////////////////////////////////////////////////////////////////////////

/**
 * \brief   The OptionPageProjectDirs class is a widget to set the project directory settings.
 **/
class OptionPageProjectDirs : public OptionPageBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit OptionPageProjectDirs(QDialog *parent);
    virtual ~OptionPageProjectDirs();

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Call when the option should apply the changes.
     **/
    virtual void applyChanges() override;
    
    /**
     * \brief   Called when the workspace directories in option pages are updated.
     * \param   sources    The sources directory.
     * \param   includes   The includes directory.
     * \param   delivery   The delivery directory.
     * \param   logs       The logs directory.
     **/
    virtual void updateWorkspaceDirectories(const sWorkspaceDir& sources, const sWorkspaceDir& includes, const sWorkspaceDir& delivery, const sWorkspaceDir& logs) override;
    
//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    /**
     * \brief   Slot triggered when the source directory browse button is clicked.
     **/
    void onSourceDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the include directory browse button is clicked.
     **/
    void onIncludeDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the delivery directory browse button is clicked.
     **/
    void onDeliveryDirBrowseBtnClicked();
    /**
     * \brief   Slot triggered when the log directory browse button is clicked.
     **/
    void onLogDirBrowseBtnClicked();

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Connects the signal handlers.
     **/
    void connectSignalHandlers();
 
    /**
     * \brief   Initializes the paths with the current workspace data.
     **/
    void initializePathsWithCurrentWorkspaceData() const;

    //!< Returns root path edit object
    inline QLineEdit* ctrlRoot() const;
    
    //!< Returns sources path edit object
    inline QLineEdit* ctrlSources() const;
    
    //!< Returns includes path edit object
    inline QLineEdit* ctrlIncludes() const;
    
    //!< Returns delivery path edit object
    inline QLineEdit* ctrlDelivery() const;
    
    //!< Returns logs path edit object
    inline QLineEdit* ctrlLogs() const;
    
//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::OptionPageProjectDirsDlg> mUi;

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageProjectDirs() = delete;
    AREG_NOCOPY_NOMOVE(OptionPageProjectDirs);
};

#endif // LUSAN_VIEW_COMMON_OPTIONPAGEPROJECTDIRS_HPP

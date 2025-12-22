#ifndef LUSAN_VIEW_COMMON_WORKSPACE_HPP
#define LUSAN_VIEW_COMMON_WORKSPACE_HPP
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
 *  \file        lusan/view/common/Workspace.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Workspace setup dialog.
 *
 ************************************************************************/
#include <QDialog>
#include "lusan/model/common/WorkspaceModel.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"

namespace Ui {
class DialogWorkspace;
}

class OptionsManager;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

/**
 * \class   Workspace
 * \brief   Represents the workspace setup dialog in the Lusan application.
 **/
class Workspace : public QDialog
{
    Q_OBJECT
public:
    /**
     * \brief   Constructor.
     * \param   options The options manager.
     * \param   parent  The parent widget.
     **/
    Workspace(OptionsManager & options, QWidget * parent = nullptr);

    /**
     * \brief   Destructor.
     **/
    virtual ~Workspace(void);

    /**
     * \brief   Returns true if the workspace has new entry.
     **/
    inline bool hasNewWorkspaceEntry(void) const;
    
protected slots:
    /**
     * \brief   Slot called when the dialog is accepted.
     **/
    void onAccept(void);
    
    /**
     * \brief   Slot called when the dialog is rejected.
     **/
    void onReject(void);
    
    /**
     * \brief   Slot called when the workspace path is changed.
     * \param   newText The new workspace path.
     **/
    void onWorskpacePathChanged(const QString & newText);
    
    /**
     * \brief   Slot called when the browse button is clicked.
     * \param   checked Indicates whether the button is checked.
     **/
    void onBrowseClicked(bool checked = true);
    
    /**
     * \brief   Slot called when the workspace index is changed.
     * \param   index   The new workspace index.
     **/
    void onWorskpaceIndexChanged(int index);
    
    /**
     * \brief   Slot called when the path selection is changed.
     * \param   topLeft     The top-left index of the selection.
     * \param   bottomRight The bottom-right index of the selection.
     * \param   roles       The roles of the selection.
     **/
    void onPathSelectionChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);
    
    /**
     * \brief   Triggered when default workspace check-box is checked or unchecked.
     * \param   checked     True is checked. False, otherwise.
     **/
    void onDefaultChecked(bool checked);
    
//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    OptionsManager &        mOptions;       //!< The options manager.
    Ui::DialogWorkspace *   mWorkspace;     //!< The UI dialog for workspace setup.
    WorkspaceModel          mModel;         //!< The model for workspace entries.
};

//////////////////////////////////////////////////////////////////////////
// Workspace inline functions implementation
//////////////////////////////////////////////////////////////////////////
inline bool Workspace::hasNewWorkspaceEntry(void) const
{
    return mModel.hasNewWorkspace();
}

#endif // LUSAN_VIEW_COMMON_WORKSPACE_HPP

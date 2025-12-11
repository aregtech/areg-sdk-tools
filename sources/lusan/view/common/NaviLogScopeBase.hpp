#ifndef LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
#define LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
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
 *  \file        lusan/view/common/NaviLogScopeBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       The base class of the log explorer view.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NavigationWindow.hpp"
#include "areg/logging/NELogging.hpp"

#include <QWidget>
#include <QString>

class LoggingScopesModelBase;
class MdiMainWindow;
class LoggingModelBase;
class ScopeNodeBase;
class QItemSelectionModel;
class QModelIndex;
class QToolButton;
class QTreeView;

class NaviLogScopeBase : public NavigationWindow
{
public:
    NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~NaviLogScopeBase(void) = default;

public:

    //!< Sets the scope navigation tree view.
    inline void setNaviTree(QTreeView* treeView);

    //!< Sets the tool-button to hide / show error priority logs.
    inline void setPrioError(QToolButton* toolButton);

    //!< Sets the tool-button to hide / show warning priority logs.
    inline void setPrioWarning(QToolButton* toolButton);

    //!< Sets the tool-button to hide / show info priority logs.
    inline void setPrioInfo(QToolButton* toolButton);

    //!< Sets the tool-button to hide / show debug priority logs.
    inline void setPrioDebug(QToolButton* toolButton);

    //!< Sets the tool-button to hide / show scopes priority logs.
    inline void setPrioScopes(QToolButton* toolButton);

    /**
     * \brief   Returns true if root entries are collapsed.
     **/
    bool areRootsCollapsed(void) const;

    //!< Sets up the model for the log scopes navigation.
    void setupModel(LoggingScopesModelBase* model);

    //!< Sets up the controls for the log scopes navigation.
    void setupControls(QTreeView* treeView, QToolButton* prioError, QToolButton* prioWarning, QToolButton* prioInfo, QToolButton* prioDebug, QToolButton* prioScopes);

public:
    /**
     * \brief   Enables or disables lot priority tool buttons based on selection index.
     *          It also changes the colors of the buttons depending on the priority.
     **/
    virtual void enableButtons(const QModelIndex& selection);

    /**
     * \brief   Updates the colors of the log priority tool buttons.
     * \param   errSelected    If true, the error button is checked and the colored.
     * \param   warnSelected   If true, the warning button is checked and the colored.
     * \param   infoSelected   If true, the info button is checked and the colored.
     * \param   dbgSelected    If true, the debug button is checked and the colored.
     * \param   scopeSelected  If true, the scopes button is checked and the colored.
     **/
    virtual void updateColors(bool errSelected, bool warnSelected, bool infoSelected, bool dbgSelected, bool scopeSelected);

    /**
     * \brief   Updates the expanded of the log scopes model based on the current index.
     * \param   current    The current index to update expanded.
     **/
    virtual void updateExpanded(const QModelIndex& current);

    /**
     * \brief   Updates the priority of the log scope at the given index.
     * \param   node       The index of the log scope to update priority.
     * \param   addPrio    If true, adds the priority. Otherwise, removes the priority.
     * \param   prio       The log priority to set or remove.
     * \return  Returns true if succeeded the request to update the priority. Otherwise, returns false.
     **/
    virtual bool updatePriority(const QModelIndex& node, bool addPrio, NELogging::eLogPriority prio);

    /**
     * \brief   Expands the child nodes of the specified scope tree recursively.
     * \param   idxNode    The index of the node to expand.
     * \param   node       The scope node to check and expand child nodes.
     **/
    virtual void expandChildNodesRecursive(const QModelIndex& idxNode, const ScopeNodeBase& node);

    /**
     * \brief   Collapses the root entries.
     **/
    virtual void collapseRoots(void);

    /**
     * \brief   Sets the pointer of associated live logs model.
     * \param   logModel    The pointer to the live logs model.
     *                      Can be nullptr if no live logs are available.
     **/
    virtual void setLoggingModel(LoggingModelBase* logModel);

    /**
     * \brief   Returns the pointer to the live logs model used by live logging scope navigation view.
     *          If no live logs are available, returns nullptr.
     **/
    virtual LoggingModelBase* getLoggingModel(void) const;

protected:

    /**
     * \brief   Slot, triggered when a log priority tool button is checked or unchecked.
     * \param   checked     The flag, indicating whether the tool button is checked or unchecked.
     * \param   toolButton  The reference to the tool button that was checked or unchecked.
     * \param   prio        The log priority associated with the tool button.
     **/
    virtual void onLogPrioChecked(bool checked, QToolButton& toolButton, NELogging::eLogPriority prio);

    /**
     * \brief   Slot, triggered when a navigation node is expanded or collapsed.
     * \param   index       The index of the navigation node that was expanded or collapsed.
     * \param   expanded    The flag indicating whether the node is expanded or collapsed.
     * \param   toolButton  The tool button associated with the navigation node, if any.
     **/
    virtual void onNodeExpanded(const QModelIndex& index, bool expanded, QToolButton* toolButton);

    //!< Slot, triggered when the selection in the log scopes navigation is changed.
    virtual void onRowChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    //!< Validates the object by checking with assertions.
    inline void validateControls(void);

protected:
    LoggingScopesModelBase* mScopesModel;   //!< The model of the log scopes
    QItemSelectionModel*    mSelModel;      //!< Selection model

private:
    QTreeView*              mNaviTree;      //!< The navigation tree view for log scopes
    QToolButton*            mPrioError;     //!< The tool button for error log priority
    QToolButton*            mPrioWarning;   //!< The tool button for warning log priority
    QToolButton*            mPrioInfo;      //!< The tool button for info log priority
    QToolButton*            mPrioDebug;     //!< The tool button for debug log priority
    QToolButton*            mPrioScopes;    //!< The tool button for scopes log priority
};

inline void NaviLogScopeBase::setNaviTree(QTreeView* treeView)
{
    mNaviTree = treeView;
}

inline void NaviLogScopeBase::setPrioError(QToolButton* toolButton)
{
    mPrioError = toolButton;
}

inline void NaviLogScopeBase::setPrioWarning(QToolButton* toolButton)
{
    mPrioWarning = toolButton;
}

inline void NaviLogScopeBase::setPrioInfo(QToolButton* toolButton)
{
    mPrioInfo = toolButton;
}

inline void NaviLogScopeBase::setPrioDebug(QToolButton* toolButton)
{
    mPrioDebug = toolButton;
}

inline void NaviLogScopeBase::setPrioScopes(QToolButton* toolButton)
{
    mPrioScopes = toolButton;
}

#endif  // LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP

#ifndef LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
#define LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP
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
 *  \file        lusan/view/common/NaviLogScopeBase.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The base class of the log explorer view.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/

#include "lusan/view/common/NaviToolbarWindow.hpp"
#include "areg/logging/areg_log.h"

#include <QList>
#include <QString>

/************************************************************************
 * Dependencies
 ************************************************************************/
class LoggingScopesModelBase;
class LoggingModelBase;
class MdiMainWindow;
class ScopeNodeBase;
class QAction;
class QItemSelectionModel;
class QModelIndex;
class QPoint;
class QToolButton;

//////////////////////////////////////////////////////////////////////////
// NaviLogScopeBase class declaration
//////////////////////////////////////////////////////////////////////////
/**
 * \brief   The base class of the live and offline log scope explorers. It builds the tool
 *          button row shared by both explorers, keeps the scope tree in sync with the scope
 *          model, and shows the context menu of the scope tree.
 **/
class NaviLogScopeBase : public NaviToolbarWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
protected:

    //!< The entries of the scope tree context menu.
    enum eScopeMenu
    {
          MenuPrioNotset    = 0 //!< Reset priorities
        , MenuPrioAllset        //!< Set all priorities
        , MenuPrioDebug         //!< Set debug priority
        , MenuPrioInfo          //!< Set info priority
        , MenuPrioWarn          //!< Set warning priority
        , MenuPrioError         //!< Set error priority
        , MenuPrioFatal         //!< Set fatal priority
        , MenuPrioScope         //!< Set scope priority
        , MenuExpandSelected    //!< Expand selected node
        , MenuCollapseSelected  //!< Collapse selected node
        , MenuExpandAll         //!< Expand all nodes
        , MenuCollapseAll       //!< Collapse all nodes
        , MenuSavePrioTarget    //!< Save priority settings of the selected target
        , MenuSavePrioAll       //!< Save priority settings of all targets

        , MenuCount             //!< The number of entries in the menu
    };

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviLogScopeBase(int naviWindow, MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~NaviLogScopeBase(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Returns true if root entries are collapsed.
     **/
    bool areRootsCollapsed(void) const;

    /**
     * \brief   Sets the scope model and binds it to the scope tree.
     * \param   model   The scope model to show in the tree.
     **/
    void setupModel(LoggingScopesModelBase* model);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
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
    virtual bool updatePriority(const QModelIndex& node, bool addPrio, areg::LogPriority prio);

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

    /**
     * \brief   Collapses the root entries or expands the complete tree.
     * \param   checked     If true, only the root entry stays expanded.
     **/
    virtual void onCollapseClicked(bool checked);

//////////////////////////////////////////////////////////////////////////
// Operations for the derived classes
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Creates the tool button row and the scope tree of the explorer. Call it from
     *          the constructor of the derived class, before the scope model is set.
     * \note    It calls addSpecificTools() and addMoveTools(), so the derived object must be
     *          constructed before the call.
     **/
    void setupScopeToolbar(void);

    /**
     * \brief   Connects the tool buttons and the scope tree. Call it after setupModel().
     **/
    void setupScopeControls(void);

    /**
     * \brief   Adds the tool buttons of the derived explorer placed between the collapse and
     *          the find buttons.
     **/
    virtual void addSpecificTools(void);

    /**
     * \brief   Adds the tool buttons of the derived explorer placed at the end of the row.
     **/
    virtual void addMoveTools(void);

    /**
     * \brief   Returns true if the context menu offers the entries to save the priorities on
     *          the logging targets.
     **/
    virtual bool hasSavePrioMenu(void) const;

    /**
     * \brief   Returns true if the priorities can be saved on the logging targets right now.
     **/
    virtual bool canSavePrio(void) const;

    /**
     * \brief   Returns true if the context menu offers the entry to select all priorities.
     **/
    virtual bool hasSelectAllPrioMenu(void) const;

    /**
     * \brief   Shows the context menu of the scope tree and runs the chosen entry.
     * \param   pos     The cursor position in the coordinates of the tree viewport.
     **/
    void showScopeContextMenu(const QPoint& pos);

    /**
     * \brief   Expands the given node and its direct children.
     * \param   node            The index of the node to expand.
     * \param   markExpanded    If true, the model keeps the expanded state of the nodes.
     **/
    void expandNodeAndChildren(const QModelIndex& node, bool markExpanded);

    /**
     * \brief   Expands the given node if it is collapsed.
     * \param   node            The index of the node to expand.
     * \param   markExpanded    If true, the model keeps the expanded state of the node.
     **/
    void expandNode(const QModelIndex& node, bool markExpanded);

//////////////////////////////////////////////////////////////////////////
// Tool button controls
//////////////////////////////////////////////////////////////////////////
protected:

    //!< Returns the control object to expand or collapse entries of scopes.
    inline QToolButton* ctrlCollapse(void) const;

    //!< Returns the control object to find a string.
    inline QToolButton* ctrlFind(void) const;

    //!< Returns the control object to set error level of the logs.
    inline QToolButton* ctrlLogError(void) const;

    //!< Returns the control object to set warning level of the logs.
    inline QToolButton* ctrlLogWarning(void) const;

    //!< Returns the control object to set information level of the logs.
    inline QToolButton* ctrlLogInfo(void) const;

    //!< Returns the control object to set debug level of the logs.
    inline QToolButton* ctrlLogDebug(void) const;

    //!< Returns the control object to enable log scopes of the logs.
    inline QToolButton* ctrlLogScopes(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   Slot, triggered when a log priority tool button is checked or unchecked.
     * \param   checked     The flag, indicating whether the tool button is checked or unchecked.
     * \param   toolButton  The reference to the tool button that was checked or unchecked.
     * \param   prio        The log priority associated with the tool button.
     **/
    virtual void onLogPrioChecked(bool checked, QToolButton& toolButton, areg::LogPriority prio);

    /**
     * \brief   Slot, triggered when a navigation node is expanded or collapsed.
     * \param   index       The index of the navigation node that was expanded or collapsed.
     * \param   expanded    The flag indicating whether the node is expanded or collapsed.
     **/
    virtual void onNodeExpanded(const QModelIndex& index, bool expanded);

    //!< Slot, triggered when the selection in the log scopes navigation is changed.
    virtual void onRowChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    //!< Validates the object by checking with assertions.
    inline void validateControls(void) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
protected:
    LoggingScopesModelBase* mScopesModel;   //!< The model of the log scopes
    QItemSelectionModel*    mSelModel;      //!< Selection model

private:
    QToolButton*            mToolCollapse;  //!< The tool button to collapse or expand the scope tree
    QToolButton*            mToolFind;      //!< The tool button to find a log message
    QToolButton*            mPrioError;     //!< The tool button for error log priority
    QToolButton*            mPrioWarning;   //!< The tool button for warning log priority
    QToolButton*            mPrioInfo;      //!< The tool button for info log priority
    QToolButton*            mPrioDebug;     //!< The tool button for debug log priority
    QToolButton*            mPrioScopes;    //!< The tool button for scopes log priority
    QList<QAction*>         mMenuActions;   //!< The entries of the scope tree context menu
};

//////////////////////////////////////////////////////////////////////////
// NaviLogScopeBase class inline methods
//////////////////////////////////////////////////////////////////////////

inline QToolButton* NaviLogScopeBase::ctrlCollapse(void) const
{
    return mToolCollapse;
}

inline QToolButton* NaviLogScopeBase::ctrlFind(void) const
{
    return mToolFind;
}

inline QToolButton* NaviLogScopeBase::ctrlLogError(void) const
{
    return mPrioError;
}

inline QToolButton* NaviLogScopeBase::ctrlLogWarning(void) const
{
    return mPrioWarning;
}

inline QToolButton* NaviLogScopeBase::ctrlLogInfo(void) const
{
    return mPrioInfo;
}

inline QToolButton* NaviLogScopeBase::ctrlLogDebug(void) const
{
    return mPrioDebug;
}

inline QToolButton* NaviLogScopeBase::ctrlLogScopes(void) const
{
    return mPrioScopes;
}

#endif  // LUSAN_VIEW_COMMON_NAVILOGSCOPEBASE_HPP

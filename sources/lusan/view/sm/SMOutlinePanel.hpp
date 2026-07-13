#ifndef LUSAN_VIEW_SM_SMOUTLINEPANEL_HPP
#define LUSAN_VIEW_SM_SMOUTLINEPANEL_HPP
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
 *  \file        lusan/view/sm/SMOutlinePanel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Design page outline panel.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include <QHash>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QTreeWidget;
class QTreeWidgetItem;
class QTimer;
class SMSceneManager;
class SMStateData;
class StateMachineModel;
enum class eDocElementKind;

/**
 * \class   SMOutlinePanel
 * \brief   The Design page's left panel: the full document hierarchy (machine levels,
 *          their states and transitions) plus the shared registry groups. Selection is
 *          two-way bound to the document selection model, so the canvas, outline, and
 *          properties always agree; a double-click navigates to the element's level
 *          (entering a composite's submachine). The tree is rebuilt from notifier
 *          signals, preserving expansion.
 **/
class SMOutlinePanel : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    SMOutlinePanel(StateMachineModel& model, SMSceneManager& sceneManager, QWidget* parent = nullptr);
    virtual ~SMOutlinePanel();

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Returns the underlying tree view (tests scope their row lookups to it).
     **/
    inline QTreeWidget* getTree() const;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    /**
     * \brief   Requested by the context menu's Rename entry for the given state.
     **/
    void signalRenameRequested(uint32_t stateId);

    /**
     * \brief   Requested by the context menu's Delete entry (operates on the selection).
     **/
    void signalDeleteRequested();

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private slots:
    void onOutlineSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onModelSelectionChanged();
    void onContextMenuRequested(const QPoint& pos);
    void scheduleRebuild();
    void rebuild();

private:
    /**
     * \brief   Appends every state of a level (with its transitions and nested substates)
     *          under \p parent; \p levelId is the level the states live on.
     **/
    void addLevelStates(QTreeWidgetItem* parent, const SMStateData& level, uint32_t levelId);

    /**
     * \brief   Creates a leaf/branch item carrying its element ID, owning level, and role,
     *          and records it in the ID lookup.
     **/
    QTreeWidgetItem* makeItem(const QString& label, uint32_t elementId, uint32_t levelId, int role, bool composite);

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    StateMachineModel&              mModel;         //!< The document facade.
    SMSceneManager&                 mSceneManager;  //!< Level navigation.
    QTreeWidget*                    mTree;          //!< The hierarchy view.
    QTimer*                         mRebuildTimer;  //!< Coalesces bursts of notifier signals.
    QHash<uint32_t, QTreeWidgetItem*> mItemById;    //!< Element ID to its row (canvas kinds only).
    bool                            mUpdating;      //!< Guards the two-way selection sync.
};

//////////////////////////////////////////////////////////////////////////
// SMOutlinePanel inline methods
//////////////////////////////////////////////////////////////////////////

inline QTreeWidget* SMOutlinePanel::getTree() const
{
    return mTree;
}

#endif  // LUSAN_VIEW_SM_SMOUTLINEPANEL_HPP

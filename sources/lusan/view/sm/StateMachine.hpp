#ifndef LUSAN_VIEW_SM_STATEMACHINE_HPP
#define LUSAN_VIEW_SM_STATEMACHINE_HPP
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
 *  \file        lusan/view/sm/StateMachine.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/data/sm/SMReferences.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/MdiChild.hpp"
#include "lusan/view/sm/SMDesign.hpp"

#include <QList>
#include <QTabWidget>
#include <QVector>

class MdiMainWindow;
class SMOverview;

class StateMachine : public MdiChild
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   The pages of the FSM editor, mirroring the Service Interface editor.
     **/
    enum eSMPages
    {
          PageOverview      = 0 //!< Machine name, version, threading, description.
        , PageDataTypes         //!< FSM data types.
        , PageAttributes        //!< FSM attributes.
        , PageEvents            //!< Events and timers.
        , PageMethods           //!< Triggers, actions, conditions.
        , PageConstants         //!< Constants.
        , PageIncludes          //!< Includes and imports.
        , PageDesign            //!< State graph canvas.
    };

public:
    static const QString& fileExtension();

public:
    StateMachine(MdiMainWindow* wndMain, const QString& filePath = QString(), const QString& sourcePath = QString(), QWidget* parent = nullptr);
    virtual ~StateMachine() = default;

public:
    void newFile() override;
    bool openSucceeded() const override;
    void undo() override;
    void redo() override;
    bool canUndo() const override;
    bool canRedo() const override;

    /**
     * \brief   Clipboard commands, forwarded to the Design page while it is the
     *          selected tab; the other pages keep the default no-op.
     **/
    void cut() override;
    void copy() override;
    void paste() override;

    /**
     * \brief   Find (Ctrl+F) and where-used (Shift+F12), forwarded from the main window's
     *          Edit menu. Find reveals the Design page's search field; where-used pops the
     *          references of the currently selected entry on whichever page is in front.
     **/
    void find() override;
    void findUsages() override;

    /**
     * \brief   Go to Declaration (F12): from a state or transition selected on the Design page,
     *          navigate to the registry declaration one of its references points at (the Design
     *          page resolves the targets and offers a picker when there is more than one).
     **/
    void gotoDefinition() override;

    void setToolbarVisible(bool visible) override;
    bool isToolbarVisible() const override;

    /**
     * \brief   Returns the Design page if it has already been built, or nullptr - never
     *          forces its (expensive) construction just to be queried (used by
     *          MdiMainWindow to populate the Design menu).
     **/
    SMDesign* designPageIfBuilt() const;

    /**
     * \brief   True when the Design page is the currently selected inner tab. The drawing
     *          toolbar's commands are only active on the Design page (they act on the canvas).
     **/
    bool isDesignPageCurrent() const;

protected:
    QString newDocumentName() override;
    const QString& newDocument() const override;
    const QString& newDocumentExt() const override;
    const QString& fileSuffix() const override;
    const QString& fileFilter() const override;
    QString suggestedSaveName() const override;
    bool writeToFile(const QString& filePath) override;
    bool maybeSave() override;
    void onWindowClosing(bool isActive) override;

    /**
     * \brief   Returns the page tab host, enabling the shared Ctrl+PageDown / Ctrl+PageUp cycling.
     **/
    QTabWidget* pageTabWidget() override;

private slots:
    /**
     * \brief   Switches to the page owning the requested declaration kind (building it if
     *          not built yet) and starts a new entry there (Declare dropdown).
     **/
    void onDeclareRequested(SMDesign::eDeclareKind kind);

    /**
     * \brief   Switches to the editor page that owns the given element kind, so a validation
     *          finding on a registry entry brings its page forward (the canvas handles state
     *          and transition findings itself).
     **/
    void onNavigateToPage(eDocElementKind kind);

private:
    bool loadDocument(const QString& documentPath, const QString& sourcePath = QString());

    /**
     * \brief   Resolves the entry selected on the current page into a search seed (its kind,
     *          id, and name), so Ctrl+F searches that specific entry's usages. False when the
     *          page has no selected referenceable entry.
     **/
    bool currentSearchSeed(SMReferences::eTarget& target, uint32_t& id, QString& name);

    /**
     * \brief   Switches to the registry page that declares the given element kind and selects the
     *          declaration by ID (the go-to-declaration landing step). Wired to the Design page's
     *          signalNavigateToDefinition; state targets are revealed on the canvas, not here.
     **/
    void navigateToDefinition(SMReferences::eTarget kind, uint32_t declId);

    //!< Number of editor pages.
    static constexpr int pageCount() { return static_cast<int>(PageDesign) + 1; }
    //!< True if the index is a valid page index.
    static bool isValidTabIndex(int index);
    //!< True if the page at the given index has already been built.
    bool isTabInitialized(int index) const;
    //!< The tab title of the page at the given index.
    QString tabTitle(int index) const;
    //!< Builds the next queued page and re-schedules itself while the queue is not empty.
    void processQueuedTabInitialization();
    //!< Builds the page at the given index if it does not exist yet.
    void ensureTabInitialized(int index);
    //!< Places the built page into the tab holder widget at the given index.
    void attachPage(int index, QWidget* page);

private:
    StateMachineModel   mModel;
    QTabWidget          mTabWidget;
    SMOverview*         mOverview;
    QVector<QWidget*>   mPages;             //!< Built page widget per tab index (nullptr until built).
    QList<int>          mPendingInitTabs;   //!< Background initialization queue.
    bool                mToolbarVisible;    //!< The Design page toolbar's requested visibility;
                                            //!< applied immediately if built, or on its construction.
};

#endif  // LUSAN_VIEW_SM_STATEMACHINE_HPP

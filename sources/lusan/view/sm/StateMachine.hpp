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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/StateMachine.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, State Machine MDI window.
 *
 ************************************************************************/

#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/MdiChild.hpp"

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
    virtual void newFile() override;
    virtual bool openSucceeded() const override;
    virtual void undo() override;
    virtual void redo() override;

protected:
    virtual QString newDocumentName() override;
    virtual const QString& newDocument() const override;
    virtual const QString& newDocumentExt() const override;
    virtual const QString& fileSuffix() const override;
    virtual const QString& fileFilter() const override;
    virtual bool writeToFile(const QString& filePath) override;
    virtual bool maybeSave() override;
    virtual void onWindowClosing(bool isActive) override;

private:
    bool loadDocument(const QString& documentPath, const QString& sourcePath = QString());

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
};

#endif  // LUSAN_VIEW_SM_STATEMACHINE_HPP

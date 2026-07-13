#ifndef LUSAN_VIEW_COMMON_NAVIDESIGNPANEL_HPP
#define LUSAN_VIEW_COMMON_NAVIDESIGNPANEL_HPP
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
 *  \file        lusan/view/common/NaviDesignPanel.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       The FSM design Properties / Outline navigation window host.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/NavigationWindow.hpp"

#include <QPointer>

/************************************************************************
 * Dependencies
 ************************************************************************/
class MdiMainWindow;
class SMDesign;
class QVBoxLayout;
class QWidget;

/**
 * \class   NaviDesignPanel
 * \brief   A navigation-dock tab that hosts the State Machine designer's Properties or
 *          Outline panel while the user chose to move it out of the Design page (issue
 *          #516). Because the two panels bind to one document's model in their constructor,
 *          this host owns a single instance and re-binds by recreating the hosted panel for
 *          whichever Design page is active. Without an active Design page it shows an empty
 *          placeholder, so the tab is present but blank until the user is on a Design page.
 **/
class NaviDesignPanel : public NavigationWindow
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \enum    eKind
     * \brief   Which Design page panel this host stands in for.
     **/
    enum class eKind
    {
          Properties    //!< Hosts a SMPropertiesPanel bound to the active Design page.
        , Outline       //!< Hosts a SMOutlinePanel bound to the active Design page.
    };

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    NaviDesignPanel(NaviDesignPanel::eKind kind, MdiMainWindow* wndMain, QWidget* parent = nullptr);
    virtual ~NaviDesignPanel() = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Re-binds the host to the given Design page by recreating the hosted panel from
     *          that page's model / scene manager; nullptr shows the empty placeholder.
     **/
    void bindDesign(SMDesign* design);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Replaces the hosted content with a centered "no Design page" placeholder.
     **/
    void showPlaceholder();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    const eKind         mKind;      //!< Whether this host stands in for Properties or Outline.
    QVBoxLayout*        mLayout;    //!< The single-child body layout.
    QWidget*            mContent;   //!< The current hosted widget (panel or placeholder).
    QPointer<SMDesign>  mBound;     //!< The currently bound Design page, if any.
};

#endif  // LUSAN_VIEW_COMMON_NAVIDESIGNPANEL_HPP

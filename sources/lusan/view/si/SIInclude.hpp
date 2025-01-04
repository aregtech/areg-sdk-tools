#ifndef LUSAN_APPLICATION_SI_SIINCLUDE_HPP
#define LUSAN_APPLICATION_SI_SIINCLUDE_HPP
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
 *  \file        lusan/view/si/SIInclude.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QScrollArea>

/************************************************************************
 * Dependencies
 ************************************************************************/
namespace Ui {
    class SIInclude;
}

class SIIncludeDetails;
class SIIncludeList;
class SIIncludeModel;

/**
 * \brief   The widget object for SIInclude page.
 **/
class SIIncludeWidget : public QWidget
{
    friend class SIInclude;

    Q_OBJECT

public:
    explicit SIIncludeWidget(QWidget* parent);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    Ui::SIInclude* ui;
};

/**
 * \brief   The SIInclude class is the view of the Service Interface
 *          Include section. It displays the list of included files
 *          and allows to add, remove, update, and insert new entries.
 **/
class SIInclude : public QScrollArea
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Constructor with initialization.
     * \param   model   The model of the SIInclude section.
     * \param   parent  The parent widget of the section.
     **/
    explicit SIInclude(SIIncludeModel & model, QWidget* parent = nullptr);

    virtual ~SIInclude(void);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected slots:

    /**
     * \brief   Triggered when the current cell is changed.
     * \param   currentRow      The current row index.
     * \param   currentColumn   The current column index.
     * \param   previousRow     The previous row index.
     * \param   previousColumn  The previous column index.
     **/
    void onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    /**
     * \brief   Triggered when the add button is clicked.
     **/
    void onAddClicked(void);

    /**
     * \brief   Triggered when the remove button is clicked.
     **/
    void onRemoveClicked(void);

    /**
     * \brief   Triggered when the insert button is clicked.
     **/
    void onInsertClicked(void);

    /**
     * \brief   Triggered when the update button is clicked.
     **/
    void onUpdateClicked(void);

    /**
     * \brief   Triggered when the browse button is clicked.
     **/
    void onBrowseClicked(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Initializes the SIInclude object.
     **/
    void updateData(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

    /**
     * \brief   Returns the list of supported file extensions.
     **/
    static QStringList getSupportedExtensions(void);

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeModel&     mModel;         //!< The model of the SIInclude section.
    SIIncludeDetails*   mPageDetails;   //!< The details page.
    SIIncludeList*      mPageList;      //!< The list page.
    SIIncludeWidget*    mWidget;        //!< The widget object.
    Ui::SIInclude&      ui;             //!< The UI object.

    QString                 mCurUrl;    //!< The current URL.
    QString                 mCurFile;   //!< The current file.
    QString                 mCurFilter; //!< The current filter.
    int                     mCurView;   //!< The current view mode.
};

#endif // LUSAN_APPLICATION_SI_SIINCLUDE_HPP
